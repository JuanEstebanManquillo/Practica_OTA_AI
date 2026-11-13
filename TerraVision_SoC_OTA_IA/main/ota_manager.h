/** @file ota_manager.h @brief Actualizacion HTTPS y confirmacion de rollback. */
#pragma once
#include <stdbool.h>
#include "esp_err.h"
/** @brief Confirma aplicacion OTA solo tras autotest exitoso. */
esp_err_t tv_ota_confirm_after_selftest(bool selftest_ok);
/** @brief Inicia tarea que busca version 2 via HTTPS cuando hay Wi-Fi. */
void tv_ota_start_task(void);
