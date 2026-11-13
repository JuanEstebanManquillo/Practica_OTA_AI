/** @file status_led.c @brief LED externo; no hay LED en la PCB actual. */
#include "status_led.h"
#include "tv_config.h"
#include "esp_app_desc.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "LED";
#if TV_STATUS_LED_ENABLED
static void led_set(int on)
{
    gpio_set_level((gpio_num_t)TV_STATUS_LED_GPIO,
        on ? TV_STATUS_LED_ACTIVE_HIGH : !TV_STATUS_LED_ACTIVE_HIGH);
}
static void task(void *unused)
{
    (void)unused;
    const int pulses = strcmp(esp_app_get_description()->version, TV_TARGET_VERSION) == 0 ? 2 : 1;
    for (;;) {
        for (int i = 0; i < pulses; ++i) {
            led_set(1); vTaskDelay(pdMS_TO_TICKS(180));
            led_set(0); vTaskDelay(pdMS_TO_TICKS(250));
        }
        vTaskDelay(pdMS_TO_TICKS(4500));
    }
}
#endif
void tv_led_start(void)
{
#if TV_STATUS_LED_ENABLED
    gpio_config_t c = {
        .pin_bit_mask = (1ULL << TV_STATUS_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&c));
    xTaskCreate(task, "version_led", 2048, NULL, 2, NULL);
    ESP_LOGI(TAG, "LED externo en GPIO%d: 1 destello v1 / 2 destellos v2", TV_STATUS_LED_GPIO);
#else
    ESP_LOGW(TAG, "LED DESACTIVADO: PCB no tiene LED cableado; verificar version por USB");
#endif
}
