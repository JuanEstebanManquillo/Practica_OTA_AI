/** @file test_ai.c @brief Pruebas de inferencia de temperatura/humedad. */
#include "ai_engine.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    assert(tv_ai_selftest());
    tv_ai_t m;
    tv_ai_init(&m);
    for (int i = 0; i < 10; ++i) {
        tv_ai_result_t out = tv_ai_update(&m, true, 24.0f + 0.1f * (i % 3),
                                          55.0f + (float)(i % 2), 20.0f);
        assert(!out.alert_temperature && !out.alert_humidity);
    }
    tv_ai_result_t result = tv_ai_update(&m, true, 36.0f, 55.0f, 20.0f);
    assert(result.alert_temperature && !result.alert_humidity);
    result = tv_ai_update(&m, true, 24.0f, 85.0f, 20.0f);
    assert(result.alert_humidity && !result.alert_temperature);
    result = tv_ai_update(&m, true, 24.0f, 55.0f, 20.0f);
    assert(!result.alert_temperature && !result.alert_humidity && result.health == TV_HEALTH_OK);
    for (int i = 0; i < 5; ++i) result = tv_ai_update(&m, false, 0, 0, 200.0f);
    assert(result.consecutive_fail == 5 && result.health == TV_HEALTH_CRITICAL);
    result = tv_ai_update(&m, true, 24.0f, 55.0f, 20.0f);
    assert(result.health == TV_HEALTH_OK);
    printf("PASS: IA SHT31, T, HR, recuperacion, fallas I2C\n");
    return 0;
}
