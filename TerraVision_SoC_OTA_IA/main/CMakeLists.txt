idf_component_register(
    SRCS "main.c" "wifi_manager.c" "ota_manager.c"
         "sht31_sensor.c" "sht31_codec.c" "ai_engine.c" "status_led.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_wifi esp_netif esp_event nvs_flash esp_https_ota esp_http_client
                  esp_partition app_update esp_app_format esp_driver_i2c esp_driver_gpio esp_timer
    EMBED_TXTFILES "certs/server_cert.pem"
)
