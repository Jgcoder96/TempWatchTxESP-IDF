#include "config.h"
#include "httpClientPost.h"
#include "convertDataToJSON.h"

extern volatile bool wifiConnected;
extern SystemCollectionStruct systemCollection;

void sendDataToServer(void *pvParameters) {
  while (1) {
    char* jsonOutput = systemsToJsonString(&systemCollection);
    if (wifiConnected && jsonOutput != NULL) {
      printf("Enviando datos al servidor: %s\n", jsonOutput);
      esp_err_t err = sendJsonPost("http://192.168.18.221:3000/api/data", jsonOutput);
      if (err != ESP_OK) {
        ESP_LOGW("HTTP", "No se pudo enviar datos al servidor");
      }
    }
    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
}
