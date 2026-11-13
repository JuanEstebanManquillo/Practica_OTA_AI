/** @file wifi_manager.h @brief Conexion a red local Wi-Fi para OTA HTTPS. */
#pragma once
#include <stdbool.h>
#include "esp_err.h"
/** @brief Configura Wi-Fi STA y conecta con red local. */
esp_err_t tv_wifi_start(void);
/** @brief Indica si hay IP valida en la interfaz STA. */
bool tv_wifi_connected(void);
