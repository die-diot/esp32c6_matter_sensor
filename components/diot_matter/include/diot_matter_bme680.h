#ifndef DIOT_MATTER_BME680_H
#define DIOT_MATTER_BME680_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t diot_matter_update_temperature(float celsius);
/* TODO(P2-3): Declare the humidity update function, following the temperature API. */
esp_err_t diot_matter_update_pressure(float hpa);
esp_err_t diot_matter_update_gas_resistance(float ohms);

#ifdef __cplusplus
}
#endif

#endif