#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "sensorData.h"  // Usa SensorDataStruct definido en sensorData.h

void senseTemperature(void *pvParam);
void temperatureControl(void *pvParam);

#endif // TEMPERATURE_SENSOR_H