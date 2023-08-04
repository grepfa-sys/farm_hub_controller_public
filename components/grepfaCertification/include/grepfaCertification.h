#pragma once

#include <esp_secure_cert_read.h>

class GrepfaCerts{
public:
    static const char* RootCA();
    static const char* ClientCRT();
#ifndef CONFIG_ESP_SECURE_CERT_DS_PERIPHERAL
    static const char* ClientKEY();
#else
    static esp_ds_data_ctx_t * dsContext();
#endif
};