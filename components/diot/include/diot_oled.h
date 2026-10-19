#ifndef DIOT_OLED_H
#define DIOT_OLED_H

/**
 * @file diot_oled.h
 * @brief Local SSD1306 display interface (HMI output only).
 *
 * CURRENT STATUS: MOCK backend. Output is logged, not drawn over I2C.
 * The physical implementation will preserve this interface.
 *
 * This local display is not a Matter endpoint.
 */

#include "esp_err.h"

#include "diot_bme680.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque display handle.
 */
typedef struct diot_oled_t diot_oled_t;

diot_oled_t *diot_oled_fsm_new(uint32_t period_ms, const diot_bme680_t *sensor);
void diot_oled_fsm_destroy(diot_oled_t *dev);
void diot_oled_fsm_fire(diot_oled_t *dev);

/**
 * @brief Allocate and initialize the display.
 *
 * MOCK: no hardware access or I2C initialization.
 *
 * @return Display handle, or NULL on allocation failure.
 */
diot_oled_t *diot_oled_new(void);

/**
 * @brief Show a local status line, such as commissioning or connection status.
 *
 * MOCK: print the line to the serial monitor.
 */
void diot_oled_show_status(diot_oled_t *dev, const char *line);

/**
 * @brief Show the local page with all four sample fields.
 *
 * MOCK: draw an ASCII frame in the serial monitor.
 *
 * @param dev Handle created with diot_oled_new().
 * @param data Sample to display; must not be NULL.
 */
void diot_oled_show_measurements(diot_oled_t *dev, const diot_bme680_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* DIOT_OLED_H */
