#include <Arduino.h>
#include <Wire.h>
#include <U8x8lib.h>

#include "config.h"

/*
  Snake game module for ATmega328P + 128x64 OLED.
  This module keeps all game logic in one translation unit and is driven from UI_GAME.
*/

namespace {

// ---------------- Display ----------------
U8X8_SSD1306_128X64_NONAME_HW_I2C display(U8X8_PIN_NONE);

// ---------------- Controls ----------------
// Keep existing project navigation pins for splash/info, add two dedicated game buttons.
static const uint8_t PIN_UP = PIN_BTN_LEFT;
static const uint8_t PIN_DOWN = PIN_BTN_RIGHT;
static const uint8_t PIN_LEFT = PIN_BTN_GAME_LEFT;
static const uint8_t PIN_RIGHT = PIN_BTN_GAME_RIGHT;

// ---------------- Game grid ----------------
static const uint8_t GRID_W = 16;                                 // 128 / 8
static const uint8_t GRID_H = 8;                                  // 64 / 8
static const uint16_t MAX_LEN = (uint16_t)GRID_W * (uint16_t)GRID_H;
static const uint8_t PLAYFIELD_MIN_Y = 1;                         // row 0 reserved for HUD
static const uint8_t PLAYFIELD_MAX_Y = GRID_H - 1;

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
static uint16_t stepIntervalMs = 180;
static bool needsRender = true;

// Exit UI_GAME by holding UP + DOWN.
static bool exitRequested = false;
static uint32_t dualHoldStartMs = 0;
static const uint16_t EXIT_HOLD_MS = 800;

struct Button {
  uint8_t pin;
  bool stable;
  bool lastStable;
  uint32_t lastChangeMs;
};

static Button btnUp    { PIN_UP,    false, false, 0 };
static Button btnDown  { PIN_DOWN,  false, false, 0 };
static Button btnLeft  { PIN_LEFT,  false, false, 0 };
static Button btnRight { PIN_RIGHT, false, false, 0 };

static const uint16_t DEBOUNCE_MS = 25;

static bool readPressedRaw(uint8_t pin) {
  return digitalRead(pin) == LOW;
}

static void updateButton(Button &b, uint32_t nowMs) {
  bool raw = readPressedRaw(b.pin);
  if (raw != b.stable && (uint32_t)(nowMs - b.lastChangeMs) >= DEBOUNCE_MS) {
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

static bool snakeOccupies(uint8_t x, uint8_t y) {
  for (uint16_t i = 0; i < snakeLen; i++) {
    if (snakeX[i] == x && snakeY[i] == y) return true;
  }
  return false;
}

static void spawnFood() {
  for (uint16_t tries = 0; tries < 2000; tries++) {
    uint8_t x = (uint8_t)random(GRID_W);
    uint8_t y = (uint8_t)random(PLAYFIELD_MIN_Y, GRID_H);
    if (!snakeOccupies(x, y)) {
      foodX = x;
      foodY = y;
      return;
    }
  }
  foodX = 0;
  foodY = 0;
}

static void resetGame() {
  snakeLen = 3;
  uint8_t startX = GRID_W / 2;
  uint8_t startY = (PLAYFIELD_MIN_Y + PLAYFIELD_MAX_Y) / 2;
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

static bool anyButtonPressedRaw() {
  return readPressedRaw(PIN_UP) || readPressedRaw(PIN_DOWN) ||
         readPressedRaw(PIN_LEFT) || readPressedRaw(PIN_RIGHT);
}

static bool startButtonPressedRaw() {
  // Game starts from the two middle buttons (D4/D5 in this project).
  return readPressedRaw(PIN_LEFT) || readPressedRaw(PIN_RIGHT);
}

static void updateExitRequest(uint32_t nowMs) {
  const bool dualPressed = readPressedRaw(PIN_UP) && readPressedRaw(PIN_DOWN);
  if (dualPressed) {
    if (dualHoldStartMs == 0) {
      dualHoldStartMs = nowMs;
    } else if ((uint32_t)(nowMs - dualHoldStartMs) >= EXIT_HOLD_MS) {
      exitRequested = true;
    }
  } else {
    dualHoldStartMs = 0;
  }
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

  if (!isOpposite(dir, queuedDir)) {
    dir = queuedDir;
  }

  clearButtonEdges();
}

static void stepGame() {
  int16_t nx = snakeX[0];
  int16_t ny = snakeY[0];
  switch (dir) {
    case DIR_UP:    ny--; break;
    case DIR_DOWN:  ny++; break;
    case DIR_LEFT:  nx--; break;
    case DIR_RIGHT: nx++; break;
  }

  if (nx < 0) nx = GRID_W - 1;
  if (nx >= GRID_W) nx = 0;
  if (ny < PLAYFIELD_MIN_Y) ny = PLAYFIELD_MAX_Y;
  if (ny > PLAYFIELD_MAX_Y) ny = PLAYFIELD_MIN_Y;

  uint8_t newX = (uint8_t)nx;
  uint8_t newY = (uint8_t)ny;

  bool willEat = (newX == foodX && newY == foodY);
  uint16_t tailIndex = snakeLen - 1;
  bool hitsBody = false;
  for (uint16_t i = 0; i < snakeLen; i++) {
    if (snakeX[i] == newX && snakeY[i] == newY) {
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
      snakeLen++;
    }
    spawnFood();
    if (stepIntervalMs > 70) stepIntervalMs -= 3;
  }
}

static void renderPlay() {
  char score[8];
  const uint16_t points = (snakeLen >= 3) ? (snakeLen - 3) : 0;

  display.clear();
  snprintf(score, sizeof(score), "S:%u", points);
  display.drawString(0, 0, score);
  display.drawString(foodX, foodY, "@");
  for (uint16_t i = 0; i < snakeLen; i++) {
    display.drawString(snakeX[i], snakeY[i], "#");
  }
}

static void renderSplash() {
  display.clear();
  display.setCursor(5, 1);
  display.print(F("SNAKE"));
  display.setCursor(0, 3);
  display.print(F("Press any middle"));
  display.setCursor(2, 5);
  display.print(F("btn to start"));
  display.setCursor(1, 7);
  display.print(F("<LEFT><RIGHT>"));
}

static void renderGameOver() {
  char scoreLine[18];
  snprintf(scoreLine, sizeof(scoreLine), "Length:%u", snakeLen);

  display.clear();
  display.setCursor(4, 1);
  display.print(F("GAME OVER"));
  display.setCursor(4, 3);
  display.print(scoreLine);
  display.setCursor(0, 5);
  display.print(F("Press middle btn"));
  display.setCursor(3, 7);
  display.print(F("to restart"));
}

} // namespace

void initSnakeGame() {
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);

  display.setI2CAddress((uint8_t)(OLED_ADDR << 1));
  display.begin();
  display.setPowerSave(0);
  display.setFont(u8x8_font_chroma48medium8_r);
  display.clear();

  randomSeed((uint32_t)analogRead(PIN_LDR) ^ ((uint32_t)micros() << 16));

  resetGame();
  state = STATE_SPLASH;
  needsRender = true;
  exitRequested = false;
  dualHoldStartMs = 0;
}

void snakeGameOnEnter() {
  state = STATE_SPLASH;
  needsRender = true;
  exitRequested = false;
  dualHoldStartMs = 0;
  clearButtonEdges();
}

void snakeGameTick(uint32_t nowMs) {
  const GameState beforeState = state;
  updateExitRequest(nowMs);

  switch (state) {
    case STATE_SPLASH:
      if (needsRender) {
        renderSplash();
        needsRender = false;
      }
      if (startButtonPressedRaw()) {
        while (startButtonPressedRaw()) { delay(10); }
        resetGame();
        state = STATE_PLAY;
        needsRender = true;
      }
      delay(20);
      break;

    case STATE_PLAY:
      {
      bool stepped = false;
      handleInput(nowMs);
      if ((uint32_t)(nowMs - lastStepMs) >= stepIntervalMs) {
        lastStepMs = nowMs;
        stepGame();
        stepped = true;
      }
      if (state == STATE_PLAY && (needsRender || stepped)) {
        renderPlay();
        needsRender = false;
      }
      }
      break;

    case STATE_GAMEOVER:
      if (needsRender) {
        renderGameOver();
        needsRender = false;
      }
      if (startButtonPressedRaw()) {
        while (startButtonPressedRaw()) { delay(10); }
        resetGame();
        state = STATE_PLAY;
        needsRender = true;
      }
      delay(20);
      break;
  }

  if (state != beforeState) {
    needsRender = true;
  }
}

bool snakeGameExitRequested() {
  return exitRequested;
}

void clearSnakeGameExitRequest() {
  exitRequested = false;
}

bool snakeGameAllowUiNavigation() {
  return state == STATE_SPLASH;
}

void snakeGameSetPowerSave(bool enabled) {
  display.setPowerSave(enabled ? 1 : 0);
  if (!enabled) {
    needsRender = true;
  }
}
