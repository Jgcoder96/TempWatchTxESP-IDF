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

    sensorData.sensor1 = (SensorDataStruct){"Sensor 1", adc_results[0].voltage};
    sensorData.sensor2 = (SensorDataStruct){"Sensor 2", adc_results[1].voltage};
    sensorData.sensor3 = (SensorDataStruct){"Sensor 3", adc_results[2].voltage};
    sensorData.sensor4 = (SensorDataStruct){"Sensor 4", adc_results[3].voltage};

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
        printf("%s\n", jsonOutput);
      }  
    }
}