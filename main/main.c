/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/**
 * @file main.c
 * @brief Student application entry point (plain C).
 *
 * Environmental sample -> C adapter -> Matter attributes.
 * The BME680 and OLED backends are mocks, not physical drivers.
 *   a) initialize the mock sensor,
 *   b) initialize the mock local display,
 *   c) initialize the managed button for factory reset,
 *   d) start Matter and run the measurement/display FSMs.
 */

#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#ifndef CONFIG_DIOT_BUTTON_LONG_PRESS_TIME_MS
#define CONFIG_DIOT_BUTTON_LONG_PRESS_TIME_MS 5000
#endif

#include "diot_bme680.h"
#include "diot_button.h"
#include "diot_matter.h"
#include "diot_matter_bme680.h"
#include "diot_oled.h"

static const char *TAG = "main";

/* Sensor sampling period in milliseconds. */
#define MEASUREMENT_PERIOD_MS 5000
#define DISPLAY_PERIOD_MS 1000
#define FSM_POLL_PERIOD_MS 20

/* Hold BOOT for at least this duration, then release to request factory reset. */
#define LONG_PRESS_MS CONFIG_DIOT_BUTTON_LONG_PRESS_TIME_MS

/* Mock driver handles shared with the common FSM task. */
static diot_bme680_t *s_sensor = NULL;
static diot_oled_t *s_oled = NULL;
static diot_button_fsm_t *s_button = NULL;

/**
 * @brief Button-to-Matter integration callback.
 *
 * The common task dispatches a validated button press after release:
 *   - short press -> log only; this application has no RGB light
 *   - long press  -> Matter factory reset
 */
static void on_button_press(uint32_t duration_ms, void *arg)
{
    if (duration_ms >= LONG_PRESS_MS) {
        ESP_LOGW(TAG, "Long press (%" PRIu32 " ms): factory reset", duration_ms);
        diot_oled_show_status(s_oled, "Factory reset...");
        diot_matter_factory_reset();
    } else {
        ESP_LOGI(TAG, "Short press (%" PRIu32 " ms): no action", duration_ms);
    }
}

static void publish_measurements(const diot_bme680_data_t *data, void *arg)
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(diot_matter_update_temperature(data->temperature_c));
    /* TODO(P2-5): Publish the sample's humidity with the same error-handling pattern. */
    ESP_ERROR_CHECK_WITHOUT_ABORT(diot_matter_update_pressure(data->pressure_hpa));
    ESP_ERROR_CHECK_WITHOUT_ABORT(diot_matter_update_gas_resistance(data->gas_resistance_ohm));
}

static void sensor_fsm_task(void *arg)
{
    TickType_t next = xTaskGetTickCount();

    while (1) {
        diot_button_fsm_fire(s_button);
        diot_bme680_fsm_fire(s_sensor);
        diot_oled_fsm_fire(s_oled);
        vTaskDelayUntil(&next, pdMS_TO_TICKS(FSM_POLL_PERIOD_MS));
    }
}

void app_main(void)
{
    /* a) Mock environmental sensor: simulated samples. */
    s_sensor = diot_bme680_fsm_new(MEASUREMENT_PERIOD_MS);
    ESP_ERROR_CHECK(s_sensor != NULL ? ESP_OK : ESP_ERR_NO_MEM);

    /* b) Local display mock: output goes to the serial monitor. */
    s_oled = diot_oled_fsm_new(DISPLAY_PERIOD_MS, s_sensor);
    ESP_ERROR_CHECK(s_oled != NULL ? ESP_OK : ESP_ERR_NO_MEM);
    diot_oled_show_status(s_oled, "Starting Matter...");

    /* c) BOOT button managed by espressif/button, as in Phase 1. */
    s_button = diot_button_fsm_new();
    ESP_ERROR_CHECK(s_button != NULL ? ESP_OK : ESP_ERR_NO_MEM);
    diot_button_fsm_register_callback(s_button, on_button_press, NULL);

    /* d) Start the provided C sensor model and Matter commissioning. */
    ESP_ERROR_CHECK(diot_matter_init());

    diot_bme680_fsm_register_callback(s_sensor, publish_measurements, NULL);
    ESP_ERROR_CHECK(xTaskCreate(sensor_fsm_task, "diot_sensor_fsm", 4096, NULL, 5, NULL) == pdPASS
                        ? ESP_OK : ESP_ERR_NO_MEM);

    ESP_LOGI(TAG, "DIOT Matter sensor starter ready (MOCK drivers)");
}
