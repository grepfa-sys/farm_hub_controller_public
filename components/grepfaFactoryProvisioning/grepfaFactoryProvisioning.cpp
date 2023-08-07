const static char* TAG = "factory_provisioning";

#include <esp_log.h>
#include "grepfaFactoryProvisioning.h"

std::unique_ptr<nvs::NVSHandle> GrepfaFactoryProvisioning::handle = nullptr;

char* GrepfaFactoryProvisioning::broker_endpoint = nullptr;
char* GrepfaFactoryProvisioning::device_id = nullptr;
char* GrepfaFactoryProvisioning::pop = nullptr;
char* GrepfaFactoryProvisioning::username = nullptr;
char* GrepfaFactoryProvisioning::service_key = nullptr;

size_t GrepfaFactoryProvisioning::broker_endpoint_len = 0;
size_t GrepfaFactoryProvisioning::device_id_len = 0;
size_t GrepfaFactoryProvisioning::pop_len = 0;
size_t GrepfaFactoryProvisioning::username_len = 0;
size_t GrepfaFactoryProvisioning::service_key_len = 0;

esp_err_t  GrepfaFactoryProvisioning::Init() {
    esp_err_t err;
    handle = nvs::open_nvs_handle("grepfa", NVS_READWRITE, &err);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "open nvs error - 0x%x", err);
        return err;
    }

    handle->get_item_size(nvs::ItemType::SZ, "broker_ep", broker_endpoint_len);
    handle->get_item_size(nvs::ItemType::SZ, "dev_id", device_id_len);
    handle->get_item_size(nvs::ItemType::SZ, "pop", pop_len);
    handle->get_item_size(nvs::ItemType::SZ, "username", username_len);
    handle->get_item_size(nvs::ItemType::SZ, "service_key", service_key_len);

    broker_endpoint = static_cast<char *>(malloc(broker_endpoint_len));
    device_id       = static_cast<char *>(malloc(device_id_len));
    pop             = static_cast<char *>(malloc(pop_len));
    username        = static_cast<char *>(malloc(username_len));
    service_key     = static_cast<char *>(malloc(service_key_len));

    handle->get_string("broker_ep", broker_endpoint, broker_endpoint_len);
    handle->get_string("dev_id", device_id, device_id_len);
    handle->get_string("pop", pop, pop_len);
    handle->get_string("username", username, username_len);
    handle->get_string("service_key", service_key, service_key_len);

    return ESP_OK;
}

char *GrepfaFactoryProvisioning::getBrokerEndpoint() {
    return broker_endpoint;
}

char *GrepfaFactoryProvisioning::getDeviceId() {
    return device_id;
}

char *GrepfaFactoryProvisioning::getPop() {
    return pop;
}

char *GrepfaFactoryProvisioning::getUsername() {
    return username;
}

char *GrepfaFactoryProvisioning::getServiceKey() {
    return service_key;
}
