#ifndef SENSORS_H
#define SENSORS_H

#include <Adafruit_SHT31.h>
#include "config.h"

extern float temperatureC;
extern float humidityPct;
extern int lightRaw;
extern bool sensorOk;

void initSensors();
void readSensors();

#endif