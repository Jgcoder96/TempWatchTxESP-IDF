#include "config.h"
#include "adc.h"
#include "systemStatus.h"
#include "temperatureSensor.h"
#include "sensorData.h"
#include "httpClientPost.h"

extern SystemCollectionStruct systemCollection;

extern QueueHandle_t sensor_queue;

extern volatile bool wifiConnected;

void senseTemperature(void *pvParam) {
  
  SensorDataCollectionStruct sensorData;
  
  while (1) {
    adcReadingStruct adc_results[NUM_ADC_CHANNELS];
    
    readAllAdcChannels(adc_results);

    sensorData.sensor1 = (SensorDataStruct){"f5deb1d4-3eb3-41b7-a4bb-cba02954ae8d", adc_results[0].voltage};
    sensorData.sensor2 = (SensorDataStruct){"593740d7-5284-4226-abe9-b19c42dce662", adc_results[1].voltage};
    sensorData.sensor3 = (SensorDataStruct){"fdc21238-2c88-4df6-9759-c89da74aef3e", adc_results[2].voltage};
    sensorData.sensor4 = (SensorDataStruct){"e5ec1e01-8fb4-47f3-9510-af2b2be20c86", adc_results[3].voltage};

    xQueueSend(sensor_queue, &sensorData, pdMS_TO_TICKS(100));
    
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void temperatureControl(void *pvParam) {

  SensorDataCollectionStruct receivedData;
  
    while (1) {
      if (xQueueReceive(sensor_queue, &receivedData, pdMS_TO_TICKS(portMAX_DELAY)) == pdTRUE) {
        processSensorData(receivedData);
        /* if (wifiConnected && json_changes != NULL) {
            esp_err_t err = sendJsonPost("http://192.168.18.221:3000/api/data", json_changes);
            if (err != ESP_OK) {
              ESP_LOGW("HTTP", "No se pudo enviar datos al servidor");
            }
          
        } */
      }  
    }
}

