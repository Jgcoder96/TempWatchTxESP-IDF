#include "nvsManager.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"

#define TAG "NVS_MANAGER"

esp_err_t initNvs(void) {
  esp_err_t ret = nvs_flash_init();
    
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGI(TAG, "NVS corrupta o desactualizada. Borrando...");
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
    
  ESP_ERROR_CHECK(ret);
  ESP_LOGI(TAG, "NVS inicializada correctamente");
  return ret;
}