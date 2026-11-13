/** @file wifi_manager.c @brief Conexion Wi-Fi con gestion de eventos. */
#include "wifi_manager.h"
#include "tv_config.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#define WIFI_CONNECTED_BIT BIT0
static EventGroupHandle_t event_group;
static const char *TAG = "WIFI";

static void event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(event_group, WIFI_CONNECTED_BIT);
        ESP_LOGW(TAG, "Wi-Fi perdido, reintentando");
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *ev = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "IP adquirida: " IPSTR, IP2STR(&ev->ip_info.ip));
        xEventGroupSetBits(event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t tv_wifi_start(void)
{
    if (strcmp(TV_WIFI_SSID, "CAMBIAR_SSID") == 0 || TV_WIFI_SSID[0] == '\0') {
        ESP_LOGE(TAG, "Wi-Fi no configurado: crear main/tv_secrets.h");
        return ESP_ERR_INVALID_STATE;
    }
    event_group = xEventGroupCreate();
    if (!event_group) return ESP_ERR_NO_MEM;
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL));
    wifi_config_t wifi = {0};
    strlcpy((char *)wifi.sta.ssid, TV_WIFI_SSID, sizeof(wifi.sta.ssid));
    strlcpy((char *)wifi.sta.password, TV_WIFI_PASS, sizeof(wifi.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Asociando con red local");
    return ESP_OK;
}

bool tv_wifi_connected(void)
{
    if (!event_group) return false;
    return (xEventGroupGetBits(event_group) & WIFI_CONNECTED_BIT) != 0;
}
