#include "config.h"
#include "httpClientPost.h"

extern volatile bool wifiConnected;
extern const char *jsonOutput;

void sendDataToServer(void *pvParameters) {
  while (1) {
    if (wifiConnected && jsonOutput != NULL) {
      esp_err_t err = sendJsonPost("http://192.168.18.221:3000/api/data");
      if (err != ESP_OK) {
        ESP_LOGW("HTTP", "No se pudo enviar datos al servidor");
      }
    }
    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
}
