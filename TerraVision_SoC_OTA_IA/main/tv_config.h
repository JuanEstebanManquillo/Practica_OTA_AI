/** @file tv_config.h @brief Configuración local del nodo TerraVision SHT31. */
#pragma once
#include <stdint.h>

#if __has_include("tv_secrets.h")
#  include "tv_secrets.h"
#else
#  include "tv_secrets.example.h"
#endif

/** 0 = leer el SHT31 físico; 1 = demo sintética explícitamente identificada. */
#ifndef TV_DEMO_MODE
#define TV_DEMO_MODE 0
#endif

/** 1 solo con LED EXTERNO conectado físicamente a un GPIO libre. */
#ifndef TV_STATUS_LED_ENABLED
#define TV_STATUS_LED_ENABLED 0
#endif
#define TV_STATUS_LED_GPIO 48
#define TV_STATUS_LED_ACTIVE_HIGH 1

/**
 * Puerto I2C temporal a través de J_CAM de la PCB TerraVision:
 * J_CAM pin 2 = CAM_SDA (GPIO4); pin 1 = CAM_SCL (GPIO5),
 * pin 15 = 3V3; pin 16 = GND.
 * No conectar la cámara OV5640 simultáneamente a estos pines.
 */
#define TV_SHT31_SDA_GPIO 4
#define TV_SHT31_SCL_GPIO 5
/** Dirección SHT31 normal (ADDR/GND = 0x44; ADDR/3V3 = 0x45). */
#define TV_SHT31_ADDRESS 0x44

#define TV_TARGET_VERSION "2.0.0"
#define TV_SAMPLE_PERIOD_MS 2000
#define TV_OTA_RETRY_MS 30000
