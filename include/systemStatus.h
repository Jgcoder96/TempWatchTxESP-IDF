#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

#include "sensorData.h"  

typedef enum {
  STATE_NORMAL,     
  STATE_PREVENTIVE,  
  STATE_EMERGENCY    
} SystemState;

typedef struct {
  float preventive_threshold;  
  float emergency_threshold;   
  float hysteresis_range;     
} HysteresisConfig;

typedef struct {
  char name[40];
  double voltage;
  double temperature;
  SystemState previous_state; 
  SystemState current_state; 
} SystemStruct;

typedef struct {
  SystemStruct system1;
  SystemStruct system2;
  SystemStruct system3;
  SystemStruct system4;
  HysteresisConfig hysteresis;
} SystemCollectionStruct;

void processSensorData(SensorDataCollectionStruct data);

#endif // SYSTEM_STATUS_H