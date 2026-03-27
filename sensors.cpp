#include "sensors.h"

Adafruit_SHT31 sht31 = Adafruit_SHT31();

float temperatureC = NAN;
float humidityPct = NAN;
int lightRaw = 0;
bool sensorOk = false;

void initSensors() {
  sensorOk = sht31.begin(SHT31_ADDR);
  if (!sensorOk) sensorOk = sht31.begin(SHT31_ADDR_BACKUP);
}

void readSensors() {
  lightRaw = analogRead(PIN_LDR);
  temperatureC = sht31.readTemperature();
  humidityPct = sht31.readHumidity();
}