// ============================================================================
//  CyberChuck // Hacking 101  --  T-Dongle-C5 pen-testing learning tool
//  v0.1  (Lesson 1: brute-forcing a 4-digit PIN lock)
//
//  Plug it in -> Cyberchuck logo on the screen -> it hosts an open Wi-Fi AP with
//  a captive portal. Connect a phone/laptop, the lesson opens itself. Everything
//  you attack lives inside this dongle. Nothing leaves it. No internet.
//
//  Board (Arduino IDE): "ESP32C5 Dev Module"  (needs a recent ESP32 core, 3.1+)
//  Flash Size / Partition: match YOUR unit (yours reported 4MB -> pick a 4MB
//  scheme with a big enough app partition; if it's a 16MB unit, use 16MB).
//  Uploading: hold BOOT while plugging in, then flash.
// ============================================================================
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_random.h>
#include <Preferences.h>          // onboard flash storage for saved progress
#include <SPI.h>                  // APA102 LED rides the display's SPI bus

#include "pin_config.h"
#include "portal.h"

#if USE_DISPLAY
  #include <Adafruit_GFX.h>
  #include <Adafruit_ST7735.h>
  #include "logo_data.h"
  #ifndef BLACK
    #define BLACK 0x0000
  #endif
  #ifndef RGB565
    #define RGB565(r,g,b) ((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|((b)>>3))
  #endif
#endif
#if USE_SD
  #if SD_MODE_SPI
    #include <SD.h>              // C5: no SDMMC -> SD over SPI
    SPIClass sdSPI(FSPI);
  #else
    #include <SD_MMC.h>          // S3: native SDMMC (1-bit)
  #endif
#endif

#define FW_VERSION "0.1"
static const char *AP_SSID = "CyberChuck_Sandbox";   // open network, no password

WebServer server(80);
DNSServer  dns;
IPAddress  apIP(192, 168, 4, 1);
Preferences prefs;

// ---- sandbox state ----------------------------------------------------------
bool     solved[4]     = {false, false, false, false};
bool     hasVoice      = false;
uint32_t attempts      = 0;

// Lesson 1 is now clue-based password guessing. Each "case" is a fake person
// whose password is built from personal facts (the recon clues shown to the
// player). Difficulty = which stage: 0 Learn(easy), 1 Easy, 2 Medium, 3 Hard.
struct Case {
  String name, acct, pass, hint;
  String clues;                 // JSON array body (elements, no brackets)
  String pet, kid, team, city;  // raw facts, for the wordlist demo
  int    year = 0, diff = 0;
};
Case cases[5];   // 0..3 = difficulty tiers, 4 = user-built custom case

static const char* FIRST[] = {"Marcus","Elena","Derek","Priya","Sofia","Jamal","Nadia","Owen",
                              "Carla","Victor","Aisha","Grant","Lena","Tomas","Ruby","Hassan"};
static const char* LAST[]  = {"Webb","Cole","Nash","Patel","Ryan","Flores","Kim","Ford",
                              "Diaz","Stone","Reed","Vance"};
static const char* PETS[]  = {"Rex","Luna","Bella","Max","Simba","Coco","Zeus","Nala","Bruno","Ziggy","Mocha","Pixel"};
static const char* TEAMS[] = {"Lakers","Yankees","Cowboys","Celtics","Raptors","Falcons","Bruins","Rams","Heat","Kings"};
static const char* KIDS[]  = {"Ethan","Maya","Liam","Ava","Noah","Zoe","Caleb","Isla","Mila","Jonah","Nora","Reese"};
static const char* CITIES[]= {"Denver","Austin","Tampa","Reno","Boise","Fresno","Akron","Ogden","Provo","Salem"};
static const char* ACCTS[] = {"personal email","online banking","work VPN","cloud storage"};
static const char* SYMS[]  = {"!","@","#","$"};
#define PICK(arr) arr[esp_random() % (sizeof(arr)/sizeof(arr[0]))]

String jsonEsc(const String &s){
  String o; for (char c : s){ if (c=='"'||c=='\\') o+='\\'; o+=c; } return o;
}
String leet(const String &w){
  String o; for (char c : w){ char l=tolower(c);
    if(l=='a')o+='4'; else if(l=='e')o+='3'; else if(l=='i')o+='1';
    else if(l=='o')o+='0'; else if(l=='s')o+='5'; else o+=c; } return o;
}
String capFirst(String w){ if(w.length()) w.setCharAt(0, toupper(w[0])); return w; }

void buildCase(int s){
  Case c;
  const char* first=PICK(FIRST); const char* last=PICK(LAST);
  c.pet=PICK(PETS); c.kid=PICK(KIDS); c.team=PICK(TEAMS); c.city=PICK(CITIES);
  c.acct=PICK(ACCTS); c.year=1980 + (esp_random()%30);
  const char* sym=PICK(SYMS);
  char y2[3]; snprintf(y2,3,"%02d", c.year%100);
  int level = (s==0)?1:s;   // Learn behaves like Easy, just with the walkthrough
  c.diff = level;
  c.name = String(first)+" "+last;

  auto add=[&](const String &txt){ if(c.clues.length())c.clues+=","; c.clues+="\""+jsonEsc(txt)+"\""; };

  if (level<=1){                                  // EASY: pet + year
    c.pass = c.pet + String(c.year);
    add("Has a dog named "+c.pet);
    add("Born in "+String(c.year));
    add("Lives in "+c.city);
    c.hint = "Weak passwords are often a favorite name plus a year, stuck together.";
  } else if (level==2){                            // MEDIUM: pet + kid
    c.pass = c.pet + c.kid;
    add("Has a dog named "+c.pet);
    add("Has a kid named "+c.kid);
    add("Season-ticket holder for the "+c.team);
    add("Born in "+String(c.year));
    c.hint = "It's two of these favorites mashed together, each capitalized.";
  } else {                                         // HARD: leet(pet) + yr2 + symbol
    c.pass = leet(c.pet) + String(y2) + String(sym);
    add("Has a dog named "+c.pet);
    add("Has a kid named "+c.kid);
    add("Roots for the "+c.team);
    add("Born in "+String(c.year));
    add("Lives in "+c.city);
    c.hint = "";                                   // clues only, no pattern hint
  }
  cases[s] = c;
}
void seedCases(){ for (int i=0;i<4;i++) buildCase(i); }

// ---- persistent progress (survives unplugging, no SD needed) ----------------
void saveProgress(){
  uint8_t m = 0;
  for (int i = 0; i < 4; i++) if (solved[i]) m |= (1 << i);
  prefs.putUChar("l1", m);                 // lesson 1 completion bitmask
}
void loadProgress(){
  uint8_t m = prefs.getUChar("l1", 0);
  for (int i = 0; i < 4; i++) solved[i] = m & (1 << i);
}
void clearProgress(){
  for (int i = 0; i < 4; i++) solved[i] = false;
  saveProgress();
}

// ============================================================================
//  DISPLAY
// ============================================================================
#if USE_DISPLAY
SPIClass lcdSPI(FSPI);   // display on its own FSPI instance (works on C5 and S3)
Adafruit_ST7735 *gfx = new Adafruit_ST7735(&lcdSPI, LCD_CS, LCD_DC, LCD_RST);

uint16_t C_CYAN, C_GREEN, C_AMBER, C_RED, C_DIM, C_WHITE;

void drawStatus(const char *txt, uint16_t col){
  gfx->fillRect(0, 148, 80, 12, BLACK);
  gfx->setTextColor(col);
  gfx->setCursor(4, 150);
  gfx->setTextSize(1);
  gfx->print(txt);
}

void drawBootScreen(){
  gfx->fillScreen(BLACK);
  gfx->drawRGBBitmap(0, 0, (uint16_t *)LOGO_DATA, LOGO_W, LOGO_H); // 80x80 top
  gfx->setTextSize(1);
  gfx->setTextColor(C_CYAN);   gfx->setCursor(2, 86);  gfx->print("CONNECT WIFI:");
  gfx->setTextColor(C_WHITE);  gfx->setCursor(2, 98);  gfx->print(AP_SSID);
  gfx->setTextColor(C_CYAN);   gfx->setCursor(2, 122); gfx->print("OPEN BROWSER:");
  gfx->setTextColor(C_WHITE);  gfx->setCursor(2, 134); gfx->print("192.168.4.1");
  drawStatus("IDLE", C_DIM);
}
#endif

// ============================================================================
//  STATUS LED  (onboard APA102 / DotStar)
//  C5: the APA102 shares the display SPI bus -> clock frames over SPI (CS stays
//      HIGH so only the LED latches them).
//  S3: the APA102 has its own pins -> bit-bang them.
//  pin_config.h picks the method via LED_ON_DISPLAY_BUS.
// ============================================================================
#if USE_LED
#if LED_ON_DISPLAY_BUS
void ledFrame(uint8_t r, uint8_t g, uint8_t b){
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(0x00); SPI.transfer(0x00); SPI.transfer(0x00); SPI.transfer(0x00);
  SPI.transfer(0xE0 | (LED_BRIGHTNESS & 0x1F));
  #if LED_ORDER_BGR
    SPI.transfer(b); SPI.transfer(g); SPI.transfer(r);
  #else
    SPI.transfer(r); SPI.transfer(g); SPI.transfer(b);
  #endif
  SPI.transfer(0xFF); SPI.transfer(0xFF); SPI.transfer(0xFF); SPI.transfer(0xFF);
  SPI.endTransaction();
}
void ledColor(uint8_t r, uint8_t g, uint8_t b){ ledFrame(r,g,b); }
void ledInit(){
  #if !USE_DISPLAY
    SPI.begin(LCD_SCLK, LCD_MISO, LCD_MOSI, -1);   // no display to start the bus, so we do
  #endif
}
#else   // ---- bit-bang on dedicated pins (S3) ----
static inline void ledByte(uint8_t v){
  for (int i = 0; i < 8; i++){
    digitalWrite(LED_DATA, (v & 0x80) ? HIGH : LOW);
    digitalWrite(LED_CLK, HIGH); digitalWrite(LED_CLK, LOW);
    v <<= 1;
  }
}
void ledColor(uint8_t r, uint8_t g, uint8_t b){
  ledByte(0); ledByte(0); ledByte(0); ledByte(0);
  ledByte(0xE0 | (LED_BRIGHTNESS & 0x1F));
  #if LED_ORDER_BGR
    ledByte(b); ledByte(g); ledByte(r);
  #else
    ledByte(r); ledByte(g); ledByte(b);
  #endif
  ledByte(0xFF); ledByte(0xFF); ledByte(0xFF); ledByte(0xFF);
}
void ledInit(){ pinMode(LED_DATA, OUTPUT); pinMode(LED_CLK, OUTPUT); }
#endif
#else
void ledColor(uint8_t, uint8_t, uint8_t){}
void ledInit(){}
#endif

// unified status setter: screen + LED together
void setDeviceStatus(const String &s){
#if USE_DISPLAY
  if      (s == "attack") drawStatus("ATTACK...", C_AMBER);
  else if (s == "solved") drawStatus("ACCESS OK", C_GREEN);
  else                    drawStatus("IDLE",      C_DIM);
#endif
  if      (s == "attack") ledColor(255, 140,   0);   // amber
  else if (s == "solved") ledColor(  0, 255,  60);   // green
  else                    ledColor(  0, 200, 255);   // cyan idle
}

// ============================================================================
//  WEB HANDLERS
// ============================================================================
void redirectRoot(){
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
}

void handleRoot(){ server.send_P(200, "text/html", PORTAL_HTML); }

void handleState(){
  String j = "{\"ssid\":\"" + String(AP_SSID) + "\",\"version\":\"" FW_VERSION "\",";
  j += "\"hasVoice\":"; j += (hasVoice ? "true" : "false");
  j += ",\"solved\":[";
  for (int i = 0; i < 4; i++){ j += (solved[i] ? "true" : "false"); if (i < 3) j += ","; }
  j += "]}";
  server.send(200, "application/json", j);
}

void handleNewcase(){                                  // (re)generate a case for a difficulty
  int s = server.arg("stage").toInt();
  if (s < 0 || s > 3){ server.send(400, "application/json", "{\"error\":\"stage\"}"); return; }
  buildCase(s);                                        // fresh scenario every time -> replayable
  Case &c = cases[s];
  String j = "{";
  j += "\"name\":\""  + jsonEsc(c.name) + "\",";
  j += "\"acct\":\""  + jsonEsc(c.acct) + "\",";
  j += "\"diff\":"    + String(c.diff) + ",";
  j += "\"hint\":\""  + jsonEsc(c.hint) + "\",";
  j += "\"clues\":["  + c.clues + "]";
  j += "}";
  server.send(200, "application/json", j);
}

void handleCustom(){                                    // build a case from user-supplied facts
  Case c;
  String name = server.arg("name"); if(!name.length()) name="Target User";
  c.name = name;
  c.acct = "the account you described";
  c.pet  = server.arg("pet");
  c.kid  = server.arg("kid");
  c.team = server.arg("team");
  c.city = server.arg("city");
  int year = server.arg("year").toInt(); if (year<1900 || year>2100) year=1990;
  c.year = year;
  int level = server.arg("diff").toInt(); if (level<1) level=1; if (level>3) level=3;
  c.diff = level;
  char y2[3]; snprintf(y2,3,"%02d", year%100);

  String base = c.pet.length()? c.pet : (c.kid.length()? c.kid : name);
  base = capFirst(base);

  auto add=[&](const String &t){ if(t.length()){ if(c.clues.length())c.clues+=","; c.clues+="\""+jsonEsc(t)+"\""; } };
  if (c.pet.length())  add("Pet named "+capFirst(c.pet));
  if (c.kid.length())  add("Kid named "+capFirst(c.kid));
  if (c.team.length()) add("Fan of the "+capFirst(c.team));
  add("Born in "+String(year));
  if (c.city.length()) add("Lives in "+capFirst(c.city));

  if (level==1){
    c.pass = base + String(year);
    c.hint = "Favorite name plus birth year, stuck together.";
  } else if (level==2){
    String w2 = c.kid.length()? capFirst(c.kid)
              : c.team.length()? capFirst(c.team)
              : (c.city.length()? capFirst(c.city) : base);
    c.pass = base + w2;
    c.hint = "Two favorites mashed together, capitalized.";
  } else {
    c.pass = leet(base) + String(y2) + "!";
    c.hint = "";
  }
  cases[4] = c;

  String j = "{";
  j += "\"name\":\""  + jsonEsc(c.name) + "\",";
  j += "\"acct\":\""  + jsonEsc(c.acct) + "\",";
  j += "\"diff\":"    + String(c.diff) + ",";
  j += "\"hint\":\""  + jsonEsc(c.hint) + "\",";
  j += "\"clues\":["  + c.clues + "]";
  j += "}";
  server.send(200, "application/json", j);
}

void handleAttempt(){
  int s = server.arg("stage").toInt();
  if (s < 0 || s > 4){ server.send(400, "application/json", "{\"error\":\"stage\"}"); return; }
  String guess = server.arg("guess");
  attempts++;
  bool correct = guess.length() && guess == cases[s].pass;   // exact, case-sensitive
  if (correct && s <= 3){ solved[s] = true; saveProgress(); }  // custom (4) doesn't affect progress
  server.send(200, "application/json", correct ? "{\"correct\":true}" : "{\"correct\":false}");
}

void handleSweep(){                                    // in-portal wordlist attack (demo)
  int s = server.arg("stage").toInt();
  if (s < 0 || s > 4){ server.send(400, "application/json", "{\"error\":\"stage\"}"); return; }
  Case &c = cases[s];
  if (s <= 3){ solved[s] = true; saveProgress(); }
  char y2[3]; snprintf(y2,3,"%02d", c.year%100);
  // a handful of plausible candidates a cracker would mangle from the recon,
  // real password last. The visible "tries" count grows with difficulty.
  String tried[8];
  tried[0]=c.pet;
  tried[1]=leet(c.pet);                       // lowercased-ish leet
  tried[2]=c.pet+String(y2);
  tried[3]=c.city+String(c.year);
  tried[4]=c.kid+String(c.year);
  tried[5]=c.team+"!";
  tried[6]=leet(c.pet)+String(y2);
  tried[7]=c.pass;                            // the hit
  String arr; for(int i=0;i<8;i++){ if(i)arr+=","; arr+="\""+jsonEsc(tried[i])+"\""; }
  long fake = (c.diff<=1)? 130 : (c.diff==2? 3400 : 21000);
  String j = "{\"password\":\""+jsonEsc(c.pass)+"\",\"tries\":"+String(fake)+",\"tried\":["+arr+"]}";
  server.send(200, "application/json", j);
}

void handleSolved(){
  int s = server.arg("stage").toInt();
  if (s >= 0 && s <= 3){ solved[s] = true; saveProgress(); }
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleClearProgress(){
  clearProgress();
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleReset(){
  int s = server.arg("stage").toInt();
  if (s >= 0 && s <= 3){ buildCase(s); solved[s] = false; saveProgress(); }   // fresh case, replayable
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleStatus(){
  setDeviceStatus(server.arg("state"));
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleNotFound(){
  String uri = server.uri();
#if USE_SD
  if (uri.startsWith("/voice/")){                     // serve narration clips off SD
   #if SD_MODE_SPI
    if (hasVoice && SD.exists(uri)){
      File f = SD.open(uri, "r");
      server.streamFile(f, "audio/mpeg"); f.close(); return;
    }
   #else
    if (hasVoice && SD_MMC.exists(uri)){
      File f = SD_MMC.open(uri, "r");
      server.streamFile(f, "audio/mpeg"); f.close(); return;
    }
   #endif
    server.send(404, "text/plain", "no clip");        // portal falls back to TTS
    return;
  }
#endif
  redirectRoot();                                     // captive-portal catch-all
}

// ============================================================================
void setup(){
  Serial.begin(115200);
  seedCases();                         // fresh scenarios every boot
  prefs.begin("cyberchuck", false);    // onboard storage
  loadProgress();                      // restore completed stages
  ledInit();

#if USE_DISPLAY
  if (LCD_BL >= 0){
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, LCD_BL_ACTIVE_LOW ? LOW : HIGH);   // backlight ON
  }
  lcdSPI.begin(LCD_SCLK, LCD_MISO, LCD_MOSI, LCD_CS);       // bind display SPI pins
  gfx->initR(INITR_MINI160x80);                             // 0.96" 80x160 panel
  gfx->invertDisplay(LCD_IPS);                              // IPS needs inversion on
  gfx->setRotation(0);                                      // portrait 80x160
  // This panel is BGR, but INITR_MINI160x80 assumes RGB (cyan came out yellow).
  // Re-send MADCTL with raw values (avoids ST77XX_/ST7735_ naming differences):
  // 0x36 = MADCTL register; 0xC8 = MX(0x40) | MY(0x80) | BGR(0x08), rotation-0.
  { uint8_t madctl = 0xC8; gfx->sendCommand(0x36, &madctl, 1); }
  C_CYAN  = RGB565(0x38, 0xD6, 0xFF);
  C_GREEN = RGB565(0x3F, 0xF0, 0xA0);
  C_AMBER = RGB565(0xFF, 0xB4, 0x54);
  C_RED   = RGB565(0xFF, 0x64, 0x72);
  C_DIM   = RGB565(0x64, 0x7A, 0x8C);
  C_WHITE = 0xFFFF;
  drawBootScreen();
#endif

#if USE_SD
 #if SD_MODE_SPI
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (SD.begin(SD_CS, sdSPI)){
    hasVoice = SD.exists("/voice");
    Serial.println(hasVoice ? "SD: /voice found" : "SD: mounted, no /voice");
  } else {
    Serial.println("SD: not mounted (voice disabled) - check SD_* pins");
  }
 #else
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
  if (SD_MMC.begin("/sdcard", true /*1-bit*/)){
    hasVoice = SD_MMC.exists("/voice");
    Serial.println(hasVoice ? "SD: /voice found" : "SD: mounted, no /voice");
  } else {
    Serial.println("SD: not mounted (voice disabled) - check SD_MMC_* pins");
  }
 #endif
#endif

  // Wi-Fi access point (open, 2.4GHz for max phone compatibility)
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID);
  Serial.print("AP up: "); Serial.println(WiFi.softAPIP());

  dns.start(53, "*", apIP);                            // wildcard DNS -> us

  server.on("/",             handleRoot);
  server.on("/api/state",    handleState);
  server.on("/api/newcase",  HTTP_POST, handleNewcase);
  server.on("/api/custom",   HTTP_POST, handleCustom);
  server.on("/api/attempt",  HTTP_POST, handleAttempt);
  server.on("/api/sweep",    HTTP_POST, handleSweep);
  server.on("/api/solved",   HTTP_POST, handleSolved);
  server.on("/api/reset",    HTTP_POST, handleReset);
  server.on("/api/clear",    HTTP_POST, handleClearProgress);
  server.on("/api/status",   HTTP_POST, handleStatus);
  // OS captive-portal probes -> bounce to the lesson so it auto-pops
  const char *probes[] = {"/generate_204","/gen_204","/hotspot-detect.html",
                          "/ncsi.txt","/connecttest.txt","/redirect",
                          "/canonical.html","/success.txt","/library/test/success.html"};
  for (auto p : probes) server.on(p, redirectRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  setDeviceStatus("idle");
}

void loop(){
  dns.processNextRequest();
  server.handleClient();
}
