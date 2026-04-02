# Spaghettiboy-2000

A retro-inspired handheld game console built on Arduino Uno. Features a classic Snake game, environmental sensors (temperature/humidity/light), and an adaptive OLED display.

## What It Does

The Spaghettiboy-2000 is a self-contained gaming device with:
- **Snake Game**: A classic grid-based snake game with increasing difficulty
- **Environmental Monitoring**: Real-time temperature, humidity, and light level sensing
- **Adaptive Display**: OLED brightness automatically adjusts based on ambient light
- **Navigation UI**: Three-screen interface (Splash, Info, Game) with button-based navigation

## How It Works: Architecture Overview

The code is organized around a **state machine** that manages four UI states:

```
[SPLASH] <-> [INFO] <-> [GAME]
   ^          ^          ^
   |          |          |
   +----------+----------+
            |
         [IDLE]
    (Power-save mode)
```

The system constantly reads sensors and updates the display regardless of which screen is active. You can enter **IDLE mode** at any time by activating the kill switch.

### Project Structure

- **`spaghettiboy-2000-project.ino`** — Main application loop
  - State machine management (splash → info → game transitions)
  - Button input debouncing and edge detection
  - Sensor read timing (1 second intervals)
  - Display refresh and brightness adjustment

- **`display.h` / `display.cpp`** — OLED display module
  - Initializes U8g2 library for 128×64 OLED
  - `renderSplash()`: Title screen with navigation hints
  - `renderInfo()`: Shows real-time sensor readings (temperature, humidity, light level)
  - `updateDisplayBrightness()`: Automatically adjusts contrast based on LDR reading
  - Shared `u8g2` display object used by all modules

- **`sensors.h` / `sensors.cpp`** — Environmental sensor module
  - `initSensors()`: Initializes SHT31 temperature/humidity sensor via I2C
  - `readSensors()`: Reads temperature, humidity, and light level
  - Exposes globals: `temperatureC`, `humidityPct`, `lightRaw`, `sensorOk`

- **`snake_game.cpp`** — Snake game module
  - Complete game implementation: grid, snake, food, collision detection
  - Two dedicated buttons (UP/DOWN on D4/D5) for snake movement
  - Game states: splash (waiting for start), play, and game over
  - Press UP+DOWN for 800ms to exit game back to UI
  - Dynamic difficulty: game speeds up as you eat food

- **`config.h`** — Configuration constants
  - Pin definitions for all inputs/outputs
  - I2C addresses for display and sensors
  - Display dimensions and timing values

## State Machine Flow

### SPLASH Screen
- Shows the Spaghettiboy-2000 title and retro ASCII art
- Navigation hints: `<LEFT   RIGHT>` to move between screens
- **LEFT button**: Go to INFO  
- **RIGHT button**: Enter GAME

### INFO Screen
- Displays real-time sensor readings:
  - **Temperature**: From SHT31 sensor (°C)
  - **Humidity**: From SHT31 sensor (%)
  - **Light Level**: Raw ADC value from LDR (0–1023)
- Navigation hints: `<LEFT   RIGHT>` to move between screens
- **LEFT button**: Go to SPLASH  
- **RIGHT button**: Enter GAME

### GAME Screen
- Snake game arena: 16×8 grid (8×8 pixels per cell)
- **UP button** (D4): Move snake up
- **DOWN button** (D5): Move snake down
- **LEFT button** (D2) or **RIGHT button** (D3): Move snake left or right
- **Exit to SPLASH**: Hold UP + DOWN for 800ms
- Game speed increases as your score goes up (shorter step interval)

### IDLE State
- **Activation**: Pull the kill switch (D6) to GND
- **Effect**: Display and sensors power down (low-power mode)
- **Display**: Screen goes blank/black
- **Reactivation**: Release the kill switch to return to the previous active state (SPLASH, INFO, or GAME)
- **Use Case**: Battery-saving mode; press the switch to sleep, press again to wake
- **Note**: The system remembers which screen you were on before entering IDLE

## Snake Game Details

**Grid Layout**:
- 16 columns × 8 rows of 8-pixel cells
- Top row reserved for HUD (score/status)
- Playfield: rows 1–7

**Gameplay**:
- Snake starts in the center facing right with length 3
- Eating food extends the snake and slightly increases speed
- Colliding with walls or yourself ends the game
- Food spawns randomly in empty grid cells

**Controls**:
- Tap UP/DOWN/LEFT/RIGHT to queue direction
- Game does NOT allow 180° turns (e.g., LEFT → RIGHT blocked if moving right)
- Game ticks occur every ~180ms, speeding up as you eat food

## Sensor & Display Behavior

**Temperature/Humidity Sensor (SHT31)**:
- Reads via I2C every 1 second
- Displays in INFO screen
- Shows "N/A" if sensor not found or reading fails
- Both I2C address variants supported (0x44 and 0x45)

**Light Sensor (LDR, Analog)**:
- Reads analog value from A0 every 1 second
- Range: 0 (dark) to 1023 (bright)
- Automatically adjusts display contrast in real-time
- Brighter environments → higher contrast for readability

**Display Rendering**:
- U8g2 library with double-buffering (flicker-free)
- 128×64 pixel monochrome OLED
- Contrast range: 5 (minimum) to 255 (maximum)
- Fonts: 6×10 bitmap font for all text

## Dependencies & Libraries

Install from **Arduino IDE Library Manager**:

1. **U8g2** (by Oliver) — OLED display driver
   - Handles graphics, text rendering, and power management
   - Supports both hardware and software I2C

2. **Adafruit SHT31 Library** — Temperature/humidity sensor
   - Communicates with SHT31 via I2C
   - Provides `readTemperature()` and `readHumidity()`

**Built-in/Core Libraries** (no install needed):
- `Arduino.h` — Core Arduino functions
- `Wire.h` — I2C communication

## Getting Started

### 1. Install Libraries

In **Arduino IDE 2.x**:
- **Sketch** → **Include Library** → **Manage Libraries**
- Search for "U8g2" and install by Oliver
- Search for "Adafruit SHT31" and install by Adafruit

### 2. Wire the Hardware

Follow the [Hardware & Wiring](#hardware--wiring) section above. Double-check:
- I2C pull-up resistors (if needed)
- Button connections to GND (not 5V)
- LDR voltage divider for optimal ADC range

### 3. Upload to Arduino

1. Launch **Arduino IDE 2.x**
2. Open **`spaghettiboy-2000-project.ino`**
3. Select **Board**: Arduino Uno
4. Select **Port**: Your COM port where Uno is connected
5. Click **Verify** (compile without uploading)
6. Click **Upload** (compile and write to device)

### 4. Test the Device

- **Splash Screen**: Should appear on OLED after upload
- **Navigate**: Press LEFT/RIGHT buttons to move between screens
- **INFO Screen**: Temperature, humidity, and light level should update every second
- **GAME Screen**: Press RIGHT from splash, then use UP/DOWN to move the snake
- **Exit Game**: Hold UP + DOWN for ~800ms to return to splash

## Troubleshooting

| Issue | Possible Cause | Solution |
|-------|----------------|----------|
| OLED blank on startup | I2C not initialized | Check I2C wiring (SDA/SCL); verify device address (0x3C) in config |
| "INFO" shows T/H as "N/A" | SHT31 not found | Check I2C connections; try both addresses (0x44 and 0x45) |
| Buttons don't respond | Wiring reversed or missing pull-ups | Verify buttons go to GND, not 5V; check `INPUT_PULLUP` pins |
| Display contrast doesn't change | LDR not reading | Check analog voltage divider; verify A0 wiring |
| Game is too fast/slow | Gameplay tuning | Adjust `stepIntervalMs` initial value in `snake_game.cpp` |
| Kill switch doesn't trigger IDLE | Wired incorrectly | Verify switch connects to D6 and GND; check polarity |
| Device won't wake from IDLE | Switch stuck | Release the kill switch completely (should be open); check for mechanical jamming |
| I2C conflicts | Multiple I2C devices on same bus | Check device addresses don't overlap; add proper pull-up resistors |

## Code Flow: Main Loop

```
setup()
├─ Initialize pins (buttons, LED)
├─ Initialize OLED via I2C
├─ Initialize SHT31 sensor
└─ Enter UI_SPLASH state

loop()
├─ Read all buttons (with debouncing)
├─ Handle navigation state transitions
├─ Every 1 second: Read sensors + update display brightness
├─ If in UI_GAME: Tick snake game logic
└─ Render current state to OLED
```

## Development Notes

- **Modular Design**: Each system (display, sensors, game) is self-contained for easy debugging
- **Memory-Efficient**: Raw C++ without unnecessary dynamic allocations
- **No Blocking**: Sensor reads and rendering use non-blocking timing checks
- **Debouncing**: Hardware debouncing or 8ms software delays prevent false button presses
- **Snake Game**: Uses a fixed-size circular buffer for the snake body (max 128 segments)

## Hardware & Wiring

### Required Components

- **Microcontroller**: Arduino Uno (ATmega328P)
- **Display**: SSD1306 OLED 128×64 (I2C, address 0x3C)
- **Sensors**:
  - SHT31 Temperature/Humidity (I2C, address 0x44 or 0x45)
  - LDR (Light-dependent resistor) with analog input
- **Input**:
  - 4 Pushbuttons (normally-open to GND)
  - 1 Kill switch (SPST to GND)

### Pin Configuration

From `config.h`:

| Function | Pin | Notes |
|----------|-----|-------|
| **Navigation Buttons** |
| Left (UI/Game) | D2 | `INPUT_PULLUP`, normally high |
| Right (UI/Game) | D3 | `INPUT_PULLUP`, normally high |
| **Game-Only Buttons** |
| Up | D4 | `INPUT_PULLUP`, snake control |
| Down | D5 | `INPUT_PULLUP`, snake control |
| **Power & Control** |
| Kill Switch (IDLE) | D6 | `INPUT_PULLUP`, activates power-save mode when pulled low |
| Status LED | D13 | Built-in LED (optional) |
| **Sensors** |
| Light Sensor | A0 | Analog input 0–1023 |

### I2C Wiring

**Standard Arduino Uno I2C Pins**:
- **SDA** (Data): A4
- **SCL** (Clock): A5

Both OLED and SHT31 share the same I2C bus (they have different addresses).

**Typical I2C Connections**:
```
OLED SDA  → Uno A4
OLED SCL  → Uno A5
SHT31 SDA → Uno A4
SHT31 SCL → Uno A5
```

Include pull-up resistors (typically 4.7kΩ) if devices don't have them built-in.

### Button Wiring

All buttons use `INPUT_PULLUP` mode, so they connect to the pin and to GND:

```
[Pushbutton]
    Pin ----+
            |
           GND
```

When pressed, the pin reads LOW; when released, it reads HIGH (due to internal pull-up).

### Kill Switch Wiring (IDLE Mode)

The kill switch also uses `INPUT_PULLUP` mode:

```
[Kill Switch / SPST]
    D6 ----+
           |
          GND
```

- **Switch OPEN** (released): Pin reads HIGH → Device active (displays, sensors running)
- **Switch CLOSED** (pressed): Pin reads LOW → Device enters IDLE mode (display off, sensors paused)

This allows you to toggle power-save mode on and off without restarting. Perfect for extending battery life.

### Light Sensor (LDR) Wiring

Use a voltage divider with a fixed resistor (typically 10kΩ):

```
+5V
 |
[10kΩ resistor]
 |---- A0 (to Uno analog input)
[LDR]
 |
GND
```

As light increases, the voltage at A0 increases (0–1023 range in ADC).

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
