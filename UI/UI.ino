// SpaghettiBOY-2000 - minimal UI/navigation for ATmega328P + 128x64 OLED
// Uses U8x8 text mode for low RAM usage (no full frame buffer in sketch).
//
// IMPORTANT:
// If your exact OLED controller/wiring differs, adjust the constructor below.

#include <Arduino.h>
#include <U8x8lib.h>

// -----------------------------
// Pin configuration
// -----------------------------
constexpr uint8_t PIN_BTN_LEFT  = 2;
constexpr uint8_t PIN_BTN_RIGHT = 3;

// -----------------------------
// OLED setup (choose constructor matching your module)
// Example below: ST7565 128x64 via software SPI
// -----------------------------
U8X8_ST7565_NHD_C12864_4W_SW_SPI oled(
  /* clock=*/13, /* data=*/11, /* cs=*/10, /* dc=*/9, /* reset=*/8
);

// -----------------------------
// UI state machine
// -----------------------------
enum UiState : uint8_t {
  UI_SPLASH = 0,
  UI_INFO   = 1,
  UI_GAME   = 2
};

UiState currentState = UI_SPLASH;

// Button edge detection (pressed event only)
bool prevLeftPressed  = false;
bool prevRightPressed = false;

// -----------------------------
// Rendering
// -----------------------------
void renderSplash() {
  oled.clearDisplay();
  oled.setFont(u8x8_font_chroma48medium8_r);
  oled.drawString(1, 1, "   (o)   ");
  oled.drawString(1, 2, "SpaghettiBOY");
  oled.drawString(3, 3, " -2000- ");
  oled.drawString(0, 6, "<LEFT  RIGHT>");
}

void renderInfo() {
  oled.clearDisplay();
  oled.setFont(u8x8_font_chroma48medium8_r);
  oled.drawString(0, 1, "INFO SCREEN");
  oled.drawString(0, 3, "Content later");
  oled.drawString(0, 6, "LEFT: Splash");
}

void renderGame() {
  oled.clearDisplay();
  oled.setFont(u8x8_font_chroma48medium8_r);
  oled.drawString(0, 1, "GAME SCREEN");
  oled.drawString(0, 3, "Content later");
  oled.drawString(0, 6, "RIGHT: Splash");
}

void renderCurrentState() {
  switch (currentState) {
    case UI_SPLASH: renderSplash(); break;
    case UI_INFO:   renderInfo();   break;
    case UI_GAME:   renderGame();   break;
  }
}

void setState(UiState next) {
  if (next != currentState) {
    currentState = next;
    renderCurrentState();
  }
}

// -----------------------------
// Input + navigation
// -----------------------------
bool isPressed(uint8_t pin) {
  // Buttons expected with internal pull-up: pressed = LOW.
  return digitalRead(pin) == LOW;
}

void handleNavigation() {
  const bool leftPressed  = isPressed(PIN_BTN_LEFT);
  const bool rightPressed = isPressed(PIN_BTN_RIGHT);

  const bool leftEdge  = leftPressed  && !prevLeftPressed;
  const bool rightEdge = rightPressed && !prevRightPressed;

  prevLeftPressed  = leftPressed;
  prevRightPressed = rightPressed;

  // Minimal debounce cost; keeps code tiny.
  delay(8);

  switch (currentState) {
    case UI_SPLASH:
      if (leftEdge) {
        setState(UI_INFO);
      } else if (rightEdge) {
        setState(UI_GAME);
      }
      break;

    case UI_INFO:
      if (leftEdge) {
        setState(UI_SPLASH);
      }
      break;

    case UI_GAME:
      if (rightEdge) {
        setState(UI_SPLASH);
      }
      break;
  }
}

void setup() {
  pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
  pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);

  oled.begin();
  oled.setPowerSave(0);
  oled.setContrast(180);

  setState(UI_SPLASH);
}

void loop() {
  handleNavigation();
}
