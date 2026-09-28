// portal.h -- the entire lesson UI, embedded in flash and served from RAM-free
// PROGMEM. No external fonts/scripts/images: it must run on an offline AP.
#pragma once
#include <Arduino.h>

const char PORTAL_HTML[] PROGMEM = R"=====(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<title>CyberChuck // Hacking 101</title>
<style>
:root{
  --bg:#05080d; --panel:#0a121d; --edge:#123047; --edge2:#0e2233;
  --cyan:#38d6ff; --cyanDim:#1c7ea0; --ink:#d3e3f0; --dim:#647a8c;
  --ok:#3ff0a0; --warn:#ffb454; --bad:#ff6472;
  --mono:ui-monospace,"Cascadia Code","JetBrains Mono",Consolas,"Courier New",monospace;
}
*{box-sizing:border-box}
html,body{height:100%}
body{
  margin:0;background:
    radial-gradient(120% 60% at 50% -10%, #0b1b2a 0%, transparent 60%), var(--bg);
  color:var(--ink);font-family:var(--mono);
  padding-top:env(safe-area-inset-top,0);padding-bottom:env(safe-area-inset-bottom,0);
  -webkit-text-size-adjust:100%;
}
.wrap{max-width:640px;margin:0 auto;min-height:100%;display:flex;flex-direction:column}
header{
  display:flex;align-items:center;justify-content:space-between;
  padding:12px 14px;border-bottom:1px solid var(--edge);
  background:linear-gradient(#0a1622,#070f18);position:sticky;top:0;z-index:5;
}
.brand{font-weight:800;letter-spacing:.5px;font-size:15px}
.brand b{color:var(--cyan);text-shadow:0 0 12px rgba(56,214,255,.45)}
.brand span{color:var(--dim);font-weight:600}
.stat{display:flex;align-items:center;gap:8px;font-size:12px;color:var(--dim)}
.dot{width:9px;height:9px;border-radius:50%;background:var(--cyanDim);box-shadow:0 0 8px var(--cyan)}
.dot.attack{background:var(--warn);box-shadow:0 0 10px var(--warn);animation:pulse 1s infinite}
.dot.solved{background:var(--ok);box-shadow:0 0 12px var(--ok)}
@keyframes pulse{50%{opacity:.35}}
main{flex:1;padding:16px 14px 8px}
footer{padding:10px 14px 16px;font-size:11px;line-height:1.5;color:var(--dim);border-top:1px solid var(--edge2)}
footer b{color:var(--warn)}
.screen{display:none}
.screen.on{display:block}
h1{font-size:20px;margin:.2em 0 .1em;letter-spacing:.5px}
h2{font-size:15px;margin:0 0 10px;color:var(--cyan);font-weight:700}
p{line-height:1.55;color:#b7c8d6}
.rune{color:var(--cyanDim);letter-spacing:6px;font-size:12px;margin:4px 0 14px}
.btn{
  font-family:var(--mono);font-size:14px;font-weight:700;cursor:pointer;
  color:#04121a;background:var(--cyan);border:0;border-radius:8px;padding:11px 16px;
  box-shadow:0 0 18px rgba(56,214,255,.25);
}
.btn:active{transform:translateY(1px)}
.btn.ghost{background:transparent;color:var(--cyan);border:1px solid var(--edge)}
.btn.small{padding:7px 10px;font-size:12px}
.btn:disabled{opacity:.35;cursor:not-allowed;box-shadow:none}
.stages{display:flex;flex-direction:column;gap:10px;margin-top:8px}
.stage{
  display:flex;align-items:center;gap:12px;text-align:left;width:100%;
  background:var(--panel);border:1px solid var(--edge);border-radius:10px;padding:12px 14px;
  color:var(--ink);font-family:var(--mono);cursor:pointer;
}
.stage:disabled{opacity:.4;cursor:not-allowed}
.stage .n{font-size:18px;font-weight:800;color:var(--cyan);min-width:22px}
.stage .t{font-weight:700}
.stage .d{font-size:12px;color:var(--dim);margin-top:2px}
.stage .chk{margin-left:auto;color:var(--ok);font-weight:800}
#term{
  background:#040a10;border:1px solid var(--edge);border-radius:10px;
  padding:12px;height:46vh;min-height:230px;overflow-y:auto;
  font-size:13px;line-height:1.5;white-space:pre-wrap;word-break:break-word;
  box-shadow:inset 0 0 40px rgba(0,40,60,.25);
}
#term .sys{color:var(--cyanDim)}
#term .cmd{color:var(--cyan)}
#term .ok{color:var(--ok)}
#term .bad{color:var(--bad)}
#term .deny{color:var(--dim)}
#term .info{color:#b7c8d6}
.cursor::after{content:"\2588";color:var(--cyan);animation:bl 1s steps(1) infinite}
@keyframes bl{50%{opacity:0}}
.bar{display:flex;flex-wrap:wrap;gap:6px;align-items:center;margin-top:10px}
.seg{display:flex;border:1px solid var(--edge);border-radius:8px;overflow:hidden}
.seg button{background:transparent;color:var(--dim);border:0;padding:7px 9px;font-family:var(--mono);font-size:12px;cursor:pointer}
.seg button.sel{background:var(--edge);color:var(--cyan)}
.spd{margin-left:auto;font-size:11px;color:var(--dim)}
.hint{background:var(--edge2);border-left:3px solid var(--cyan);border-radius:6px;padding:10px 12px;margin:12px 0;font-size:13px;color:#bcd0de}
.hint code{color:var(--cyan)}
.pad{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;max-width:240px;margin-top:12px}
.pad button{padding:14px 0;font-size:18px;font-weight:700;font-family:var(--mono);background:var(--panel);color:var(--ink);border:1px solid var(--edge);border-radius:10px;cursor:pointer}
.pad button:active{background:var(--edge)}
.pad .wide{grid-column:span 1}
.entry{font-size:26px;letter-spacing:10px;text-align:center;color:var(--cyan);margin:14px 0 2px;min-height:34px;text-shadow:0 0 14px rgba(56,214,255,.4)}
.row{display:flex;gap:8px;flex-wrap:wrap;margin-top:12px}
.win{text-align:center;padding:10px 0}
.win h1{color:var(--ok);text-shadow:0 0 20px rgba(63,240,160,.5);font-size:24px}
.grant{font-size:12px;color:var(--ok);white-space:pre;line-height:1.15;margin:14px 0;overflow-x:auto}
a{color:var(--cyan)}
:focus-visible{outline:2px solid var(--cyan);outline-offset:2px}
@media (prefers-reduced-motion:reduce){.dot.attack{animation:none}.cursor::after{animation:none}}
@keyframes shake{10%,90%{transform:translateX(-1px)}30%,70%{transform:translateX(-5px)}40%,60%{transform:translateX(4px)}50%{transform:translateX(5px)}}
.shake{animation:shake .38s}
@keyframes flashok{0%{box-shadow:inset 0 0 0 2px var(--ok),inset 0 0 70px rgba(63,240,160,.55)}100%{box-shadow:inset 0 0 40px rgba(0,40,60,.25)}}
.flashok{animation:flashok .7s}
#mute{padding:5px 8px}
@media (prefers-reduced-motion:reduce){.shake,.flashok{animation:none}}
#scenario{margin-top:12px}
.scen{background:var(--panel);border:1px solid var(--edge);border-radius:8px;padding:10px 12px;font-size:13px}
.scen .lab{color:var(--dim);font-size:11px;letter-spacing:1px;margin-right:8px}
.scen b{color:var(--cyan)}
.reconhdr{color:var(--cyanDim);font-size:11px;letter-spacing:1px;margin:12px 0 6px}
.clue{font-size:13px;color:#bcd0de;padding:2px 0}
.clue::before{content:"\25B8";color:var(--cyan);margin-right:8px}
.guessrow{display:flex;gap:8px;margin-top:14px}
#guess{flex:1;min-width:0;font-family:var(--mono);font-size:16px;color:var(--cyan);
  background:#040a10;border:1px solid var(--edge);border-radius:8px;padding:11px 12px;
  letter-spacing:1px;box-shadow:inset 0 0 24px rgba(0,40,60,.25)}
#guess::placeholder{color:var(--dim);letter-spacing:0}
.form{display:flex;flex-direction:column;gap:10px;margin-top:12px}
.form label{display:flex;flex-direction:column;gap:4px;font-size:12px;color:var(--dim)}
.form input,.form select{font-family:var(--mono);font-size:15px;color:var(--ink);
  background:#040a10;border:1px solid var(--edge);border-radius:8px;padding:9px 11px}
.form input:focus,.form select:focus{outline:none;border-color:var(--cyan)}
</style>
</head>
<body>
<div class="wrap">
  <header>
    <div class="brand"><b>CYBER</b>CHUCK <span>// hacking 101</span></div>
    <div class="stat">
      <button class="btn ghost small" id="mute" onclick="toggleSound()" title="sound on/off">&#128266;</button>
      <span id="statTxt">idle</span><span class="dot" id="dot"></span>
    </div>
  </header>

  <main>
    <!-- BOOT -->
    <section class="screen on" id="boot">
      <h1>Welcome to the sandbox.</h1>
      <div class="rune">&#5792; PRIVACY IS CURRENCY &#5792;</div>
      <p>This little dongle is a self-contained hacking range. Everything you attack
      lives inside it &mdash; nothing here touches the internet or any real system.</p>
      <p>Lesson 1: <b style="color:var(--cyan)">cracking a password from clues</b>. Four
      difficulties, each with less hand-holding than the last. Take your time &mdash; you
      can slow the walkthrough down to any pace you like.</p>
      <div class="row"><button class="btn" onclick="go('home')">Start &raquo;</button></div>
    </section>

    <!-- MAIN MENU (lessons) -->
    <section class="screen" id="home">
      <h2>Main menu</h2>
      <div class="stages" id="lessons"></div>
      <div class="row" style="margin-top:14px">
        <button class="btn ghost small" onclick="resetProgress()">Reset progress</button>
      </div>
      <p style="font-size:12px;color:var(--dim);margin-top:10px">
        Progress saves to the dongle automatically &mdash; unplug it and pick up where you left off.</p>
    </section>

    <!-- MENU -->
    <section class="screen" id="menu">
      <h2>Lesson 1 &mdash; Password guessing</h2>
      <div class="stages" id="stages"></div>
      <div class="row" style="margin-top:12px">
        <button class="btn ghost small" onclick="go('custom')">&#43; Custom case</button>
        <button class="btn ghost small" onclick="go('home')">&laquo; All lessons</button>
      </div>
    </section>

    <!-- CUSTOM CASE -->
    <section class="screen" id="custom">
      <h2>Custom case</h2>
      <p style="font-size:13px;color:#b7c8d6;line-height:1.55">Enter a target's details the
        way recon would surface them. The sandbox builds a password from them so you can show
        how guessable that style is. <b style="color:var(--warn)">Use fake or consenting-demo
        data only.</b></p>
      <div class="form">
        <label>Name<input id="c_name" type="text" autocomplete="off" placeholder="Alex Rivera"></label>
        <label>Pet's name<input id="c_pet" type="text" autocomplete="off" placeholder="Bella"></label>
        <label>Kid's name<input id="c_kid" type="text" autocomplete="off" placeholder="Noah"></label>
        <label>Birth year<input id="c_year" type="text" inputmode="numeric" autocomplete="off" placeholder="1990"></label>
        <label>Favorite team<input id="c_team" type="text" autocomplete="off" placeholder="Lakers"></label>
        <label>City<input id="c_city" type="text" autocomplete="off" placeholder="Denver"></label>
        <label>Difficulty
          <select id="c_diff">
            <option value="1">Easy &mdash; name + year</option>
            <option value="2">Medium &mdash; two facts mashed</option>
            <option value="3">Hard &mdash; leet + symbol</option>
          </select>
        </label>
      </div>
      <div class="row" style="margin-top:12px">
        <button class="btn" onclick="buildCustomCase()">Build case &raquo;</button>
        <button class="btn ghost" onclick="go('menu')">Back</button>
      </div>
    </section>

    <!-- STAGE -->
    <section class="screen" id="stage">
      <h2 id="stageTitle"></h2>
      <div id="term"></div>
      <div class="bar" id="playbar">
        <button class="btn ghost small" id="bPlay" onclick="P.toggle()">&#9654; Play</button>
        <button class="btn ghost small" id="bBack" onclick="P.step(-1)">&#9198;</button>
        <button class="btn ghost small" id="bFwd" onclick="P.step(1)">&#9197;</button>
        <button class="btn ghost small" onclick="P.restart()">&#8635; Restart</button>
        <div class="seg" id="speed"></div>
        <span class="spd" id="spdTxt">1&times;</span>
      </div>

      <div id="attack">
        <div id="scenario"></div>
        <div id="clues"></div>
        <div id="hints"></div>
        <div class="guessrow">
          <input id="guess" type="text" inputmode="text" autocomplete="off"
                 autocapitalize="off" autocorrect="off" spellcheck="false"
                 placeholder="type the password" />
          <button class="btn" onclick="submitGuess()">Try</button>
        </div>
        <div style="font-size:11px;color:var(--dim);margin-top:8px">
          Type a guess and hit Enter. Read the recon &mdash; the password is built from it.</div>
        <div class="row">
          <button class="btn" id="bAuto" onclick="autoAttack()">&#9889; Run wordlist attack</button>
          <button class="btn ghost" onclick="newCase()">New case</button>
          <button class="btn ghost" onclick="stopAll()">Stop</button>
          <button class="btn ghost" onclick="go('menu')">Menu</button>
        </div>
      </div>
    </section>

    <!-- WIN -->
    <section class="screen" id="win">
      <div class="win">
        <h1>ACCESS GRANTED</h1>
        <div class="grant" id="grant"></div>
        <p>Lesson 1 complete. You cracked every difficulty &mdash; even Hard with no pattern hint.</p>
        <p style="color:var(--dim)">More lessons ship in future firmware drops &mdash;
        traffic sniffing and network enumeration are next.</p>
        <div class="row" style="justify-content:center">
          <button class="btn" onclick="go('home')">Back to lessons</button>
        </div>
      </div>
    </section>
  </main>

  <footer>
    <b>For practice only.</b> No certificate, no score that means anything &mdash; just reps.
    Brute forcing a login you don&rsquo;t own is a crime. Keep it in the sandbox.
    Open source: inspect the firmware, this box phones no one home.
  </footer>
</div>

<script>
const $=s=>document.querySelector(s);
const TERM=$('#term');
let ST={ssid:'CyberChuck_Sandbox',hasVoice:false,solved:[false,false,false,false],version:'0.1'};
let cur=0;                 // current stage index
const STAGES=[
 {t:'Learn (guided)', d:'How recon turns into a cracked password.'},
 {t:'Easy',           d:'Name + year. The clues point right at it.'},
 {t:'Medium',         d:'Two facts mashed together, capitalized.'},
 {t:'Hard',           d:'Leet swaps + a symbol. Recon only, no pattern hint.'},
];

// ---------- screen routing ----------
function go(id){
  document.querySelectorAll('.screen').forEach(s=>s.classList.remove('on'));
  $('#'+id).classList.add('on');
  if(id==='home') renderHome();
  if(id==='menu') renderMenu();
  if(id==='stage') enterStage();
  window.scrollTo(0,0);
}
const LESSONS=[
 {id:1,t:'Password guessing', d:'Crack a client\'s password from recon. 4 difficulties.', ready:true},
 {id:2,t:'Login brute force',  d:'Coming in a future drop.',         ready:false},
 {id:3,t:'Traffic sniffing',   d:'Wireshark basics. Coming soon.',   ready:false},
 {id:4,t:'Network enumeration',d:'Nmap basics. Coming soon.',        ready:false},
];
async function renderHome(){
  try{ const s=await (await fetch('/api/state')).json(); Object.assign(ST,s);}catch(e){}
  const box=$('#lessons'); box.innerHTML='';
  LESSONS.forEach(L=>{
    const done = L.id===1 ? ST.solved.filter(Boolean).length : 0;
    const complete = L.id===1 && done===4;
    const b=document.createElement('button');
    b.className='stage'; b.disabled=!L.ready;
    b.innerHTML=`<span class="n">${L.id}</span><span><span class="t">${L.t}</span>
      <div class="d">${L.ready?L.d:'&#128274; '+L.d}</div></span>
      ${L.ready?`<span class="chk" style="color:${complete?'var(--ok)':'var(--dim)'}">${done}/4</span>`:''}`;
    if(L.ready) b.onclick=()=>{ if(L.id===1) go('menu'); };
    box.appendChild(b);
  });
}
async function resetProgress(){
  try{ await post('/api/clear',{}); }catch(e){}
  ST.solved=[false,false,false,false];
  renderHome();
}

function renderMenu(){
  const box=$('#stages'); box.innerHTML='';
  STAGES.forEach((s,i)=>{
    const locked = i>0 && !ST.solved[i-1];
    const b=document.createElement('button');
    b.className='stage'; b.disabled=locked;
    b.innerHTML=`<span class="n">${i+1}</span><span><span class="t">${s.t}</span>
      <div class="d">${locked?'Clear stage '+i+' to unlock':s.d}</div></span>
      ${ST.solved[i]?'<span class="chk">&#10003;</span>':''}`;
    b.onclick=()=>{cur=i; go('stage');};
    box.appendChild(b);
  });
}

// ---------- terminal ----------
function line(text,cls){const d=document.createElement('div');if(cls)d.className=cls;d.textContent=text;TERM.appendChild(d);TERM.scrollTop=TERM.scrollHeight;return d;}
function clearTerm(){TERM.innerHTML='';}
const reduce=matchMedia('(prefers-reduced-motion:reduce)').matches;

// ---------- playback engine (shared by walkthrough + live attack) ----------
const P={
  speed:1, playing:false, token:0, paused:false, resume:null,
  script:[], idx:0, mode:'script', audio:null,
  setSpeed(v){this.speed=v;$('#spdTxt').textContent=v+'\u00d7';
    document.querySelectorAll('#speed button').forEach(b=>b.classList.toggle('sel',+b.dataset.v===v));},
  gate(){return this.paused?new Promise(r=>this.resume=r):Promise.resolve();},
  async sleep(ms){const end=Date.now()+ms/this.speed;const t=this.token;
    while(Date.now()<end){await this.gate();if(t!==this.token)throw'abort';await new Promise(r=>setTimeout(r,30));}
    if(t!==this.token)throw'abort';},
  toggle(){ if(this.mode==='script'){ this.playing?this.pause():this.play(); }
            else { this.paused?this.unpause():this.pause(); } },
  pause(){this.paused=true;this.playing=false;$('#bPlay').innerHTML='&#9654; Play';
    if(this.audio)this.audio.pause();try{speechSynthesis.pause()}catch(e){}clearInterval(this._ka);},
  unpause(){this.paused=false;$('#bPlay').innerHTML='&#9208; Pause';
    if(this.audio)this.audio.play().catch(()=>{});try{speechSynthesis.resume()}catch(e){}
    if(this.resume){this.resume();this.resume=null}},
  stop(){this.token++;this.paused=false;this.playing=false;if(this.resume){this.resume();this.resume=null}
    if(this.audio){this.audio.pause();this.audio=null}try{speechSynthesis.cancel()}catch(e){}clearInterval(this._ka);},
  // scripted walkthrough
  load(script){this.mode='script';this.script=script;this.idx=0;},
  restart(){this.stop();clearTerm();this.idx=0;if(this.mode==='script')this.play();},
  play(){this.stop();this.paused=false;this.playing=true;$('#bPlay').innerHTML='&#9208; Pause';this.run();},
  async run(){
    const t=(this.token+1); this.token=t;
    try{
      for(;this.idx<this.script.length;this.idx++){
        if(t!==this.token)return;
        const s=this.script[this.idx];
        await this.render(s);
        if(s.voice) await this.waitVoice(6000);   // let the narration line finish
        await this.sleep(s.after||300);
      }
      this.playing=false;$('#bPlay').innerHTML='&#9654; Play';
    }catch(e){/*aborted*/}
  },
  async waitVoice(capMs){
    const t=this.token, end=Date.now()+capMs;
    await new Promise(r=>setTimeout(r,140));      // let it start
    while(Date.now()<end){
      if(t!==this.token) throw 'abort';
      const busy = (this.audio && !this.audio.paused && !this.audio.ended)
                || (('speechSynthesis' in window) && (speechSynthesis.speaking||speechSynthesis.pending));
      if(!busy) break;
      await this.gate(); if(t!==this.token) throw 'abort';
      await new Promise(r=>setTimeout(r,80));
    }
  },
  step(dir){ if(this.mode!=='script')return;
    this.stop();
    this.idx=Math.max(0,Math.min(this.script.length-1,this.idx+dir));
    clearTerm();
    for(let i=0;i<this.idx;i++) this.renderInstant(this.script[i]);
    this.render(this.script[this.idx]); },
  renderInstant(s){ if(s.clear){clearTerm();return;} if(s.text!=null) line(s.text,s.cls); },
  async render(s){
    if(s.clear){clearTerm();}
    if(s.voice) this.speak(s.voice,s.text);
    if(s.text==null)return;
    if(reduce||s.instant){line(s.text,s.cls);return;}
    const el=line('',s.cls); el.classList.add('cursor');
    const txt=s.text; const per=(s.typ||16);
    for(let i=0;i<txt.length;i++){await this.sleep(per);el.textContent=txt.slice(0,i+1);TERM.scrollTop=TERM.scrollHeight;}
    el.classList.remove('cursor');
  },
  speak(key,text){
    if(ST.hasVoice){
      const a=new Audio('/voice/'+key+'.mp3'); this.audio=a;
      a.onerror=()=>this.tts(text); a.play().catch(()=>this.tts(text));
    } else this.tts(text);
  },
  tts(text){ if(!text)return; if(!('speechSynthesis' in window))return;
    try{
      speechSynthesis.cancel();
      const u=new SpeechSynthesisUtterance(text.replace(/[>_#]/g,' ').replace(/\s+/g,' ').trim());
      u.rate=0.95; u.pitch=0.6;    // fixed + clear; the speed slider paces the visuals, not the voice
      const vs=speechSynthesis.getVoices();
      if(vs&&vs.length){
        u.voice = vs.find(v=>/en/i.test(v.lang)&&/(male|david|daniel|fred|rishi|google uk english male)/i.test(v.name))
               || vs.find(v=>/en/i.test(v.lang)) || vs[0];
      }
      // Chrome/Android kills long speech after ~15s unless nudged
      u.onstart=()=>{ clearInterval(this._ka); this._ka=setInterval(()=>{try{speechSynthesis.resume()}catch(e){}},7000); };
      u.onend=u.onerror=()=>{ clearInterval(this._ka); };
      speechSynthesis.speak(u);
    }catch(e){} }
};
// speed control buttons
[0.25,0.5,1,2,4].forEach(v=>{const b=document.createElement('button');b.textContent=v+'\u00d7';b.dataset.v=v;b.onclick=()=>P.setSpeed(v);$('#speed').appendChild(b);});
P.setSpeed(1);

// ---------- stage entry ----------
let caseData=null;
const DIFF_LABEL=['Learn (guided)','Easy','Medium','Hard','Custom'];

async function enterStage(){
  $('#stageTitle').textContent = `Lesson 1 \u00b7 ${DIFF_LABEL[cur]}`;
  clearTerm(); const g=$('#guess'); if(g)g.value=''; P.stop();
  stageStart=Date.now(); stageTries=0;
  const guided = cur===0;
  $('#playbar').style.display = guided ? 'flex':'none';
  setAutoBtn();
  if(cur===4){                                   // custom case (already built)
    renderCase(caseData);
    P.mode='live';
    line('custom case built.','sys');
    line('crack '+(caseData?caseData.name:'the target')+"'s password from their own details.",'info');
    if(!autoUnlocked) line('wordlist attack unlocks after '+AUTO_UNLOCK_AT+' manual tries.','deny');
  } else {
    await newCase(true);
    if(guided){ P.load(lesson1Script()); P.play(); }
    else {
      P.mode='live';
      line('sandbox ready.','sys');
      line('read the recon below, then guess '+(caseData?caseData.name:'the target')+"'s password.",'info');
      if(!autoUnlocked) line('wordlist attack unlocks after '+AUTO_UNLOCK_AT+' manual tries.','deny');
    }
  }
  if(g) g.focus();
}

async function buildCustomCase(){
  const v=id=>$('#'+id).value.trim();
  const data={name:v('c_name'),pet:v('c_pet'),kid:v('c_kid'),year:v('c_year'),
              team:v('c_team'),city:v('c_city'),diff:$('#c_diff').value};
  try{ caseData=await post('/api/custom',data); }catch(e){ caseData=null; }
  cur=4; go('stage');
}

async function newCase(quiet){
  if(cur===4){ go('custom'); return; }           // custom "New case" -> re-edit the form
  try{ caseData = await post('/api/newcase',{stage:cur}); }
  catch(e){ caseData=null; }
  renderCase(caseData);
  if(!quiet){ clearTerm(); line('new case loaded \u2014 '+(caseData?caseData.name:'?'),'sys'); stageStart=Date.now(); stageTries=0; }
}

function renderCase(c){
  if(!c){ $('#scenario').innerHTML=''; $('#clues').innerHTML=''; $('#hints').innerHTML=''; return; }
  $('#scenario').innerHTML =
    `<div class="scen"><span class="lab">TARGET</span><b>${c.name}</b> &middot; ${c.acct}</div>`;
  $('#clues').innerHTML =
    '<div class="reconhdr">// recon notes</div>' + c.clues.map(x=>`<div class="clue">${x}</div>`).join('');
  $('#hints').innerHTML = c.hint ? `<div class="hint">${c.hint}</div>` : '';
}

// ---------- talking to the sandbox ----------
async function post(url,data){
  const body=Object.entries(data).map(([k,v])=>k+'='+encodeURIComponent(v)).join('&');
  const r=await fetch(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
  return r.json();
}
// ---------- fun: sound, flair, gating ----------
const SFX={ on:true, ctx:null,
  ac(){ if(!this.ctx){ try{this.ctx=new (window.AudioContext||window.webkitAudioContext)();}catch(e){} }
        if(this.ctx&&this.ctx.state==='suspended'){ this.ctx.resume().catch(()=>{}); } return this.ctx; },
  tone(f,dur,type,vol){ if(!this.on)return; const c=this.ac(); if(!c)return;
    const o=c.createOscillator(),g=c.createGain(); o.type=type||'square'; o.frequency.value=f;
    const t=c.currentTime; g.gain.setValueAtTime(vol||0.05,t); o.connect(g); g.connect(c.destination);
    o.start(t); g.gain.exponentialRampToValueAtTime(0.0001,t+(dur||0.08)); o.stop(t+(dur||0.08)+0.02); },
  blip(){ this.tone(660,0.05,'square',0.04); },
  deny(){ this.tone(150,0.2,'sawtooth',0.06); setTimeout(()=>this.tone(110,0.2,'sawtooth',0.05),90); },
  tick(){ this.tone(380,0.02,'square',0.02); },
  win(){ [523,659,784,1047].forEach((f,i)=>setTimeout(()=>this.tone(f,0.15,'triangle',0.06),i*110)); },
};
function toggleSound(){ SFX.on=!SFX.on; $('#mute').innerHTML=SFX.on?'&#128266;':'&#128263;'; if(SFX.on)SFX.blip(); }
const DENIES=['access denied','nope','denied','not even close','the lock laughs','try harder, human',
  'nice try','rejected','the mainframe is unimpressed','wrong. again.','denied. keep going.'];
function shake(el){ if(!el)return; el.classList.remove('shake'); void el.offsetWidth; el.classList.add('shake'); }
function flashOk(){ TERM.classList.remove('flashok'); void TERM.offsetWidth; TERM.classList.add('flashok'); }

let manualCount=0, autoUnlocked=false, stageStart=0, stageTries=0;
const AUTO_UNLOCK_AT=10;
function setAutoBtn(){
  const b=$('#bAuto'); if(!b)return;
  if(autoUnlocked){ b.disabled=false; b.innerHTML='&#9889; Run auto-attack'; }
  else { b.disabled=true; b.innerHTML='&#128274; Auto locked &middot; '+Math.min(manualCount,AUTO_UNLOCK_AT)+'/'+AUTO_UNLOCK_AT; }
}
function bumpManual(){
  manualCount++;
  if(!autoUnlocked && manualCount>=AUTO_UNLOCK_AT){
    autoUnlocked=true; line('','info');
    line('>> auto-attack UNLOCKED \u2014 you put in the reps. easy button earned.','ok'); SFX.win();
  }
  setAutoBtn();
}
// physical keyboard: Enter submits the password field
document.addEventListener('keydown',e=>{
  if(!$('#stage').classList.contains('on')) return;
  if(e.key==='Enter' && document.activeElement===$('#guess')){ e.preventDefault(); submitGuess(); }
});

async function submitGuess(){
  const g=$('#guess'); if(!g) return;
  const guess=g.value.trim(); if(!guess) return;
  g.value=''; line('> try '+guess,'cmd'); stageTries++;
  try{
    const r=await post('/api/attempt',{stage:cur,guess});
    if(r.correct){ onSolved(guess); return; }
    line('  '+DENIES[Math.floor(Math.random()*DENIES.length)],'bad');
    SFX.deny(); shake($('#guess')); bumpManual();
  }catch(e){ line('  (no response from target)','bad'); }
  g.focus();
}

let running=false;
async function autoAttack(){
  if(running)return;
  if(!autoUnlocked){ line('wordlist attack locked \u2014 '+(AUTO_UNLOCK_AT-manualCount)+' more manual tries to go.','deny'); SFX.deny(); return; }
  stopAll(); running=true;
  setStatus('attack'); P.mode='live'; P.paused=false; P.token++;
  const t=P.token; $('#bAuto').disabled=true;
  line('','info'); line('# building wordlist from recon, mangling & trying...','sys');
  let res;
  try{ res=await post('/api/sweep',{stage:cur}); }
  catch(e){ line('  (target unreachable)','bad'); running=false; setAutoBtn(); return; }
  stageTries += res.tries;
  try{
    for(const cand of res.tried){
      if(t!==P.token) throw 'abort';
      const hit=(cand===res.password);
      line('  '+cand+(hit?'  ... MATCH':'  ... nope'), hit?'ok':'deny');
      SFX.tick();
      await P.sleep(300);
      if(hit) break;
    }
    onSolved(res.password);
  }catch(e){ line('  [stopped]','deny'); }
  running=false; setAutoBtn();
}

function onSolved(pw){
  setStatus('solved');
  const secs=((Date.now()-stageStart)/1000).toFixed(1);
  line('','info');
  line('ACCESS GRANTED  \u2014  password was  '+pw,'ok');
  line('  cracked in '+stageTries+' tries / '+secs+'s','info');
  SFX.win(); flashOk();
  P.stop();
  if(cur===4){                                   // custom demo: no progression
    line('','info');
    line("that's how fast a 'personal' password falls. long + random is the fix.",'sys');
    setTimeout(()=>setStatus('idle'), 400);
    return;
  }
  ST.solved[cur]=true;
  post('/api/solved',{stage:cur}).catch(()=>{});
  if(cur>=3){ setTimeout(showWin,1100); }
  else {
    line('','info');
    line('stage '+(cur+1)+' cleared. stage '+(cur+2)+' unlocked.','sys');
    setTimeout(()=>go('menu'),1600);
  }
}

function showWin(){
  $('#grant').textContent =
`  ######  ######
  #    #  #
  #    #  #  ###
  ######  ######
     C Y B E R C H U C K
        g g   :)`;
  setStatus('idle'); go('win');
}

function stopAll(){ running=false; P.stop(); setStatus('idle'); }

// ---------- status light (mirrors the dongle screen/LED) ----------
function setStatus(s){
  const d=$('#dot'); d.className='dot'+(s==='idle'?'':' '+s);
  $('#statTxt').textContent=s;
  post('/api/status',{state:s}).catch(()=>{});
}

// ---------- boot ----------
async function init(){
  try{ const s=await (await fetch('/api/state')).json(); Object.assign(ST,s);}catch(e){}
  // warm up TTS voice list on some browsers
  try{ speechSynthesis.getVoices(); speechSynthesis.onvoiceschanged=()=>{}; }catch(e){}
}
init();

// ================= Lesson 1 walkthrough script =================
function lesson1Script(){
  return [
   {text:'>_ initializing lesson_01 :: password recon & guessing',cls:'sys',after:700,voice:'l1_00'},
   {text:'',after:200},
   {text:'Most people do not use random passwords. They use their life.',cls:'info',voice:'l1_01',after:1000},
   {text:'A pet\'s name. A kid. A birth year. A team they love.',cls:'info',voice:'l1_02',after:1000},
   {text:'',after:250},
   {text:'Step one is recon: quietly gather those personal facts.',cls:'info',voice:'l1_03',after:1000},
   {text:'Step two: mash the facts into likely passwords and try them fast.',cls:'info',voice:'l1_04',after:1100},
   {text:'',after:300},
   {text:'Say recon on "Sarah Kline" turns up a cat named Bella, born 2016.',cls:'sys',voice:'l1_05',after:1000},
   {text:'A cracker\'s first guesses write themselves:',cls:'info',voice:'l1_06',after:700},
   {text:'  bella ... nope',cls:'deny',after:260},
   {text:'  Bella2016 ... nope',cls:'deny',after:260},
   {text:'  bella16 ... nope',cls:'deny',after:260},
   {text:'  B3lla2016! ... nope',cls:'deny',after:260},
   {text:'  Bella2016 ... MATCH',cls:'ok',after:900,voice:'l1_07'},
   {text:'',after:300},
   {text:'One of those usually lands. Human passwords are predictable.',cls:'info',voice:'l1_08',after:1100},
   {text:'',after:300},
   {text:'Your live target is shown below with real recon notes.',cls:'sys',voice:'l1_09',after:900},
   {text:'Read the clues and figure out how they built the password.',cls:'info',voice:'l1_10',after:1000},
   {text:'Type a guess, or after 10 tries run the wordlist attack.',cls:'cmd',voice:'l1_11',after:900},
   {text:'Harder difficulties add mashups and l33t swaps. Pick them from the menu.',cls:'deny',voice:'l1_12',after:400},
  ];
}
</script>
</body>
</html>)=====";
