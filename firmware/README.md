# Firmware — which file do I flash?

Pick the folder for YOUR dongle. Each is a complete, ready-to-flash sketch with
the correct board already selected — no editing needed.

| Your board | Open this | Arduino board setting |
|---|---|---|
| **LILYGO T-Dongle-S3** (ESP32-S3) | `CyberChuck_Sandbox_S3/CyberChuck_Sandbox_S3.ino` | ESP32S3 Dev Module |
| **LILYGO T-Dongle-C5** (ESP32-C5) | `CyberChuck_Sandbox_C5/CyberChuck_Sandbox_C5.ino` | ESP32C5 Dev Module |

Not sure which you have? The S3 is the far more common one. Check the print on the
board or your order.

Both need the same two libraries (Library Manager):
**Adafruit ST7735 and ST7789 Library** + **Adafruit GFX Library**.

## `CyberChuck_Sandbox/` — dev source (optional)

The same firmware split into `.ino` + `pin_config.h` + `portal.h` + `logo_data.h`,
with a `BOARD_S3` / `BOARD_C5` toggle at the top of `pin_config.h`. Use this if you
want to read/edit the code in pieces. The two labeled sketches above are generated
from it, so you don't need this to just flash.
