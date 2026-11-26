# ECNG 3006 - Lab 3: Embedded System: Signal and System Integrity

**Target Platform:** Arduino MKR Zero (`ATSAMD21G18A`, ARM Cortex-M0+)  
**Programming Interface:** CMSIS / Register-level (`sam.h`)

## Selected Function (Question 1)
- [x] **Absolute Timing Reference** — Internal `RTC` peripheral on `ATSAMD21G18A` clocked by the Arduino MKR Zero's onboard 32.768 kHz crystal (`XOSC32K` on `PA00`/`PA01`)
- [ ] Analog to Digital Converter (ADC)
- [ ] Non-Volatile Storage (NVM / Flash / SD)

## Structure (PlatformIO)
- [platformio.ini](platformio.ini) — PlatformIO environment (`atmelsam`, `board = mkrzero`, `framework = arduino`, `upload_protocol = sam-ba`).
- [include/rtc_driver.h](include/rtc_driver.h) — C header prototyping the 5 RTC routines (`init_rtc`, `start_rtc`, `stop_rtc`, `singleread_rtc`, `singlewrite_rtc`).
- [src/rtc_driver.c](src/rtc_driver.c) — Register-level `<sam.h>` C implementation for `ATSAMD21G18A` `RTC` Mode 0 + `XOSC32K` via `GCLK2`.
- [include/rtc_unit_test.h](include/rtc_unit_test.h) / [src/rtc_unit_test.c](src/rtc_unit_test.c) — Pure C unit test verifying RTC stop, write, readback, start, and live counting.
- [src/main.c](src/main.c) — Pure `<sam.h>` C runner for Arduino MKR Zero calling the C unit tests and toggling `PB08` on each passing read/write cycle (no `Arduino.h`).
- [bin/firmware.bin](bin/firmware.bin) — Compiled application binary (`11,824` bytes).
- [evidence/ECNG_3006_LAB_3_Q_2.mp4](evidence/ECNG_3006_LAB_3_Q_2.mp4) — Question 2 build, upload, and hardware execution proof video (`~2.83 MB`).

## Build & Upload (PlatformIO)
- Build: `pio run` (outputs `.pio/build/mkrzero/firmware.bin`)
- Upload via bootloader: `pio run -t upload`
- Serial Monitor: `pio device monitor` (`115200` baud)
