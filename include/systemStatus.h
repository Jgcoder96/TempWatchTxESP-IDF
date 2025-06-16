#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

#include "sensorData.h"  // Incluye SensorDataStruct y SensorDataCollectionStruct

typedef struct {
  char name[40];
  double voltage;
  double temperature;
} SystemStruct;

typedef struct {
  SystemStruct system1;
  SystemStruct system2;
  SystemStruct system3;
  SystemStruct system4;
} SystemCollectionStruct;

void processSensorData(SensorDataCollectionStruct data);

#endif // SYSTEM_STATUS_H