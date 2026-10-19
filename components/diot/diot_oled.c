/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/**
 * @file diot_oled.c
 * @brief Mock SSD1306 display: samples are printed to the serial monitor.
 *
 * A physical display backend must preserve the diot_oled.h interface.
 */

#include <stdlib.h>

#include "esp_log.h"
#include "esp_timer.h"

#include "diot_oled.h"

static const char *TAG = "diot_oled";

struct diot_oled_t {
    fsm_t fsm;
    uint32_t frames; /*!< Mock display refresh counter. */
    int64_t next_us;
    int64_t period_us;
    const diot_bme680_t *sensor;
};

static bool check_next(const fsm_t *machine)
{
    const diot_oled_t *dev = (const diot_oled_t *)machine;
    return esp_timer_get_time() >= dev->next_us;
}

static void do_print_data(fsm_t *machine)
{
    diot_oled_t *dev = (diot_oled_t *)machine;
    diot_bme680_data_t data;
    if (diot_bme680_get_data(dev->sensor, &data)) {
        diot_oled_show_measurements(dev, &data);
    }
    dev->next_us = esp_timer_get_time() + dev->period_us;
}

diot_oled_t *diot_oled_new(void)
{
    static fsm_trans_t transitions[] = {
        {0, check_next, 0, do_print_data},
        {-1, NULL, -1, NULL},
    };
    diot_oled_t *dev = (diot_oled_t *)calloc(1, sizeof(diot_oled_t));
    if (dev == NULL) {
        ESP_LOGE(TAG, "Failed to allocate the display");
        return NULL;
    }
    fsm_init(&dev->fsm, transitions);
    dev->next_us = esp_timer_get_time();
    dev->period_us = 1000000;
    ESP_LOGW(TAG, "SSD1306 MOCK: display output is printed to the serial monitor");
    return dev;
}

diot_oled_t *diot_oled_fsm_new(uint32_t period_ms, const diot_bme680_t *sensor)
{
    if (period_ms == 0 || sensor == NULL) {
        return NULL;
    }
    diot_oled_t *dev = diot_oled_new();
    if (dev != NULL) {
        dev->period_us = (int64_t)period_ms * 1000;
        dev->sensor = sensor;
    }
    return dev;
}

void diot_oled_fsm_destroy(diot_oled_t *dev)
{
    free(dev);
}

void diot_oled_fsm_fire(diot_oled_t *dev)
{
    if (dev != NULL) {
        fsm_fire(&dev->fsm);
    }
}

void diot_oled_show_status(diot_oled_t *dev, const char *line)
{
    if (dev == NULL || line == NULL) {
        return;
    }
    ESP_LOGI(TAG, "[OLED] %s", line);
}

void diot_oled_show_measurements(diot_oled_t *dev, const diot_bme680_data_t *data)
{
    if (dev == NULL || data == NULL) {
        return;
    }

    dev->frames++;

    /* Render the local display as a serial text frame. */
    ESP_LOGI(TAG, "+------------ DIOT BME680 ------------+");
    ESP_LOGI(TAG, "| Temp : %6.2f  C                    |", data->temperature_c);
    ESP_LOGI(TAG, "| Hum  : %6.2f  %%                    |", data->humidity_pct);
    ESP_LOGI(TAG, "| Pres : %7.2f hPa                   |", data->pressure_hpa);
    ESP_LOGI(TAG, "| Gas  : %7.1f ohm (MOCK)            |", data->gas_resistance_ohm);
    ESP_LOGI(TAG, "+-------------------------------------+");
}
