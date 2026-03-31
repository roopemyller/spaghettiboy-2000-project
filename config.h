#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pins
constexpr uint8_t PIN_LDR = A0;
constexpr uint8_t PIN_STATUS_LED = LED_BUILTIN;

constexpr uint8_t PIN_BTN_LEFT  = 2;
constexpr uint8_t PIN_BTN_RIGHT = 3;
constexpr uint8_t PIN_BTN_GAME_UP = 4;
constexpr uint8_t PIN_BTN_GAME_DOWN = 5;
constexpr uint8_t PIN_KILL_SWITCH = 6;


// I2C
constexpr uint8_t OLED_ADDR = 0x3C;
constexpr uint8_t SHT31_ADDR = 0x44;
constexpr uint8_t SHT31_ADDR_BACKUP = 0x45;

// OLED size
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;

// Timing
constexpr unsigned long READ_INTERVAL_MS = 1000;

// Serial
constexpr unsigned long SERIAL_BAUD = 9600;

#endif