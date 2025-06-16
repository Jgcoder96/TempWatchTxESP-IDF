#include "config.h"
#include "adc.h"
#include "systemStatus.h"
#include "temperatureSensor.h"
#include "sensorData.h"
#include "convertDataToJSON.h"

extern SystemCollectionStruct systemCollection;
extern QueueHandle_t sensor_queue;

char* jsonOutput;

void senseTemperature(void *pvParam) {
  
  SensorDataCollectionStruct sensorData;
  
  while (1) {
    adcReadingStruct adc_results[NUM_ADC_CHANNELS];
    
    readAllAdcChannels(adc_results);

    sensorData.sensor1 = (SensorDataStruct){"1617eb36-d677-44d9-add1-f9ee84da4d91", adc_results[0].voltage};
    sensorData.sensor2 = (SensorDataStruct){"057f4ad8-fd5e-4337-bbaa-649abb7fdf2c", adc_results[1].voltage};
    sensorData.sensor3 = (SensorDataStruct){"ab9865f0-3e68-4e2c-98e1-fd1936c452eb", adc_results[2].voltage};
    sensorData.sensor4 = (SensorDataStruct){"731e6ebb-4ddd-4fb0-8b01-14c61f7a61f6", adc_results[3].voltage};

    xQueueSend(sensor_queue, &sensorData, pdMS_TO_TICKS(1000));
    
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void temperatureControl(void *pvParam) {

  SensorDataCollectionStruct receivedData;
  
    while (1) {
      if (xQueueReceive(sensor_queue, &receivedData, pdMS_TO_TICKS(portMAX_DELAY)) == pdTRUE) {
        processSensorData(receivedData);
        jsonOutput = systemsToJsonString(&systemCollection);
      }  
    }
}