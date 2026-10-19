#ifndef DIOT_BME680_H
#define DIOT_BME680_H

/**
 * @file diot_bme680.h
 * @brief Environmental sensor interface: temperature, humidity, pressure and gas resistance.
 *
 * CURRENT STATUS: MOCK backend. Samples are simulated, not read over I2C/SPI.
 * The instructor will integrate the physical driver behind this interface.
 * The student exercise changes the Matter model/publication, not the driver.
 */

#include <stdint.h>

#include "esp_err.h"
#include "fsm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief One complete environmental sample.
 */
typedef struct {
    float temperature_c;        /*!< Temperature in degrees Celsius. */
    float humidity_pct;         /*!< Relative humidity in percent. */
    float pressure_hpa;         /*!< Pressure in hectopascals. */
    float gas_resistance_ohm;
} diot_bme680_data_t;

/**
 * @brief Opaque sensor handle.
 */
typedef struct diot_bme680_t diot_bme680_t;

typedef void (*diot_bme680_callback_t)(const diot_bme680_data_t *data, void *arg);

diot_bme680_t *diot_bme680_fsm_new(uint32_t period_ms);
void diot_bme680_fsm_destroy(diot_bme680_t *dev);
void diot_bme680_fsm_fire(diot_bme680_t *dev);
void diot_bme680_fsm_register_callback(diot_bme680_t *dev, diot_bme680_callback_t callback, void *arg);
bool diot_bme680_get_data(const diot_bme680_t *dev, diot_bme680_data_t *out);

/**
 * @brief Allocate and initialize the sensor.
 *
 * MOCK: allocate state without accessing hardware.
 *
 * @return Sensor handle, or NULL on allocation failure.
 */
diot_bme680_t *diot_bme680_new(void);

/**
 * @brief Read one environmental sample.
 *
 * MOCK: plausible random values (20-30 C, 40-60 percent RH,
 * 990-1030 hPa, gas resistance 10000-100000 ohms). No CO2 or IAQ estimate.
 *
 * @param dev Handle created with diot_bme680_new().
 * @param out Sample destination; must not be NULL.
 * @return ESP_OK on success; ESP_ERR_INVALID_ARG for a NULL pointer.
 */
esp_err_t diot_bme680_read(diot_bme680_t *dev, diot_bme680_data_t *out);

#ifdef __cplusplus
}
#endif

#endif /* DIOT_BME680_H */
