#include "diot_matter_bme680.h"

#include <math.h>

#include "diot_matter_devices.h"
#include "diot_matter.h"

enum {
    TEMPERATURE,
    /* TODO(P2-1): Add the humidity index; keep SENSOR_COUNT last. */
    PRESSURE,
    GAS_RESISTANCE,
    SENSOR_COUNT,
};

static const diot_matter_endpoint_config_t s_endpoints[SENSOR_COUNT] = {
    [TEMPERATURE] = {
        .device_type_id = DIOT_MATTER_TEMPERATURE_SENSOR,
        .cluster_id = DIOT_MATTER_CLUSTER_TEMPERATURE_MEASUREMENT,
        .attribute_id = DIOT_MATTER_ATTRIBUTE_MEASURED_VALUE,
    },
    /* TODO(P2-2): Adapt the temperature entry for the standard humidity model. */
    [PRESSURE] = {
        .device_type_id = DIOT_MATTER_PRESSURE_SENSOR,
        .cluster_id = DIOT_MATTER_CLUSTER_PRESSURE_MEASUREMENT,
        .attribute_id = DIOT_MATTER_ATTRIBUTE_MEASURED_VALUE,
    },
    [GAS_RESISTANCE] = {
        .device_type_id = DIOT_MATTER_GAS_RESISTANCE_SENSOR,
        .cluster_id = DIOT_MATTER_CLUSTER_GAS_RESISTANCE,
        .attribute_id = DIOT_MATTER_ATTRIBUTE_MEASURED_VALUE,
    },
};

static uint16_t s_endpoint_ids[SENSOR_COUNT];

esp_err_t diot_matter_init(void)
{
    return diot_matter_init_with_endpoints(s_endpoints, SENSOR_COUNT, s_endpoint_ids);
}

esp_err_t diot_matter_update_temperature(float celsius)
{
    if (!isfinite(celsius) || celsius < -273.15f || celsius > 327.67f) {
        return ESP_ERR_INVALID_ARG;
    }
    return diot_matter_set_i16(s_endpoint_ids[TEMPERATURE], s_endpoints[TEMPERATURE].cluster_id,
                               s_endpoints[TEMPERATURE].attribute_id,
                               (int16_t)lroundf(celsius * 100.0f));
}

/* TODO(P2-4): Implement humidity publication using the reference units, range
 * and C writer type. Use the returned endpoint ID and propagate errors. */

esp_err_t diot_matter_update_pressure(float hpa)
{
    if (!isfinite(hpa) || hpa < 0.0f || hpa > 32767.0f) {
        return ESP_ERR_INVALID_ARG;
    }
    return diot_matter_set_i16(s_endpoint_ids[PRESSURE], s_endpoints[PRESSURE].cluster_id,
                               s_endpoints[PRESSURE].attribute_id, (int16_t)lroundf(hpa));
}

esp_err_t diot_matter_update_gas_resistance(float ohms)
{
    if (!isfinite(ohms) || ohms < 0.0f) {
        return ESP_ERR_INVALID_ARG;
    }
    return diot_matter_set_float(s_endpoint_ids[GAS_RESISTANCE], s_endpoints[GAS_RESISTANCE].cluster_id,
                                 s_endpoints[GAS_RESISTANCE].attribute_id, ohms);
}