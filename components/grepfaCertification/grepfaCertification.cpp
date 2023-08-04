#include <stdio.h>
#include "grepfaCertification.h"


const char *GrepfaCerts::RootCA() {
    char *addr;
    uint32_t len;
    esp_secure_cert_get_ca_cert(&addr, &len);

    return addr;
}

const char *GrepfaCerts::ClientCRT() {
    char *addr;
    uint32_t len;
    esp_secure_cert_get_device_cert(&addr, &len);

    return addr;
}

#ifndef CONFIG_ESP_SECURE_CERT_DS_PERIPHERAL
const char *GrepfaCerts::ClientKEY() {
    char *addr;
    uint32_t len;
    esp_secure_cert_get_priv_key(&addr, &len);
    return addr;
}
#else
esp_ds_data_ctx_t *GrepfaCerts::dsContext() {
    return esp_secure_cert_get_ds_ctx();
}
#endif