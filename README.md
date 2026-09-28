# CyberChuck // Hacking 101

A self-contained, **offline** pen-testing learning tool that runs on a LILYGO
T-Dongle (**S3 or C5**). Plug it into any USB port for power — it shows the
CyberChuck logo, then broadcasts an open Wi-Fi AP with a captive portal that opens
the lesson on your phone. Everything you attack lives inside the dongle. It has no
internet uplink and phones no one home.

> **For practice only. No certification.** Guessing a login you don't own is a
> crime — keep it in the sandbox. All names and scenarios in the lessons are
> fictional.

## Install (no software needed)

One-click web installer (Chrome/Edge on desktop):

**→ https://cyberchuck01.github.io/CyberChuck-Hacking101/**

Pick your board, choose the serial port, wait ~1 minute, then unplug/replug and
join the dongle's `CyberChuck_Sandbox` Wi-Fi.

*(Maintainers: the installer needs the compiled `.bin` files added to `docs/` —
see [`docs/BUILD_AND_ADD_BINARIES.md`](docs/BUILD_AND_ADD_BINARIES.md).)*

## Lesson 1 — Password guessing

You're handed a fake client scenario — a name, an account, and "recon notes" like
a pet's name and birth year — and you crack the password they built from it, the
way weak passwords actually fall. Four replayable difficulties:

1. **Learn (guided)** — an animated terminal walkthrough (slow-down / pause / rewind).
2. **Easy** — name + year, with a pattern hint.
3. **Medium** — two facts mashed together.
4. **Hard** — leet swaps + a symbol, recon clues only.

Plus a **Custom case** demo mode: type in a target's details and show how guessable
that password style is. Type guesses (physical keyboard works), or after 10 manual
tries unlock the **wordlist attack** that mangles the recon into candidates.

Sound effects, a crack timer, and on-device progress that survives unplugging.

## Build from source

Firmware lives in [`firmware/`](firmware/) — **one ready-to-flash sketch per board:**

- `firmware/CyberChuck_Sandbox_S3/` — flash for the **T-Dongle-S3**
- `firmware/CyberChuck_Sandbox_C5/` — flash for the **T-Dongle-C5**
- `firmware/CyberChuck_Sandbox/` — dev source (split files + `BOARD_S3`/`BOARD_C5` toggle)

Each labeled sketch already has the correct board selected. Libraries: **Adafruit
ST7735 and ST7789 Library** + **Adafruit GFX Library**. See
[`firmware/README.md`](firmware/README.md) and `docs/BUILD_AND_ADD_BINARIES.md`.

## Hardware notes

- **Display:** 0.96" ST7735 (80×160), driven by Adafruit ST7735 over the dongle's
  FSPI bus. The panel is BGR (handled in firmware).
- **S3 vs C5:** different pins, different LED method (S3 LED is its own bus; C5
  shares the display bus), different SD (S3 = SDMMC, C5 = SPI). All handled by the
  board toggle.
- **No true BadUSB:** the C5 can't do USB HID; the "it just comes up" experience is
  the captive portal, which works identically on both boards.

## Roadmap

- **v0.1** — Lesson 1: password guessing from recon, 4 difficulties + custom mode.
- Next — Lesson 2: network recon → login attack.
- Later — traffic sniffing (Wireshark) and enumeration (Nmap) lessons.

## License

MIT — do what you like, keep it ethical. See `LICENSE`.
