# Adding the firmware binaries to the web installer

The web installer (`index.html`) flashes two files that are **not in the repo yet**
because they're compiled output:

- `docs/firmware_s3.bin`  — for the T-Dongle-S3
- `docs/firmware_c5.bin`  — for the T-Dongle-C5

Build each one and drop it in `docs/`. It's a 5-minute job per board.

## Make a binary (per board)

1. Open the sketch for the board you're building:
   - S3 build: `firmware/CyberChuck_Sandbox_S3/CyberChuck_Sandbox_S3.ino`
   - C5 build: `firmware/CyberChuck_Sandbox_C5/CyberChuck_Sandbox_C5.ino`
   (each already has the right board set — no toggle to flip)
3. Tools → Board:
   - **ESP32S3 Dev Module** (S3)  or  **ESP32C5 Dev Module** (C5)
   - USB CDC On Boot: **Enable**
   - (S3) USB Mode: **Hardware CDC and JTAG**
   - Flash Size: **16MB**
4. Install libraries if needed: **Adafruit ST7735 and ST7789 Library** +
   **Adafruit GFX Library**.
5. **Sketch → Export Compiled Binary.** When it finishes, open the sketch folder
   (Sketch → Show Sketch Folder). In the `build/…` output you'll find:
   ```
   CyberChuck_Sandbox_SINGLE.ino.merged.bin
   ```
   That `.merged.bin` already contains bootloader + partitions + app, ready to
   flash at offset 0 — exactly what the manifest expects.
6. Copy it into `docs/` and rename:
   - S3 → `docs/firmware_s3.bin`
   - C5 → `docs/firmware_c5.bin`
7. Repeat for the other board (flip the toggle, re-export).

## Publish the installer (GitHub Pages)

1. Push the repo to GitHub (`CyberChuck01/CyberChuck-Hacking101`).
2. Repo **Settings → Pages** → Source: **Deploy from a branch** → Branch: `main`,
   Folder: **/docs** → Save.
3. In a minute your installer is live at:
   ```
   https://cyberchuck01.github.io/CyberChuck-Hacking101/
   ```
   Share that link — anyone on Chrome/Edge can one-click flash their dongle.

## Notes

- The installer uses **ESP Web Tools** over **WebSerial**, so it only works in
  **Chrome or Edge on desktop** (not Firefox/Safari, not mobile).
- **C5 caveat:** ESP Web Tools flashes the C5 only if the visitor's browser build
  of `esptool-js` recognizes the `ESP32-C5` chip family. The S3 is universally
  supported; the C5 is newer, so on older browsers the C5 button may fail at chip
  detection until they update. Nothing you can fix in the repo — it's client-side.
- Re-exporting after a firmware change? Just replace the two `.bin` files and push;
  bump `"version"` in `manifest_s3.json` / `manifest_c5.json` so returning users
  re-flash.
