#ifndef DIOT_MATTER_DEVICES_H
#define DIOT_MATTER_DEVICES_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#define DIOT_MATTER_TEMPERATURE_SENSOR 0x0302u
#define DIOT_MATTER_HUMIDITY_SENSOR 0x0307u
#define DIOT_MATTER_PRESSURE_SENSOR 0x0305u
#define DIOT_MATTER_GAS_RESISTANCE_SENSOR 0xFFF10001u
#define DIOT_MATTER_CLUSTER_TEMPERATURE_MEASUREMENT 0x0402u
#define DIOT_MATTER_CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT 0x0405u
#define DIOT_MATTER_CLUSTER_PRESSURE_MEASUREMENT 0x0403u
#define DIOT_MATTER_CLUSTER_GAS_RESISTANCE 0xFFF1FC00u
#define DIOT_MATTER_ATTRIBUTE_MEASURED_VALUE 0x0000u
#define DIOT_MATTER_MAX_SENSOR_ENDPOINTS 4u

typedef struct {
    uint32_t device_type_id;
    uint32_t cluster_id;
    uint32_t attribute_id;
} diot_matter_endpoint_config_t;

#ifdef __cplusplus
#include "esp_matter.h"

namespace diot::matter::devices {
esp_err_t validate_sensor_model(const diot_matter_endpoint_config_t *configs, size_t count);
esp_err_t create_sensor_device(esp_matter::node_t *node,
                               const diot_matter_endpoint_config_t *configs,
                               size_t count, uint16_t *endpoint_ids);
esp_err_t identification_cb(esp_matter::identification::callback_type_t type, uint16_t endpoint_id,
                            uint8_t effect_id, uint8_t effect_variant, void *priv_data);
esp_err_t attribute_update_cb(esp_matter::attribute::callback_type_t type, uint16_t endpoint_id,
                              uint32_t cluster_id, uint32_t attribute_id,
                              esp_matter_attr_val_t *val, void *priv_data);
}

extern "C" {
#endif

esp_err_t diot_matter_set_i16(uint16_t endpoint_id, uint32_t cluster_id,
                             uint32_t attribute_id, int16_t value);
esp_err_t diot_matter_set_u16(uint16_t endpoint_id, uint32_t cluster_id,
                             uint32_t attribute_id, uint16_t value);
esp_err_t diot_matter_set_float(uint16_t endpoint_id, uint32_t cluster_id,
                               uint32_t attribute_id, float value);

#ifdef __cplusplus
}
#endif

#endif