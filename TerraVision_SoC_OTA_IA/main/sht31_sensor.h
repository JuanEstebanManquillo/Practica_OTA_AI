/** @file sht31_sensor.h
 *  @brief Lector de temperatura y humedad SHT31 mediante I²C (ESP-IDF 5.3).
 */
#pragma once
#include "esp_err.h"

/** @brief Instala bus I2C y registra sensor (la presencia física se verifica al leer). */
esp_err_t tv_sht31_init(void);

/** @brief Realiza medición de alta repetibilidad sin clock stretching.
 *  @param temp_c Temperatura ambiental en grados Celsius.
 *  @param humidity_rh Humedad relativa ambiental en porcentaje.
 *  @param duration_ms Tiempo de la transacción completa (opcional).
 *  @return ESP_OK solo con lectura y CRC válidos.
 */
esp_err_t tv_sht31_read(float *temp_c, float *humidity_rh, float *duration_ms);
