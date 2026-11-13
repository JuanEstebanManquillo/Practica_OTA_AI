/** @file main.c
 *  @brief TerraVision IA: entregables SoC OTA HTTPS + IA local con SHT31.
 *  @details Modo real por defecto. El modo demostración debe activarse
 *  explícitamente; sus datos NO representan mediciones del SHT31 físico.
 */
#include "tv_config.h"
#include "sht31_sensor.h"
#include "ai_engine.h"
#include "wifi_manager.h"
#include "ota_manager.h"
#include "status_led.h"
#include <stdbool.h>
#include <stdint.h>
#include "esp_app_desc.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "TERRAVISION";
static tv_ai_t detector;

/** @brief Genera ejemplo sintético de temperatura y humedad, si está habilitado DEMO. */
#if TV_DEMO_MODE
static void demonstration_values(unsigned index, float *temp_c, float *humidity_rh)
{
    *temp_c = (index % 24 == 20) ? 37.0f : (24.0f + 0.12f * (float)((int)(index % 5) - 2));
    *humidity_rh = (index % 28 == 23) ? 85.0f : (55.0f + (float)((int)(index % 3) - 1));
}
#endif

/** @brief Obtiene lecturas SHT31 y ejecuta inferencia local. */
static void sample_loop(void *unused)
{
    (void)unused;
    unsigned index = 0;
    while (true) {
        float temp_c = 0.0f;
        float humidity_rh = 0.0f;
        float latency_ms = 0.0f;
        bool ok = false;
#if TV_DEMO_MODE
        demonstration_values(index, &temp_c, &humidity_rh);
        latency_ms = 18.0f + (float)(index % 3);
        ok = true;
        ESP_LOGW(TAG, "DEMO: temperatura/humedad SINTETICAS, no medicion fisica");
#else
        esp_err_t err = tv_sht31_read(&temp_c, &humidity_rh, &latency_ms);
        ok = (err == ESP_OK);
        if (!ok) {
            ESP_LOGW(TAG, "Lectura SHT31 fallida (%s)", esp_err_to_name(err));
        }
#endif
        tv_ai_result_t result = tv_ai_update(&detector, ok, temp_c, humidity_rh, latency_ms);
        if (ok) {
            ESP_LOGI(TAG, "Medicion %u: T=%.2f C HR=%.2f %% I2C=%.1f ms salud=%s",
                index, (double)temp_c, (double)humidity_rh,
                (double)latency_ms, tv_ai_health_name(result.health));
        } else {
            ESP_LOGW(TAG, "SHT31 sin respuesta valida; fallos=%u salud=%s",
                result.consecutive_fail, tv_ai_health_name(result.health));
        }
        if (result.alert_temperature || result.alert_humidity || result.alert_latency) {
            ESP_LOGW(TAG, "IA: ANOMALIA T=%d (z=%.2f) HR=%d (z=%.2f) I2C=%d (z=%.2f)",
                result.alert_temperature, (double)result.temperature_z,
                result.alert_humidity, (double)result.humidity_z,
                result.alert_latency, (double)result.latency_z);
        }
        if (result.health == TV_HEALTH_CRITICAL) {
            ESP_LOGE(TAG, "IA/GESTION: fallo persistente del sensor/conexion I2C");
        }
        ++index;
        vTaskDelay(pdMS_TO_TICKS(TV_SAMPLE_PERIOD_MS));
    }
}

/** @brief Arranca el nodo, verifica autopruebas y confirma OTA cuando corresponde. */
void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_LOGI(TAG, "================================================");
    ESP_LOGI(TAG, "TERRAVISION - SHT31 - FIRMWARE VERSION %s", esp_app_get_description()->version);
    ESP_LOGI(TAG, "ENTREGABLE 1: OTA HTTPS CON ROLLBACK");
    ESP_LOGI(TAG, "ENTREGABLE 2: IA LOCAL T / HR Y SALUD I2C");
    ESP_LOGI(TAG, "================================================");

    tv_ai_init(&detector);
    bool selftest_ok = tv_ai_selftest();
    err = tv_sht31_init();
    selftest_ok = selftest_ok && (err == ESP_OK);
    // La ausencia del módulo físico SHT31 no obliga a revertir una OTA
    // si el arranque de la aplicación y el controlador I2C funcionan.
    ESP_ERROR_CHECK(tv_ota_confirm_after_selftest(selftest_ok));
    if (!selftest_ok) {
        ESP_LOGE(TAG, "Autotest fallido: revisar motor IA y configuración I2C");
        return;
    }
    tv_led_start();
#if TV_DEMO_MODE
    ESP_LOGW(TAG, "DEMO ACTIVADA: lecturas fabricadas para probar la IA");
#else
    ESP_LOGI(TAG, "MODO REAL: SHT31 I2C SDA=%d SCL=%d dirección=0x%02X",
        TV_SHT31_SDA_GPIO, TV_SHT31_SCL_GPIO, TV_SHT31_ADDRESS);
#endif
    xTaskCreate(sample_loop, "sht31_ai", 4096, NULL, 3, NULL);
    if (tv_wifi_start() == ESP_OK) {
        tv_ota_start_task();
    } else {
        ESP_LOGW(TAG, "Wi-Fi no configurado; OTA pendiente, SHT31/IA siguen activos");
    }
}
