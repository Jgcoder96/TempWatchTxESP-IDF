#include "config.h"
#include "httpClientPost.h"
#include "checkSystemStateChanges.h"

extern volatile bool wifiConnected;
extern SystemCollectionStruct systemCollection;

void verifySystemStateChange(void *pvParameters) {
  while (1) {
    char* json_changes = changedSystemsToJsonString(&systemCollection);
    if (wifiConnected && json_changes != NULL) {
      printf("Enviando datos al servidor: %s\n", json_changes);
      esp_err_t err = sendJsonPost("http://192.168.18.221:3000/api/data", json_changes);
      if (err != ESP_OK) {
        ESP_LOGW("HTTP", "No se pudo enviar datos al servidor");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}