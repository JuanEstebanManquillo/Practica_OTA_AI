/** @file ai_engine.c @brief Detector estadístico de anomalías en el ESP32-S3. */
#include "ai_engine.h"
#include <math.h>
#include <string.h>

static float zscore(const float *values, uint8_t n, float current, float sd_floor)
{
    if (n < TV_AI_MIN_BASELINE) return 0.0f;
    float sum = 0.0f;
    for (uint8_t i = 0; i < n; ++i) sum += values[i];
    const float avg = sum / n;
    float var = 0.0f;
    for (uint8_t i = 0; i < n; ++i) {
        const float diff = values[i] - avg;
        var += diff * diff;
    }
    const float sd = sqrtf(var / n);
    return (current - avg) / fmaxf(sd, sd_floor);
}

static void push(float *buffer, uint8_t *count, uint8_t *pos, float x)
{
    buffer[*pos] = x;
    *pos = (uint8_t)((*pos + 1U) % TV_AI_WINDOW);
    if (*count < TV_AI_WINDOW) ++(*count);
}

void tv_ai_init(tv_ai_t *model)
{
    memset(model, 0, sizeof(*model));
    model->health = TV_HEALTH_OK;
}

tv_ai_result_t tv_ai_update(tv_ai_t *m, bool success,
                            float temp_c, float humidity_rh, float latency_ms)
{
    tv_ai_result_t r = {0};
    if (success && (!isfinite(temp_c) || !isfinite(humidity_rh) ||
        !isfinite(latency_ms) || humidity_rh < 0.0f || humidity_rh > 100.0f)) {
        success = false;
    }
    if (!success) {
        ++m->total_fail;
        if (m->consecutive_fail < 255) ++m->consecutive_fail;
    } else {
        ++m->total_ok;
        m->consecutive_fail = 0;
        r.temperature_z = zscore(m->temperature, m->temperature_count, temp_c, 0.25f);
        r.humidity_z = zscore(m->humidity, m->humidity_count, humidity_rh, 1.0f);
        r.latency_z = zscore(m->latency_ms, m->latency_count, latency_ms, 2.0f);
        r.alert_temperature = (m->temperature_count >= TV_AI_MIN_BASELINE &&
                               fabsf(r.temperature_z) > TV_AI_Z_LIMIT);
        r.alert_humidity = (m->humidity_count >= TV_AI_MIN_BASELINE &&
                            fabsf(r.humidity_z) > TV_AI_Z_LIMIT);
        r.alert_latency = (m->latency_count >= TV_AI_MIN_BASELINE &&
                           r.latency_z > TV_AI_Z_LIMIT);
        // No incorporar valores atípicos a su propia línea base.
        if (!r.alert_temperature) {
            push(m->temperature, &m->temperature_count, &m->temperature_pos, temp_c);
        }
        if (!r.alert_humidity) {
            push(m->humidity, &m->humidity_count, &m->humidity_pos, humidity_rh);
        }
        if (!r.alert_latency) {
            push(m->latency_ms, &m->latency_count, &m->latency_pos, latency_ms);
        }
    }
    if (m->consecutive_fail >= 5) {
        m->health = TV_HEALTH_CRITICAL;
    } else if (m->consecutive_fail >= 2 || r.alert_temperature ||
               r.alert_humidity || r.alert_latency) {
        m->health = TV_HEALTH_WARNING;
    } else {
        m->health = TV_HEALTH_OK;
    }
    r.consecutive_fail = m->consecutive_fail;
    r.health = m->health;
    return r;
}

const char *tv_ai_health_name(tv_health_t h)
{
    switch (h) {
        case TV_HEALTH_OK: return "NORMAL";
        case TV_HEALTH_WARNING: return "ALERTA";
        case TV_HEALTH_CRITICAL: return "CRITICO";
        default: return "DESCONOCIDO";
    }
}

bool tv_ai_selftest(void)
{
    tv_ai_t model;
    tv_ai_init(&model);
    for (int i = 0; i < TV_AI_MIN_BASELINE; ++i) {
        (void)tv_ai_update(&model, true, 24.0f + 0.1f * (float)(i % 3),
                           55.0f + (float)(i % 2), 20.0f);
    }
    const tv_ai_result_t temp = tv_ai_update(&model, true, 37.0f, 55.0f, 20.0f);
    const tv_ai_result_t hum = tv_ai_update(&model, true, 24.0f, 82.0f, 20.0f);
    return temp.alert_temperature && !temp.alert_humidity && hum.alert_humidity;
}
