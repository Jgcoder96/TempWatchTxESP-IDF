#include "config.h"
#include "sensorData.h"
#include "systemStatus.h"

SystemCollectionStruct systemCollection;

double voltageToTemperature(double voltage);
double redondear(double valor);

void processSensorData(SensorDataCollectionStruct data) {

  double voltage1 = redondear(data.sensor1.voltage);
  double voltage2 = redondear(data.sensor2.voltage);
  double voltage3 = redondear(data.sensor3.voltage);
  double voltage4 = redondear(data.sensor4.voltage);

  double temperatura1 = redondear(voltageToTemperature(voltage1));
  double temperatura2 = redondear(voltageToTemperature(voltage2));
  double temperatura3 = redondear(voltageToTemperature(voltage3));
  double temperatura4 = redondear(voltageToTemperature(voltage4));


  systemCollection.system1 = (SystemStruct) 
    {
      "1617eb36-d677-44d9-add1-f9ee84da4d91", 
      voltage1, 
      temperatura1,
    };
  systemCollection.system2 = (SystemStruct)
    {
      "057f4ad8-fd5e-4337-bbaa-649abb7fdf2c", 
      voltage2, 
      temperatura2, 
    };
  systemCollection.system3 = (SystemStruct) 
    {
      "ab9865f0-3e68-4e2c-98e1-fd1936c452eb", 
      voltage3, 
      temperatura3, 
    };

  systemCollection.system4 = (SystemStruct)
    {
      "731e6ebb-4ddd-4fb0-8b01-14c61f7a61f6", 
      voltage4, 
      temperatura4, 
    };
}

double voltageToTemperature(double voltage) {
  double temp_min = 30.0;   
  double temp_max = 140.0;  
  
  if (voltage < 0.0) voltage = 0.0;
  else if (voltage > 3.0) voltage = 3.0;

  return temp_min + (voltage / 3.0) * (temp_max - temp_min);
}

double redondear(double valor) {
  return round(valor * 100) / 100;
};