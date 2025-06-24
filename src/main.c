#include "adc.h"
#include "config.h"
#include "nvsManager.h"
#include "temperatureSensor.h"
#include "wifiManager.h"
#include "sendDataToServer.h"
#include "systemStatus.h"
#include "verifySystemStateChange.h"

QueueHandle_t sensor_queue = NULL;

SystemCollectionStruct systemCollection;

void app_main() {

  systemCollection.system1 = (SystemStruct) 
    {
      "f5deb1d4-3eb3-41b7-a4bb-cba02954ae8d", 
      0, 
      0,
      STATE_NORMAL,
      STATE_NORMAL
    };

  systemCollection.system2 = (SystemStruct) 
    {
      "593740d7-5284-4226-abe9-b19c42dce662", 
      0, 
      0,
      STATE_NORMAL,
      STATE_NORMAL
    };

  systemCollection.system3 = (SystemStruct) 
    {
      "fdc21238-2c88-4df6-9759-c89da74aef3e", 
      0, 
      0,
      STATE_NORMAL,
      STATE_NORMAL
    };

  systemCollection.system4 = (SystemStruct) 
    {
      "e5ec1e01-8fb4-47f3-9510-af2b2be20c86", 
      0, 
      0,
      STATE_NORMAL,
      STATE_NORMAL
    };

  systemCollection.hysteresis.preventive_threshold = 85.0;
  systemCollection.hysteresis.emergency_threshold = 110.0;
  systemCollection.hysteresis.hysteresis_range = 10.0;

  adcReaderInit();
  initNvs();
  wifiInitSta();
  getWifiInfo();

  sensor_queue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);

  xTaskCreatePinnedToCore(senseTemperature, "sensorTemp", 8192, NULL, 12, NULL, 1);
  xTaskCreatePinnedToCore(temperatureControl, "sensorTemp", 8192, NULL, 12, NULL, 1);
  xTaskCreatePinnedToCore(sendDataToServer, "task30s", 8192, NULL, 12, NULL, 1);
  xTaskCreatePinnedToCore(verifySystemStateChange, "verifySystemStateChanges", 8192, NULL, 12, NULL, 1);
}

