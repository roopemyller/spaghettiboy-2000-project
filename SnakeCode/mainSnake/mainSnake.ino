/*
  Snake for ATmega328P (Arduino Uno/Nano) + 128x64 I2C OLED.

  Libraries (Arduino Library Manager):
  - U8g2 (uses U8x8lib API)

  Wiring (recommended: use internal pullups, buttons to GND):
  - OLED VCC -> 5V (or 3.3V if your module requires), GND -> GND
  - OLED SDA -> A4, SCL -> A5 (Uno/Nano I2C)
  - Buttons:
      UP    -> D2  (to GND when pressed)
      DOWN  -> D3
      LEFT  -> D4
      RIGHT -> D5

  Notes:
  - U8x8 is character-cell based (8x8 cells on 128x64).
  - Game uses a 16x8 text grid (one character per snake segment).
*/

#include <Arduino.h>
#include <Wire.h>
#include <U8x8lib.h>

// ---------------- Display ----------------
static const uint8_t OLED_ADDR = 0x3C; // common; sometimes 0x3D
U8X8_SSD1306_128X64_NONAME_HW_I2C display(U8X8_PIN_NONE);

// ---------------- Controls ----------------
static const uint8_t PIN_UP = 2;
static const uint8_t PIN_DOWN = 3;
static const uint8_t PIN_LEFT = 4;
static const uint8_t PIN_RIGHT = 5;

// ---------------- Game grid ----------------
static const uint8_t GRID_W = 16;              // 128 / 8
static const uint8_t GRID_H = 8;               // 64 / 8
static const uint16_t MAX_LEN = (uint16_t)GRID_W * (uint16_t)GRID_H; // 512

// Snake body positions (0..31, 0..15)
static uint8_t snakeX[MAX_LEN];
static uint8_t snakeY[MAX_LEN];
static uint16_t snakeLen = 0;

static uint8_t foodX = 0;
static uint8_t foodY = 0;

enum Dir : uint8_t { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
static Dir dir = DIR_RIGHT;
static Dir queuedDir = DIR_RIGHT;

enum GameState : uint8_t { STATE_SPLASH, STATE_PLAY, STATE_GAMEOVER };
static GameState state = STATE_SPLASH;

// Timing
static uint32_t lastStepMs = 0;
static uint16_t stepIntervalMs = 180; // base speed

// Simple button debounce + edge detect
struct Button {
  uint8_t pin;
  bool stable;      // stable level (true = pressed)
  bool lastStable;
  uint32_t lastChangeMs;
};

static Button btnUp    { PIN_UP,    false, false, 0 };
static Button btnDown  { PIN_DOWN,  false, false, 0 };
static Button btnLeft  { PIN_LEFT,  false, false, 0 };
static Button btnRight { PIN_RIGHT, false, false, 0 };

static const uint16_t DEBOUNCE_MS = 25;

static bool readPressedRaw(uint8_t pin) {
  // INPUT_PULLUP: pressed -> LOW
  return digitalRead(pin) == LOW;
}

static void updateButton(Button &b, uint32_t nowMs) {
  bool raw = readPressedRaw(b.pin);
  if (raw != b.stable && (nowMs - b.lastChangeMs) >= DEBOUNCE_MS) {
    b.lastStable = b.stable;
    b.stable = raw;
    b.lastChangeMs = nowMs;
  }
}

static bool pressedEdge(const Button &b) {
  return (b.stable == true) && (b.lastStable == false);
}

static void clearButtonEdges() {
  btnUp.lastStable = btnUp.stable;
  btnDown.lastStable = btnDown.stable;
  btnLeft.lastStable = btnLeft.stable;
  btnRight.lastStable = btnRight.stable;
}

// ---------------- Utility ----------------
static bool snakeOccupies(uint8_t x, uint8_t y) {
  for (uint16_t i = 0; i < snakeLen; i++) {
    if (snakeX[i] == x && snakeY[i] == y) return true;
  }
  return false;
}

static void spawnFood() {
  // Find a free cell. Worst case loops, but grid is small.
  for (uint16_t tries = 0; tries < 2000; tries++) {
    uint8_t x = (uint8_t)random(GRID_W);
    uint8_t y = (uint8_t)random(GRID_H);
    if (!snakeOccupies(x, y)) {
      foodX = x;
      foodY = y;
      return;
    }
  }
  // Fallback: if nearly full, just place somewhere (game is basically won)
  foodX = 0;
  foodY = 0;
}

static void resetGame() {
  snakeLen = 3;
  uint8_t startX = GRID_W / 2;
  uint8_t startY = GRID_H / 2;
  snakeX[0] = startX;
  snakeY[0] = startY;
  snakeX[1] = startX - 1;
  snakeY[1] = startY;
  snakeX[2] = startX - 2;
  snakeY[2] = startY;
  dir = DIR_RIGHT;
  queuedDir = DIR_RIGHT;
  stepIntervalMs = 180;
  spawnFood();
  lastStepMs = millis();
}

static bool isOpposite(Dir a, Dir b) {
  return (a == DIR_UP && b == DIR_DOWN) ||
         (a == DIR_DOWN && b == DIR_UP) ||
         (a == DIR_LEFT && b == DIR_RIGHT) ||
         (a == DIR_RIGHT && b == DIR_LEFT);
}

static void handleInput(uint32_t nowMs) {
  updateButton(btnUp, nowMs);
  updateButton(btnDown, nowMs);
  updateButton(btnLeft, nowMs);
  updateButton(btnRight, nowMs);

  if (pressedEdge(btnUp))    queuedDir = DIR_UP;
  if (pressedEdge(btnDown))  queuedDir = DIR_DOWN;
  if (pressedEdge(btnLeft))  queuedDir = DIR_LEFT;
  if (pressedEdge(btnRight)) queuedDir = DIR_RIGHT;

  // Prevent instant reversal
  if (!isOpposite(dir, queuedDir)) {
    dir = queuedDir;
  }

  // Consume edges so a held button doesn't continuously re-trigger
  clearButtonEdges();
}

static void stepGame() {
  // Compute next head
  int16_t nx = snakeX[0];
  int16_t ny = snakeY[0];
  switch (dir) {
    case DIR_UP:    ny--; break;
    case DIR_DOWN:  ny++; break;
    case DIR_LEFT:  nx--; break;
    case DIR_RIGHT: nx++; break;
  }

  // Wrap around edges (classic mode). If you prefer walls, change this.
  if (nx < 0) nx = GRID_W - 1;
  if (nx >= GRID_W) nx = 0;
  if (ny < 0) ny = GRID_H - 1;
  if (ny >= GRID_H) ny = 0;

  uint8_t newX = (uint8_t)nx;
  uint8_t newY = (uint8_t)ny;

  // Collision with self: allow moving into last tail cell only if not growing.
  bool willEat = (newX == foodX && newY == foodY);
  uint16_t tailIndex = snakeLen - 1;
  bool hitsBody = false;
  for (uint16_t i = 0; i < snakeLen; i++) {
    if (snakeX[i] == newX && snakeY[i] == newY) {
      // If not growing, the tail moves away, so head may step into current tail.
      if (!willEat && i == tailIndex) {
        hitsBody = false;
      } else {
        hitsBody = true;
      }
      break;
    }
  }
  if (hitsBody) {
    state = STATE_GAMEOVER;
    return;
  }

  // Move body: shift right
  if (snakeLen < MAX_LEN) {
    for (int16_t i = (int16_t)snakeLen - 1; i >= 1; i--) {
      snakeX[i] = snakeX[i - 1];
      snakeY[i] = snakeY[i - 1];
    }
  }
  snakeX[0] = newX;
  snakeY[0] = newY;

  if (willEat) {
    if (snakeLen < MAX_LEN) {
      // Grow by duplicating last segment (already shifted), so just increase len.
      snakeLen++;
    }
    spawnFood();
    // Speed up a bit as you grow (floor at 70ms)
    if (stepIntervalMs > 70) stepIntervalMs -= 3;
  }
}

static void drawCell(uint8_t gx, uint8_t gy, char ch) {
  display.setCursor(gx, gy);
  display.print(ch);
}

static void renderPlay() {
  display.clear();

  // Food
  drawCell(foodX, foodY, '@');

  // Snake
  for (uint16_t i = 0; i < snakeLen; i++) {
    drawCell(snakeX[i], snakeY[i], '#');
  }
}

static void renderSplash() {
  display.clear();
  display.setCursor(5, 1);
  display.print(F("SNAKE"));
  display.setCursor(0, 4);
  display.print(F("Press any btn"));
  display.setCursor(2, 6);
  display.print(F("to start"));
}

static void renderGameOver() {
  display.clear();
  display.setCursor(3, 1);
  display.print(F("GAME OVER"));
  display.setCursor(3, 4);
  display.print(F("Length:"));
  display.print(snakeLen);
  display.setCursor(0, 6);
  display.print(F("Press restart"));
}

static bool anyButtonPressedRaw() {
  return readPressedRaw(PIN_UP) || readPressedRaw(PIN_DOWN) ||
         readPressedRaw(PIN_LEFT) || readPressedRaw(PIN_RIGHT);
}

void setup() {
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);

  Wire.begin();
  display.setI2CAddress((uint8_t)(OLED_ADDR << 1));
  display.begin();
  display.setPowerSave(0);
  display.setFont(u8x8_font_chroma48medium8_r);
  display.clear();

  randomSeed((uint32_t)analogRead(A0) ^ ((uint32_t)micros() << 16));

  resetGame();
  state = STATE_SPLASH;
}

void loop() {
  uint32_t now = millis();

  switch (state) {
    case STATE_SPLASH:
      renderSplash();
      if (anyButtonPressedRaw()) {
        // simple "press to start" — wait for release to avoid immediate turn spam
        while (anyButtonPressedRaw()) { delay(10); }
        resetGame();
        state = STATE_PLAY;
      }
      delay(20);
      break;

    case STATE_PLAY:
      handleInput(now);
      if ((uint32_t)(now - lastStepMs) >= stepIntervalMs) {
        lastStepMs = now;
        stepGame();
      }
      renderPlay();
      break;

    case STATE_GAMEOVER:
      renderGameOver();
      if (anyButtonPressedRaw()) {
        while (anyButtonPressedRaw()) { delay(10); }
        resetGame();
        state = STATE_PLAY;
      }
      delay(20);
      break;
  }
}
