/** @file ai_engine.h
 *  @brief Análisis adaptativo local de temperatura, humedad y salud I2C.
 *  Inferencia estadística ligera (z-score); no es una red neuronal.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define TV_AI_WINDOW 16
#define TV_AI_MIN_BASELINE 8
#define TV_AI_Z_LIMIT 2.8f

typedef enum {
    TV_HEALTH_OK = 0,
    TV_HEALTH_WARNING = 1,
    TV_HEALTH_CRITICAL = 2
} tv_health_t;

typedef struct {
    float temperature[TV_AI_WINDOW];
    float humidity[TV_AI_WINDOW];
    float latency_ms[TV_AI_WINDOW];
    uint8_t temperature_count, temperature_pos;
    uint8_t humidity_count, humidity_pos;
    uint8_t latency_count, latency_pos;
    uint32_t total_ok, total_fail;
    uint8_t consecutive_fail;
    tv_health_t health;
} tv_ai_t;

typedef struct {
    bool alert_temperature;
    bool alert_humidity;
    bool alert_latency;
    float temperature_z, humidity_z, latency_z;
    uint8_t consecutive_fail;
    tv_health_t health;
} tv_ai_result_t;

/** @brief Inicializa tres ventanas: temperatura, humedad y tiempo de lectura. */
void tv_ai_init(tv_ai_t *model);
/** @brief Evalúa una lectura real o de DEMO (con el indicador correspondiente).
 *  @param success Verdadero cuando el SHT31 respondió y pasó verificación CRC.
 *  @param temp_c Temperatura ambiental en °C.
 *  @param humidity_rh Humedad relativa ambiental en %.
 *  @param latency_ms Tiempo de transacción I2C en milisegundos.
 */
tv_ai_result_t tv_ai_update(tv_ai_t *model, bool success,
                            float temp_c, float humidity_rh, float latency_ms);
/** @brief Validación local del motor estadístico. */
bool tv_ai_selftest(void);
/** @brief Texto del estado de salud. */
const char *tv_ai_health_name(tv_health_t health);
