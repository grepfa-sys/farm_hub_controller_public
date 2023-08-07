#pragma once

#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"



class GrepfaFactoryProvisioning {
private:
    static std::unique_ptr<nvs::NVSHandle> handle;

    static char* broker_endpoint;
    static char* device_id;
    static char* pop;
    static char* username;
    static char* service_key;

    static size_t broker_endpoint_len;
    static size_t device_id_len;
    static size_t pop_len;
    static size_t username_len;
    static size_t service_key_len;
public:
    static esp_err_t Init();
    static char *getBrokerEndpoint();
    static char *getDeviceId();
    static char *getPop();
    static char *getUsername();
    static char *getServiceKey();
};