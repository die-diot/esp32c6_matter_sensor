#ifndef DIOT_MATTER_H
#define DIOT_MATTER_H

/**
 * @file diot_matter.h
 * @brief Matter stack wrapper with a plain C application API.
 *
 * Hides C++ node/endpoint construction, commissioning and network startup.
 * The student model is defined in C in diot_matter_bme680.c.
 *
 * Temperature, pressure and laboratory gas resistance are provided.
 * The humidity model and publication are student TODOs.
 * Matter assigns endpoint IDs; application code must use the returned IDs.
 *
 * Application updates change attributes; controllers may read or subscribe.
 */

#include <stdint.h>

#include "esp_err.h"
#include "diot_matter_bme680.h"
#include "diot_matter_devices.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start Matter with the C model currently declared in the adapter.
 *
 * Initialize NVS, create the configured endpoints and start commissioning.
 * Gas resistance is expressed in ohms, never CO2 concentration.
 *
 * @return ESP_OK on success, or an error code.
 */
esp_err_t diot_matter_init(void);
esp_err_t diot_matter_init_with_endpoints(const diot_matter_endpoint_config_t *configs,
										 size_t count, uint16_t *endpoint_ids);

/**
 * @brief Request a Matter factory reset, for example after a long BOOT press.
 *
 * The wrapper handles the Matter operation internally.
 */
void diot_matter_factory_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* DIOT_MATTER_H */
