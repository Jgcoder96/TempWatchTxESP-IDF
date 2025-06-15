#include "WifiManager.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "esp_err.h"
#include "nvs_flash.h"

// Tags para logging
static const char *TAG_WIFI = "WiFi";

// Variables para manejo de conexión WiFi
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
    } 
    else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) eventData;
        ESP_LOGW(TAG_WIFI, "Desconectado del AP (razón: %d)", event->reason);
        
        if (retryNum < MAXIMUM_RETRY) {
            esp_wifi_connect();
            retryNum++;
            ESP_LOGI(TAG_WIFI, "Reintentando conexión... Intento %d/%d", retryNum, MAXIMUM_RETRY);
        } else {
            // Después de MAXIMUM_RETRY, programa un reintento después de un delay
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
            
            // Reset contador para el próximo ciclo
            retryNum = 0;
        }
    } 
    else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) eventData;
        ESP_LOGI(TAG_WIFI, "Conectado con IP: " IPSTR, IP2STR(&event->ip_info.ip));
        retryNum = 0;
        
        // Detener cualquier temporizador de reconexión activo
        if (reconnectTimer != NULL && xTimerIsTimerActive(reconnectTimer)) {
            xTimerStop(reconnectTimer, 0);
        }
        
        xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

// Función para inicializar WiFi en modo station
void wifiInitSta() {
    // 1. Crear grupo de eventos
    wifiEventGroup = xEventGroupCreate();

    // 2. Inicializar stack de red
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *staNetif = esp_netif_create_default_wifi_sta();
    assert(staNetif);

    // 3. Configuración WiFi básica
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 4. Registrar manejadores de eventos
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, NULL, NULL));

    // 5. Configurar credenciales WiFi
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

    // 6. Iniciar WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG_WIFI, "Iniciando conexión WiFi a SSID: %s", WIFI_SSID);

    // 7. Esperar conexión o fallo
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    // 8. Manejar resultado
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_WIFI, "Conectado exitosamente a AP SSID: %s", WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG_WIFI, "Falló la conexión a AP SSID: %s", WIFI_SSID);
    } else {
        ESP_LOGE(TAG_WIFI, "Evento inesperado");
    }
}

// Función para obtener información WiFi
void getWifiInfo() {
    // Obtener información del AP
    wifi_ap_record_t apInfo;
    if(esp_wifi_sta_get_ap_info(&apInfo) == ESP_OK) {
        ESP_LOGI(TAG_WIFI, "SSID: %s", apInfo.ssid);
        ESP_LOGI(TAG_WIFI, "RSSI: %d", apInfo.rssi);
        ESP_LOGI(TAG_WIFI, "Canal: %d", apInfo.primary);
    }

    // Obtener información IP
    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ipInfo);
    ESP_LOGI(TAG_WIFI, "Dirección IP: " IPSTR, IP2STR(&ipInfo.ip));

    // Obtener MAC address
    uint8_t mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    ESP_LOGI(TAG_WIFI, "MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}