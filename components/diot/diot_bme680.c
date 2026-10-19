/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/**
 * @file diot_bme680.c
 * @brief Mock BME680 backend with simulated measurements and a sampling FSM.
 *
 * Generates plausible random samples instead of reading hardware.
 * A physical backend must preserve the public sample and callback contract.
 */

#include <stdlib.h>

#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"

#include "diot_bme680.h"

static const char *TAG = "diot_bme680";

struct diot_bme680_t {
    fsm_t fsm;
    uint32_t reads; /*!< Mock sample counter. */
    int64_t next_us;
    int64_t period_us;
    diot_bme680_data_t data;
    bool has_data;
    diot_bme680_callback_t callback;
    void *callback_arg;
};

/* Return a random float in [min, max). */
static float random_in_range(float min, float max)
{
    float span = max - min;
    return min + (span * (float)(esp_random() % 10000) / 10000.0f);
}

static bool check_next(const fsm_t *machine)
{
    const diot_bme680_t *dev = (const diot_bme680_t *)machine;
    return esp_timer_get_time() >= dev->next_us;
}

static void do_measure(fsm_t *machine)
{
    diot_bme680_t *dev = (diot_bme680_t *)machine;
    diot_bme680_data_t data;
    if (diot_bme680_read(dev, &data) == ESP_OK) {
        dev->data = data;
        dev->has_data = true;
        if (dev->callback != NULL) {
            dev->callback(&dev->data, dev->callback_arg);
        }
    }
    dev->next_us = esp_timer_get_time() + dev->period_us;
}

diot_bme680_t *diot_bme680_new(void)
{
    static fsm_trans_t transitions[] = {
        {0, check_next, 0, do_measure},
        {-1, NULL, -1, NULL},
    };
    diot_bme680_t *dev = (diot_bme680_t *)calloc(1, sizeof(diot_bme680_t));
    if (dev == NULL) {
        ESP_LOGE(TAG, "Failed to allocate the sensor");
        return NULL;
    }
    fsm_init(&dev->fsm, transitions);
    dev->next_us = esp_timer_get_time();
    dev->period_us = 5000000;
    ESP_LOGW(TAG, "BME680 MOCK: random samples, no physical sensor reads");
    return dev;
}

diot_bme680_t *diot_bme680_fsm_new(uint32_t period_ms)
{
    if (period_ms == 0) {
        return NULL;
    }
    diot_bme680_t *dev = diot_bme680_new();
    if (dev != NULL) {
        dev->period_us = (int64_t)period_ms * 1000;
    }
    return dev;
}

void diot_bme680_fsm_destroy(diot_bme680_t *dev)
{
    free(dev);
}

void diot_bme680_fsm_fire(diot_bme680_t *dev)
{
    if (dev != NULL) {
        fsm_fire(&dev->fsm);
    }
}

void diot_bme680_fsm_register_callback(diot_bme680_t *dev, diot_bme680_callback_t callback, void *arg)
{
    if (dev != NULL) {
        dev->callback = callback;
        dev->callback_arg = arg;
    }
}

bool diot_bme680_get_data(const diot_bme680_t *dev, diot_bme680_data_t *out)
{
    if (dev == NULL || out == NULL || !dev->has_data) {
        return false;
    }
    *out = dev->data;
    return true;
}

esp_err_t diot_bme680_read(diot_bme680_t *dev, diot_bme680_data_t *out)
{
    if (dev == NULL || out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    out->temperature_c = random_in_range(20.0f, 30.0f);
    out->humidity_pct = random_in_range(40.0f, 60.0f);
    out->pressure_hpa = random_in_range(990.0f, 1030.0f);
    out->gas_resistance_ohm = random_in_range(10000.0f, 100000.0f);

    dev->reads++;
    return ESP_OK;
}
