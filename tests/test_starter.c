#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "diot_matter.h"

static uint16_t s_endpoint;
static uint32_t s_cluster;
static double s_value;
static unsigned s_writes;
static esp_err_t s_result = ESP_OK;

esp_err_t diot_matter_init_with_endpoints(const diot_matter_endpoint_config_t *configs,
                                         size_t count, uint16_t *endpoint_ids)
{
    static const uint32_t types[] = {
        DIOT_MATTER_TEMPERATURE_SENSOR,
        DIOT_MATTER_PRESSURE_SENSOR,
        DIOT_MATTER_GAS_RESISTANCE_SENSOR,
    };
    static const uint32_t clusters[] = {
        DIOT_MATTER_CLUSTER_TEMPERATURE_MEASUREMENT,
        DIOT_MATTER_CLUSTER_PRESSURE_MEASUREMENT,
        DIOT_MATTER_CLUSTER_GAS_RESISTANCE,
    };
    assert(count == sizeof(types) / sizeof(types[0]));
    for (size_t index = 0; index < count; ++index) {
        assert(configs[index].device_type_id == types[index]);
        assert(configs[index].cluster_id == clusters[index]);
        assert(configs[index].attribute_id == DIOT_MATTER_ATTRIBUTE_MEASURED_VALUE);
        endpoint_ids[index] = (uint16_t)(20 + index);
    }
    return s_result;
}

static esp_err_t record_value(uint16_t endpoint_id, uint32_t cluster_id,
                               uint32_t attribute_id, double value)
{
    assert(attribute_id == DIOT_MATTER_ATTRIBUTE_MEASURED_VALUE);
    s_endpoint = endpoint_id;
    s_cluster = cluster_id;
    s_value = value;
    ++s_writes;
    return s_result;
}

esp_err_t diot_matter_set_i16(uint16_t endpoint_id, uint32_t cluster_id,
                             uint32_t attribute_id, int16_t value)
{
    return record_value(endpoint_id, cluster_id, attribute_id, value);
}

esp_err_t diot_matter_set_float(uint16_t endpoint_id, uint32_t cluster_id,
                               uint32_t attribute_id, float value)
{
    return record_value(endpoint_id, cluster_id, attribute_id, value);
}

int main(void)
{
    assert(diot_matter_init() == ESP_OK);
    assert(diot_matter_update_temperature(21.15f) == ESP_OK);
    assert(s_endpoint == 20 && s_cluster == DIOT_MATTER_CLUSTER_TEMPERATURE_MEASUREMENT);
    assert(s_value == 2115);
    assert(diot_matter_update_pressure(1001.6f) == ESP_OK);
    assert(s_endpoint == 21 && s_cluster == DIOT_MATTER_CLUSTER_PRESSURE_MEASUREMENT);
    assert(s_value == 1002);
    assert(diot_matter_update_gas_resistance(45000.5f) == ESP_OK);
    assert(s_endpoint == 22 && s_cluster == DIOT_MATTER_CLUSTER_GAS_RESISTANCE);
    assert(s_value == 45000.5);
    unsigned writes = s_writes;
    assert(diot_matter_update_temperature(NAN) == ESP_ERR_INVALID_ARG);
    assert(diot_matter_update_pressure(-1) == ESP_ERR_INVALID_ARG);
    assert(diot_matter_update_gas_resistance(INFINITY) == ESP_ERR_INVALID_ARG);
    assert(s_writes == writes);
    s_result = ESP_FAIL;
    assert(diot_matter_update_pressure(1000) == ESP_FAIL);
    puts("Uncompleted starter: three valid provided endpoints, returned IDs, units and errors OK");
    return 0;
}