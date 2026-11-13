/** @file sht31_sensor.c @brief Controlador SHT31 I2C para ESP-IDF 5.3. */
#include "sht31_sensor.h"
#include "sht31_codec.h"
#include "tv_config.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SHT31";
static i2c_master_bus_handle_t bus;
static i2c_master_dev_handle_t device;

esp_err_t tv_sht31_init(void)
{
    if (device) return ESP_OK;
    i2c_master_bus_config_t conf = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = TV_SHT31_SCL_GPIO,
        .sda_io_num = TV_SHT31_SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t err = i2c_new_master_bus(&conf, &bus);
    if (err != ESP_OK) return err;
    const i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TV_SHT31_ADDRESS,
        .scl_speed_hz = 100000,
    };
    err = i2c_master_bus_add_device(bus, &cfg, &device);
    if (err != ESP_OK) {
        i2c_del_master_bus(bus);
        bus = NULL;
        return err;
    }
    ESP_LOGI(TAG, "Bus I2C SDA=GPIO%d SCL=GPIO%d, SHT31 0x%02X",
        TV_SHT31_SDA_GPIO, TV_SHT31_SCL_GPIO, TV_SHT31_ADDRESS);
    // La ausencia de sensor no impide arrancar ni confirmar el firmware OTA.
    err = i2c_master_probe(bus, TV_SHT31_ADDRESS, 200);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "SHT31 no responde al sondeo inicial: %s; revisar cableado/pull-ups",
                 esp_err_to_name(err));
    }
    return ESP_OK;
}

esp_err_t tv_sht31_read(float *temp_c, float *humidity_rh, float *duration_ms)
{
    if (!temp_c || !humidity_rh) return ESP_ERR_INVALID_ARG;
    if (!device) return ESP_ERR_INVALID_STATE;
    const int64_t start = esp_timer_get_time();
    // Single-shot, high repeatability, clock stretching deshabilitado (SHT3x).
    const uint8_t command[2] = {0x24, 0x00};
    esp_err_t err = i2c_master_transmit(device, command, 2, 250);
    if (err == ESP_OK) {
        vTaskDelay(pdMS_TO_TICKS(20));  // Conversión ~15 ms (margen).
        uint8_t packet[6] = {0};
        err = i2c_master_receive(device, packet, sizeof(packet), 250);
        if (err == ESP_OK && !tv_sht31_decode(packet, temp_c, humidity_rh)) {
            ESP_LOGW(TAG, "Respuesta SHT31 con CRC incorrecto");
            err = ESP_ERR_INVALID_CRC;
        }
    }
    if (duration_ms) *duration_ms = (float)(esp_timer_get_time() - start) / 1000.0f;
    return err;
}
