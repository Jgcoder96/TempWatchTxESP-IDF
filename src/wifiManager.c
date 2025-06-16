#include "WifiManager.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "string.h"

// Tags para logging
static const char *TAG_WIFI = "WiFi";

// Variable global para estado de conexión WiFi
volatile bool wifiConnected = false;  // <<< NUEVO

// Variables internas
static int retryNum = 0;
static EventGroupHandle_t wifiEventGroup;
static TimerHandle_t reconnectTimer = NULL;

// Función para reconexión programada
static void wifiReconnectTimerCallback(TimerHandle_t xTimer) {
  ESP_LOGI(TAG_WIFI, "Intentando reconexión WiFi...");
  esp_wifi_connect();
}

// Manejador de eventos WiFi
void wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData) {
  if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
      wifiConnected = false;  // <<< NUEVO

      wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) eventData;
      ESP_LOGW(TAG_WIFI, "Desconectado del AP (razón: %d)", event->reason);
        
      if (retryNum < MAXIMUM_RETRY) {
        esp_wifi_connect();
        retryNum++;
        ESP_LOGI(TAG_WIFI, "Reintentando conexión... Intento %d/%d", retryNum, MAXIMUM_RETRY);
      } else {
          if (reconnectTimer == NULL) {
            reconnectTimer = xTimerCreate(
                                          "wifi_reconnect_timer",
                                          pdMS_TO_TICKS(WIFI_RECONNECT_DELAY_MS),
                                          pdFALSE,
                                          (void*)0,
                                          wifiReconnectTimerCallback);
          }
            
          if (reconnectTimer != NULL && xTimerStart(reconnectTimer, 0) == pdPASS) {
            ESP_LOGI(TAG_WIFI, "Programando reconexión en %d ms", WIFI_RECONNECT_DELAY_MS);
          } else {
            ESP_LOGE(TAG_WIFI, "Error al programar temporizador de reconexión");
          }
            
            retryNum = 0;
        }
    } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
      ip_event_got_ip_t* event = (ip_event_got_ip_t*) eventData;
      ESP_LOGI(TAG_WIFI, "Conectado con IP: " IPSTR, IP2STR(&event->ip_info.ip));
      retryNum = 0;

      wifiConnected = true;  // <<< NUEVO

      if (reconnectTimer != NULL && xTimerIsTimerActive(reconnectTimer)) {
        xTimerStop(reconnectTimer, 0);
      }

      xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

// Inicializa WiFi en modo STA
void wifiInitSta() {
  wifiEventGroup = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t *staNetif = esp_netif_create_default_wifi_sta();
  assert(staNetif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, NULL, NULL));

  wifi_config_t wifiConfig = {
    .sta = {
      .ssid = WIFI_SSID,
      .password = WIFI_PASS,
      .threshold.authmode = WIFI_AUTH_WPA2_PSK,
      .pmf_cfg = {
        .capable = true,
        .required = false
      },
    },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG_WIFI, "Iniciando conexión WiFi a SSID: %s", WIFI_SSID);

  EventBits_t bits = xEventGroupWaitBits(
                                          wifiEventGroup,
                                          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG_WIFI, "Conectado exitosamente a SSID: %s", WIFI_SSID);
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGE(TAG_WIFI, "Falló la conexión a SSID: %s", WIFI_SSID);
  } else {
    ESP_LOGE(TAG_WIFI, "Evento inesperado");
  }
}

// Función para imprimir información WiFi
void getWifiInfo() {
  wifi_ap_record_t apInfo;
  if(esp_wifi_sta_get_ap_info(&apInfo) == ESP_OK) {
    ESP_LOGI(TAG_WIFI, "SSID: %s", apInfo.ssid);
    ESP_LOGI(TAG_WIFI, "RSSI: %d", apInfo.rssi);
    ESP_LOGI(TAG_WIFI, "Canal: %d", apInfo.primary);
  }

  esp_netif_ip_info_t ipInfo;
  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ipInfo);
  ESP_LOGI(TAG_WIFI, "Dirección IP: " IPSTR, IP2STR(&ipInfo.ip));

  uint8_t mac[6];
  esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
  ESP_LOGI(TAG_WIFI, "MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
