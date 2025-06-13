#include "config.h"
#include "temperatureSensor.h"
#include "adc.h"

QueueHandle_t sensor_queue = NULL;

void app_main() {
  
  adcReaderInit();

  sensor_queue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);

  xTaskCreatePinnedToCore(senseTemperature, "sensorTemp", 4096, NULL, 12, NULL, 1);
  xTaskCreatePinnedToCore(temperatureControl, "sensorTemp", 4096, NULL, 12, NULL, 1);
}