/** @file sht31_codec.h
 *  @brief Conversión de mediciones SHT31 y comprobación CRC8.
 *  Código independiente de ESP-IDF, probado en computadora.
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @brief CRC8 SHT3x (polinomio 0x31, inicialización 0xFF). */
uint8_t tv_sht31_crc8(const uint8_t *data, size_t size);

/** @brief Decodifica 6 bytes (T MSB, T LSB, CRC, HR MSB, HR LSB, CRC).
 *  @return true si ambos CRC son correctos y los valores son coherentes.
 */
bool tv_sht31_decode(const uint8_t data[6], float *temp_c, float *humidity_rh);
