# Spaghettiboy-2000

Very simple Arduino Uno project (minimal tabs, minimal logic).

Files:
- `spaghettiboy-2000-project.ino` (all logic)
- `config.h` (pins and constants)

## Arduino IDE Quick Start

1. Open Arduino IDE 2.x
2. Open `spaghettiboy-2000-project.ino`
3. Tools -> Board -> Arduino AVR Boards -> Arduino Uno
4. Tools -> Port -> select your Uno port
5. Install libraries in Library Manager:
	- Adafruit GFX Library
	- Adafruit SSD1306
	- Adafruit SHT31 Library
6. Click Verify
7. Click Upload

## Wiring

From `config.h`:
- LDR -> A0
- Mode button -> D2 to GND (uses INPUT_PULLUP)
- Status LED -> onboard LED

I2C devices on Uno:
- SDA -> A4
- SCL -> A5
- OLED default address: 0x3C
- SHT31 default address: 0x44

## What The Code Does

Simple state machine:
- BOOT
- SHOW_ENV
- SHOW_DEBUG

Behavior:
- Boots for ~1.2s with blinking LED
- Reads sensor + LDR every 1s
- Button press switches between ENV and DEBUG screen
- If sensor read fails, display shows N/A

## Customizing

- Change pins and addresses in `config.h`
- Change read rate with `READ_INTERVAL_MS`
- Edit screen content directly in `spaghettiboy-2000-project.ino`
