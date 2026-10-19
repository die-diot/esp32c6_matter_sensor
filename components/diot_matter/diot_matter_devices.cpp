#include "diot_matter_devices.h"

#include "esp_log.h"
#include <cmath>

#include <app/ConcreteClusterPath.h>
#include <app/data-model/Nullable.h>
#include <data_model_provider/esp_matter_data_model_provider.h>
#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>
#include <app/clusters/relative-humidity-measurement-server/RelativeHumidityMeasurementCluster.h>
#include <app/clusters/pressure-measurement-server/PressureMeasurementCluster.h>

using namespace esp_matter;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

static const char *TAG = "diot_matter_devices";
static uint16_t s_endpoint_ids[DIOT_MATTER_MAX_SENSOR_ENDPOINTS];
static diot_matter_endpoint_config_t s_configs[DIOT_MATTER_MAX_SENSOR_ENDPOINTS];
static size_t s_endpoint_count;

static chip::app::ServerClusterInterface *get_server_cluster(uint16_t endpoint_id, uint32_t cluster_id)
{
    if (endpoint_id == 0) {
        return nullptr;
    }
    return esp_matter::data_model::provider::get_instance().registry().Get(
        chip::app::ConcreteClusterPath(endpoint_id, cluster_id));
}

namespace diot::matter::devices {

esp_err_t validate_sensor_model(const diot_matter_endpoint_config_t *configs, size_t count)
{
    if (configs == nullptr || count == 0 || count > DIOT_MATTER_MAX_SENSOR_ENDPOINTS) {
        return ESP_ERR_INVALID_ARG;
    }
    size_t required_endpoints = count + 1;
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    ++required_endpoints;
#endif
    if (required_endpoints > CONFIG_ESP_MATTER_MAX_DYNAMIC_ENDPOINT_COUNT) {
        ESP_LOGE(TAG, "Model needs %u endpoints; configured capacity: %u",
                 (unsigned)required_endpoints, CONFIG_ESP_MATTER_MAX_DYNAMIC_ENDPOINT_COUNT);
        return ESP_ERR_NO_MEM;
    }
    for (size_t index = 0; index < count; ++index) {
        uint32_t expected_cluster;
        switch (configs[index].device_type_id) {
        case DIOT_MATTER_TEMPERATURE_SENSOR:
            expected_cluster = DIOT_MATTER_CLUSTER_TEMPERATURE_MEASUREMENT;
            break;
        case DIOT_MATTER_HUMIDITY_SENSOR:
            expected_cluster = DIOT_MATTER_CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT;
            break;
        case DIOT_MATTER_PRESSURE_SENSOR:
            expected_cluster = DIOT_MATTER_CLUSTER_PRESSURE_MEASUREMENT;
            break;
        case DIOT_MATTER_GAS_RESISTANCE_SENSOR:
            expected_cluster = DIOT_MATTER_CLUSTER_GAS_RESISTANCE;
            break;
        default:
            return ESP_ERR_NOT_SUPPORTED;
        }
        if (configs[index].cluster_id != expected_cluster ||
            configs[index].attribute_id != DIOT_MATTER_ATTRIBUTE_MEASURED_VALUE) {
            return ESP_ERR_INVALID_ARG;
        }
    }
    return ESP_OK;
}

static endpoint_t *create_gas_endpoint(node_t *node)
{
    endpoint_t *gas_ep = endpoint::create(node, ENDPOINT_FLAG_NONE, nullptr);
    if (gas_ep == nullptr) {
        return nullptr;
    }
    cluster::descriptor::config_t descriptor_config;
    if (endpoint::add_device_type(gas_ep, DIOT_MATTER_GAS_RESISTANCE_SENSOR, 1) != ESP_OK ||
        cluster::descriptor::create(gas_ep, &descriptor_config, CLUSTER_FLAG_SERVER) == nullptr) {
        return nullptr;
    }
    cluster_t *gas_cluster = cluster::create(gas_ep, DIOT_MATTER_CLUSTER_GAS_RESISTANCE,
                                             CLUSTER_FLAG_SERVER);
    if (gas_cluster == nullptr ||
        attribute::create(gas_cluster, 0xFFFC, ATTRIBUTE_FLAG_NONE, esp_matter_bitmap32(0)) == nullptr ||
        attribute::create(gas_cluster, 0xFFFD, ATTRIBUTE_FLAG_NONE, esp_matter_uint16(1)) == nullptr ||
        attribute::create(gas_cluster, DIOT_MATTER_ATTRIBUTE_MEASURED_VALUE,
                          ATTRIBUTE_FLAG_NONE, esp_matter_float(0.0f)) == nullptr) {
        return nullptr;
    }
    return gas_ep;
}

esp_err_t create_sensor_device(node_t *node, const diot_matter_endpoint_config_t *configs,
                               size_t count, uint16_t *endpoint_ids)
{
    if (node == nullptr || endpoint_ids == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t err = validate_sensor_model(configs, count);
    if (err != ESP_OK) {
        return err;
    }
    for (size_t index = 0; index < count; ++index) {
        endpoint_t *sensor_ep = nullptr;
        switch (configs[index].device_type_id) {
        case DIOT_MATTER_TEMPERATURE_SENSOR: {
            temperature_sensor::config_t config;
            sensor_ep = temperature_sensor::create(node, &config, ENDPOINT_FLAG_NONE, nullptr);
            break;
        }
        case DIOT_MATTER_HUMIDITY_SENSOR: {
            humidity_sensor::config_t config;
            sensor_ep = humidity_sensor::create(node, &config, ENDPOINT_FLAG_NONE, nullptr);
            break;
        }
        case DIOT_MATTER_PRESSURE_SENSOR: {
            pressure_sensor::config_t config;
            sensor_ep = pressure_sensor::create(node, &config, ENDPOINT_FLAG_NONE, nullptr);
            break;
        }
        case DIOT_MATTER_GAS_RESISTANCE_SENSOR:
            sensor_ep = create_gas_endpoint(node);
            break;
        }
        if (sensor_ep == nullptr) {
            return ESP_FAIL;
        }
        endpoint_ids[index] = endpoint::get_id(sensor_ep);
        s_endpoint_ids[index] = endpoint_ids[index];
        s_configs[index] = configs[index];
        ESP_LOGI(TAG, "Model: ep=%u device=0x%08lx cluster=0x%08lx attr=0x%08lx",
                 endpoint_ids[index], (unsigned long)configs[index].device_type_id,
                 (unsigned long)configs[index].cluster_id, (unsigned long)configs[index].attribute_id);
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    secondary_network_interface::config_t secondary_network_interface_config;
    endpoint_t *secondary_ep = endpoint::secondary_network_interface::create(
        node, &secondary_network_interface_config, ENDPOINT_FLAG_NONE, nullptr);
    if (secondary_ep == nullptr) {
        return ESP_FAIL;
    }
#endif

    s_endpoint_count = count;
    return ESP_OK;
}

esp_err_t identification_cb(identification::callback_type_t type, uint16_t endpoint_id,
                            uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %u endpoint: %u effect: %u variant: %u",
             type, endpoint_id, effect_id, effect_variant);
    return ESP_OK;
}

esp_err_t attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id,
                              uint32_t cluster_id, uint32_t attribute_id,
                              esp_matter_attr_val_t *val, void *priv_data)
{
    return ESP_OK;
}

}

static bool has_route(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id)
{
    for (size_t index = 0; index < s_endpoint_count; ++index) {
        if (s_endpoint_ids[index] == endpoint_id && s_configs[index].cluster_id == cluster_id &&
            s_configs[index].attribute_id == attribute_id) {
            return true;
        }
    }
    return false;
}

static esp_err_t report_result(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id,
                               CHIP_ERROR err)
{
    if (err != CHIP_NO_ERROR) {
        ESP_LOGW(TAG, "Endpoint %u update failed: %" CHIP_ERROR_FORMAT, endpoint_id, err.Format());
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Local attribute updated: ep=%u cluster=0x%08lx attr=0x%08lx",
             endpoint_id, (unsigned long)cluster_id, (unsigned long)attribute_id);
    return ESP_OK;
}

extern "C" esp_err_t diot_matter_set_i16(uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, int16_t value)
{
    if (!esp_matter::is_started()) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!has_route(endpoint_id, cluster_id, attribute_id) ||
        (cluster_id != TemperatureMeasurement::Id && cluster_id != PressureMeasurement::Id)) {
        return ESP_ERR_INVALID_ARG;
    }
    esp_matter::lock::ScopedChipStackLock lock(portMAX_DELAY);
    auto *iface = get_server_cluster(endpoint_id, cluster_id);
    if (iface == nullptr) {
        return ESP_ERR_NOT_FOUND;
    }
    CHIP_ERROR err = cluster_id == TemperatureMeasurement::Id
        ? static_cast<TemperatureMeasurementCluster *>(iface)->SetMeasuredValue(
              chip::app::DataModel::MakeNullable(value))
        : static_cast<PressureMeasurementCluster *>(iface)->SetMeasuredValue(
              chip::app::DataModel::MakeNullable(value));
    return report_result(endpoint_id, cluster_id, attribute_id, err);
}

extern "C" esp_err_t diot_matter_set_u16(uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, uint16_t value)
{
    if (!esp_matter::is_started()) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!has_route(endpoint_id, cluster_id, attribute_id) ||
        cluster_id != RelativeHumidityMeasurement::Id || value > 10000) {
        return ESP_ERR_INVALID_ARG;
    }
    esp_matter::lock::ScopedChipStackLock lock(portMAX_DELAY);
    auto *iface = get_server_cluster(endpoint_id, cluster_id);
    if (iface == nullptr) {
        return ESP_ERR_NOT_FOUND;
    }
    CHIP_ERROR err = static_cast<RelativeHumidityMeasurementCluster *>(iface)->SetMeasuredValue(
        chip::app::DataModel::MakeNullable(value));
    return report_result(endpoint_id, cluster_id, attribute_id, err);
}

extern "C" esp_err_t diot_matter_set_float(uint16_t endpoint_id, uint32_t cluster_id,
                                           uint32_t attribute_id, float value)
{
    if (!esp_matter::is_started()) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!has_route(endpoint_id, cluster_id, attribute_id) ||
        cluster_id != DIOT_MATTER_CLUSTER_GAS_RESISTANCE || !std::isfinite(value) || value < 0) {
        return ESP_ERR_INVALID_ARG;
    }
    esp_matter::lock::ScopedChipStackLock lock(portMAX_DELAY);
    esp_matter_attr_val_t attr_value = esp_matter_float(value);
    esp_err_t err = attribute::update(endpoint_id, cluster_id, attribute_id, &attr_value);
    if (err != ESP_OK) {
        return err;
    }
    ESP_LOGI(TAG, "Local gas attribute updated: ep=%u cluster=0x%08lx attr=0x%08lx ohm=%.2f",
             endpoint_id, (unsigned long)cluster_id, (unsigned long)attribute_id, (double)value);
    return ESP_OK;
}