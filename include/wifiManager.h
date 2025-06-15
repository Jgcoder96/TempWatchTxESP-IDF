#pragma once

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

// Configuración WiFi
#define WIFI_SSID "Sin Servicio"
#define WIFI_PASS "Gregory.040620"
#define MAXIMUM_RETRY 5
#define WIFI_RECONNECT_DELAY_MS 5000  // 5 segundos entre intentos

// Bits de evento WiFi
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

void wifiInitSta();
void getWifiInfo();