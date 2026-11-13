/** @file sht31_codec.c @brief Implementación portable de CRC y conversión SHT31. */
#include "sht31_codec.h"
#include <math.h>

uint8_t tv_sht31_crc8(const uint8_t *data, size_t size)
{
    uint8_t crc = 0xFF;
    if (!data) return crc;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80U) ? (uint8_t)((crc << 1) ^ 0x31U)
                                : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

bool tv_sht31_decode(const uint8_t data[6], float *temp_c, float *humidity_rh)
{
    if (!data || !temp_c || !humidity_rh) return false;
    if (tv_sht31_crc8(data, 2) != data[2] ||
        tv_sht31_crc8(&data[3], 2) != data[5]) return false;

    uint16_t raw_t = (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
    uint16_t raw_h = (uint16_t)(((uint16_t)data[3] << 8) | data[4]);
    float t = -45.0f + 175.0f * (float)raw_t / 65535.0f;
    float h = 100.0f * (float)raw_h / 65535.0f;
    if (!isfinite(t) || !isfinite(h) || h < 0.0f || h > 100.0f) return false;
    *temp_c = t;
    *humidity_rh = h;
    return true;
}
