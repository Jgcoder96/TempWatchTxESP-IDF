#include "httpClientPost.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include <string.h>
#include <inttypes.h>  // Para PRId64

static const char *TAG = "HTTP_CLIENT";

// Manejador de eventos del cliente HTTP
static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Si no es una respuesta chunked, imprime los datos
                ESP_LOGI(TAG, "%.*s", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        default:
            break;
    }
    return ESP_OK;
}

// Función para enviar JSON por POST a una URL y mostrar la respuesta
esp_err_t sendJsonPost(const char *url, const char *json_data) {
    if (url == NULL || json_data == NULL) {
        ESP_LOGE(TAG, "URL o JSON data es NULL");
        return ESP_ERR_INVALID_ARG;
    }

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
        .buffer_size = 1024,  // Buffer para almacenar la respuesta
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        ESP_LOGE(TAG, "Error al inicializar el cliente HTTP");
        return ESP_FAIL;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_data, strlen(json_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        int content_length = esp_http_client_get_content_length(client);

        ESP_LOGI(TAG, "POST Status = %d, Content Length = %d", status, content_length);

        if (content_length > 0) {
            // Buffer para guardar la respuesta del servidor
            char buffer[1025] = {0};  // +1 para el '\0'

            int read_len = esp_http_client_read_response(client, buffer, sizeof(buffer) - 1);

            if (read_len >= 0) {
                buffer[read_len] = '\0';  // Asegurar terminación de cadena
                ESP_LOGI(TAG, "Respuesta del servidor: %s", buffer);
            } else {
                ESP_LOGW(TAG, "No se pudo leer la respuesta del servidor");
            }
        }
    } else {
        ESP_LOGE(TAG, "POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}