# Spaghettiboy-2000

Arduino Uno project with split modules for app flow, display rendering, and sensors.

## Current Project Structure

- `spaghettiboy-2000-project.ino`
	- Main app loop
	- Button navigation state machine
	- Sensor read timing
- `display.h` / `display.cpp`
	- OLED rendering with U8g2
	- UI states: splash, info, game
	- Dynamic contrast based on LDR value
- `sensors.h` / `sensors.cpp`
	- SHT31 initialization and readings
	- LDR analog read
	- Shared sensor globals
- `config.h`
	- Pins, I2C addresses, timing, serial baud

## Dependencies

Install these from Arduino Library Manager:

- U8g2
- Adafruit SHT31 Library

`Wire` and `Arduino` are part of the core.

## Arduino IDE Quick Start

1. Open Arduino IDE 2.x.
2. Open `spaghettiboy-2000-project.ino`.
3. Select board: `Arduino Uno`.
4. Select the correct serial port.
5. Install required libraries.
6. Click Verify.
7. Click Upload.

## Wiring

From `config.h`:

- `PIN_LDR` -> `A0`
- `PIN_BTN_LEFT` -> `D2` to GND (uses `INPUT_PULLUP`)
- `PIN_BTN_RIGHT` -> `D3` to GND (uses `INPUT_PULLUP`)
- `PIN_STATUS_LED` -> built-in LED

I2C on Uno:

- `SDA` -> `A4`
- `SCL` -> `A5`
- OLED address -> `0x3C` (`OLED_ADDR`)
- SHT31 address -> `0x44` (`SHT31_ADDR`), fallback `0x45`

## Current UI Behavior

States:

- `UI_SPLASH`
- `UI_INFO`
- `UI_GAME`

Navigation:

- From splash: left -> info, right -> game
- From info: left -> splash, right -> game
- From game: right -> splash, left -> info

Info screen content:

- Temperature (`T`)
- Humidity (`H`)
- LDR raw value (`LDR`)

Sensor values are refreshed every `READ_INTERVAL_MS` (default 1000 ms).

## Notes

- OLED rendering uses U8g2 page-buffer flow (`firstPage` / `nextPage`).
- Float values are formatted with `dtostrf`, which avoids AVR `printf` float issues.
- Contrast is adjusted continuously from the LDR reading.

## Customization

- Edit pins, I2C addresses, and timing in `config.h`.
- Edit text/layout in `display.cpp`.
- Change navigation behavior in `spaghettiboy-2000-project.ino`.
