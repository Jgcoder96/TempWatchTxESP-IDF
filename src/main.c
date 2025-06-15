#include "adc.h"
#include "config.h"
#include "nvsManager.h"
#include "temperatureSensor.h"
#include "wifiManager.h"

QueueHandle_t sensor_queue = NULL;

void app_main() {
  
  adcReaderInit();
  initNvs();
  wifiInitSta();
  getWifiInfo();

  sensor_queue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);

  xTaskCreatePinnedToCore(senseTemperature, "sensorTemp", 4096, NULL, 12, NULL, 1);
  xTaskCreatePinnedToCore(temperatureControl, "sensorTemp", 4096, NULL, 12, NULL, 1);
}

