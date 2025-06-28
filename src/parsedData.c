#include "config.h"
#include "sensorData.h"
#include "systemStatus.h"

extern SystemCollectionStruct systemCollection;

double voltageToTemperature(double voltage);
double round(double value);
static void updateSingleSystemState(SystemStruct *system, const HysteresisConfig *hyst);
void updateSystemStatesWithHysteresis(SystemCollectionStruct *systems);
static const char* stateToString(SystemState state);
void printSystemStates(const SystemCollectionStruct *systems);


void processSensorData(SensorDataCollectionStruct data) {

  double voltage1 = redondear(data.sensor1.voltage);
  double voltage2 = redondear(data.sensor2.voltage);
  double voltage3 = redondear(data.sensor3.voltage);
  double voltage4 = redondear(data.sensor4.voltage);

  double temperatura1 = redondear(voltageToTemperature(voltage1));
  double temperatura2 = redondear(voltageToTemperature(voltage2));
  double temperatura3 = redondear(voltageToTemperature(voltage3));
  double temperatura4 = redondear(voltageToTemperature(voltage4));

  systemCollection.system1.voltage = voltage1;
  systemCollection.system1.temperature = temperatura1;

  systemCollection.system2.voltage = voltage2;
  systemCollection.system2.temperature = temperatura2;

  systemCollection.system3.voltage = voltage3;
  systemCollection.system3.temperature = temperatura3;

  systemCollection.system4.voltage = voltage4;
  systemCollection.system4.temperature = temperatura4;

  updateSystemStatesWithHysteresis(&systemCollection);

}

double voltageToTemperature(double voltage) {
  double temp_min = 30.0;   
  double temp_max = 140.0;  
  
  if (voltage < 0.0) voltage = 0.0;
  else if (voltage > 3.0) voltage = 3.0;

  return temp_min + (voltage / 3.0) * (temp_max - temp_min);
}

double round(double value) {
  return round(value * 100) / 100;
};

static void updateSingleSystemState(SystemStruct *system, const HysteresisConfig *hyst) {
  system->previous_state = system->current_state;

  switch (system->current_state) {
    case STATE_NORMAL:
      if (system->temperature > hyst->preventive_threshold + hyst->hysteresis_range) {
        system->current_state = STATE_PREVENTIVE;
      }
      break;

    case STATE_PREVENTIVE:
      if (system->temperature > hyst->emergency_threshold + hyst->hysteresis_range) {
        system->current_state = STATE_EMERGENCY;
      } else if (system->temperature < hyst->preventive_threshold - hyst->hysteresis_range) {
        system->current_state = STATE_NORMAL;
      }
      break;

    case STATE_EMERGENCY:
      if (system->temperature < hyst->emergency_threshold - hyst->hysteresis_range) {
        system->current_state = STATE_PREVENTIVE;
      }
      break;
  }
}

// Actualiza todos los sistemas en la colección
void updateSystemStatesWithHysteresis(SystemCollectionStruct *systems) {
  updateSingleSystemState(&systems->system1, &systems->hysteresis);
  updateSingleSystemState(&systems->system2, &systems->hysteresis);
  updateSingleSystemState(&systems->system3, &systems->hysteresis);
  updateSingleSystemState(&systems->system4, &systems->hysteresis);
}

// Convierte el estado a texto (para logging)
static const char* stateToString(SystemState state) {
  switch (state) {
    case STATE_NORMAL:    return "NORMAL";
    case STATE_PREVENTIVE:return "PREVENTIVE";
    case STATE_EMERGENCY: return "EMERGENCY";
    default:              return "UNKNOWN";
  }
}; 

void printSystemStates(const SystemCollectionStruct *systems) {
  printf("=== Estados de los Sistemas (Histeresis: ±%.1f°C) ===\n", systems->hysteresis.hysteresis_range);
  
  const SystemStruct *sysArray[] = {&systems->system1, &systems->system2, &systems->system3, &systems->system4};
  
  for (int i = 0; i < 4; i++) {
    printf("[%s]\n", sysArray[i]->name);
    printf("  Temp: %.1f°C | Voltaje: %.1fV\n", sysArray[i]->temperature, sysArray[i]->voltage);
    printf("  Estado: %s -> %s\n", 
      stateToString(sysArray[i]->previous_state), 
      stateToString(sysArray[i]->current_state));
    printf("----------------------------------\n");
  }
}
