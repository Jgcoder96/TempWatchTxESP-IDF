#include "config.h"
#include "sensorData.h"
#include "systemStatus.h"

SystemCollectionStruct systemCollection;

float voltageToTemperature(float voltage);

void processSensorData(SensorDataCollectionStruct data) {

  float voltage1 = data.sensor1.voltage;
  float voltage2 = data.sensor2.voltage;
  float voltage3 = data.sensor3.voltage;
  float voltage4 = data.sensor4.voltage;

  float temperatura1 = voltageToTemperature(voltage1);
  float temperatura2 = voltageToTemperature(voltage2);
  float temperatura3 = voltageToTemperature(voltage3);
  float temperatura4 = voltageToTemperature(voltage4);


  systemCollection.system1 = (SystemStruct) 
    {
      "Sensor 1", 
      voltage1, 
      temperatura1,
    };
  systemCollection.system2 = (SystemStruct)
    {
      "Sensor 2", 
      voltage2, 
      temperatura2, 
    };
  systemCollection.system3 = (SystemStruct) 
    {
      "Sensor 3", 
      voltage3, 
      temperatura3, 
    };

  systemCollection.system4 = (SystemStruct)
    {
      "Sensor 4", 
      voltage3, 
      temperatura4, 
    };
}

float voltageToTemperature(float voltage) {
  float temp_min = 30.0;   
  float temp_max = 140.0;  
  
  if (voltage < 0.0) voltage = 0.0;
  else if (voltage > 3.0) voltage = 3.0;

  return temp_min + (voltage / 3.0) * (temp_max - temp_min);
}