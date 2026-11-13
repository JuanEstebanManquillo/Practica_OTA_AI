/** @file ota_manager.c @brief OTA segura con verificacion TLS y rollback. */
#include "ota_manager.h"
#include "tv_config.h"
#include "wifi_manager.h"
#include <string.h>
#include "esp_app_desc.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "OTA";
// El simbolo depende del path escrito en EMBED_TXTFILES (certs/server_cert.pem).
extern const char server_cert_start[] asm("_binary_certs_server_cert_pem_start");

esp_err_t tv_ota_confirm_after_selftest(bool ok)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (!running) return ESP_FAIL;
    esp_ota_img_states_t state;
    if (esp_ota_get_state_partition(running, &state) != ESP_OK ||
        state != ESP_OTA_IMG_PENDING_VERIFY) {
        ESP_LOGI(TAG, "Arranque normal (sin confirmacion OTA pendiente)");
        return ESP_OK;
    }
    if (!ok) {
        ESP_LOGE(TAG, "Autotest fallido: revirtiendo firmware");
        esp_ota_mark_app_invalid_rollback_and_reboot();
        return ESP_FAIL;
    }
    esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "ACTUALIZACION OTA CONFIRMADA: firmware %s", esp_app_get_description()->version);
    }
    return err;
}

static void ota_worker(void *unused)
{
    (void)unused;
    vTaskDelay(pdMS_TO_TICKS(12000));
    if (strcmp(esp_app_get_description()->version, TV_TARGET_VERSION) == 0) {
        ESP_LOGI(TAG, "Version objetivo %s ya instalada; OTA omitida", TV_TARGET_VERSION);
        vTaskDelete(NULL);
        return;
    }
    for (;;) {
        if (!tv_wifi_connected()) {
            ESP_LOGW(TAG, "Sin Wi-Fi; OTA pospuesta");
        } else {
            ESP_LOGI(TAG, "Iniciando OTA HTTPS desde %s", TV_OTA_URL);
            esp_http_client_config_t http_cfg = {
                .url = TV_OTA_URL,
                .cert_pem = server_cert_start,
                .timeout_ms = 12000,
                .keep_alive_enable = true,
            };
            esp_https_ota_config_t ota_cfg = { .http_config = &http_cfg };
            esp_err_t result = esp_https_ota(&ota_cfg);
            if (result == ESP_OK) {
                ESP_LOGI(TAG, "Firmware grabado; reiniciando para verificar nueva version");
                vTaskDelay(pdMS_TO_TICKS(500));
                esp_restart();
            }
            ESP_LOGE(TAG, "OTA fallo: %s; se conserva version anterior", esp_err_to_name(result));
        }
        vTaskDelay(pdMS_TO_TICKS(TV_OTA_RETRY_MS));
    }
}

void tv_ota_start_task(void)
{
    xTaskCreate(ota_worker, "ota_worker", 8192, NULL, 4, NULL);
}
