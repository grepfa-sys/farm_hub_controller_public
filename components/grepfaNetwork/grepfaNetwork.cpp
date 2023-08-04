const char* TAG = "GrepfaConnector";

#include <lwip/netdb.h>
#include "grepfaNetwork.h"

typedef enum {
    CONNECTED = BIT0,
    FAIL = BIT1
} connector_event_type_t;


esp_err_t GrepfaNetworkConnector::Connect(grepfa_connect_option_t *opt) {
    ESP_LOGI(TAG, "start connector");

    memcpy(&option, opt, sizeof(grepfa_connect_option_t));

    esp_err_t err = ESP_OK;

    switch (option.connection_type) {
        case WIFI:
            ESP_LOGI(TAG, "start connect wifi");
            err = wifiInit();
            break;
        case ETHERNET:
            ESP_LOGE(TAG, "ethernet not support");
            break;
        default:
            err = ESP_ERR_INVALID_ARG;
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "err code: 0x%x", err);
    }

    return ESP_OK;
}

esp_err_t GrepfaNetworkConnector::commonInit() {
    esp_err_t err;
    connector_event_group = xEventGroupCreate();
    return 0;
}

esp_err_t GrepfaNetworkConnector::wifiInit() {
    esp_err_t err = commonInit();
    if (err != ESP_OK) {
        return err;
    }

    ni = esp_netif_create_default_wifi_sta();

    // set static address if option set static ip
    if (option.ip_option == STATIC_IP) {
        err = setStatic();
        if(err != ESP_OK) {
            ESP_LOGE(TAG, "configuration static ip error - 0x%x", err);
            return err;
        }
    }

    // set static dns server if option set static dns
    if (option.ip_option == DYNAMIC_IP_STATIC_DNS || option.ip_option == STATIC_IP) {
        err = setDNS();
        if(err != ESP_OK) {
            ESP_LOGE(TAG, "configuration DNS error - 0x%x", err);
            return err;
        }
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "wifi init fail - 0x%x", err);
        return err;
    }

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;


    err = esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_ev_handler,
                                                        this,
                                                        &instance_any_id);
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, " - 0x%x", err);
        return err;
    }

    err = esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_ev_handler,
                                                        this,
                                                        &instance_got_ip);
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, " - 0x%x", err);
        return err;
    }



    wifi_config_t wifi_config = {
            .sta = {
                .scan_method = WIFI_ALL_CHANNEL_SCAN
            }
    };

    strncpy(reinterpret_cast<char *>(wifi_config.sta.ssid), option.wifi_option.ssid, 32);
    strncpy(reinterpret_cast<char *>(wifi_config.sta.password), option.wifi_option.password, 64);

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "wifi set mode fail - 0x%x", err);
        return err;
    }

    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "wifi init fail - 0x%x", err);
        return err;
    }

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "wifi set config fail - 0x%x", err);
        return err;
    }

    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, " - 0x%x", err);
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "wifi start fail - 0x%x", err);
        return err;
    }


    ESP_LOGI(TAG, "wifi initialization done");
    EventBits_t bits = xEventGroupWaitBits(connector_event_group,
                                           CONNECTED | FAIL,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
 * happened. */
    if (bits & CONNECTED) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 option.wifi_option.ssid, option.wifi_option.password);
    } else if (bits & FAIL) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 option.wifi_option.ssid, option.wifi_option.password);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }
    return ESP_OK;
}

void
GrepfaNetworkConnector::wifi_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    auto it = (GrepfaNetworkConnector*) arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (it->retry_num < it->option.connect_retry_num) {
            esp_wifi_connect();
            it->retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(it->connector_event_group, FAIL);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        auto* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        it->retry_num = 0;
        xEventGroupSetBits(it->connector_event_group, CONNECTED);
    }
}

esp_err_t GrepfaNetworkConnector::setDNS() {
    esp_err_t err = ESP_OK;
    if (option.dns_option.main_dns_address && (option.dns_option.main_dns_address != IPADDR_NONE)) {
        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4.addr = option.dns_option.main_dns_address;
        dns.ip.type = IPADDR_TYPE_V4;
        err = esp_netif_set_dns_info(ni, ESP_NETIF_DNS_MAIN, &dns);
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "configuration main dns fail");
        return err;
    }

    if (option.dns_option.backup_dns_address && (option.dns_option.backup_dns_address != IPADDR_NONE)) {

        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4.addr = option.dns_option.backup_dns_address;
        dns.ip.type = IPADDR_TYPE_V4;
        err = esp_netif_set_dns_info(ni, ESP_NETIF_DNS_BACKUP, &dns);
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "configuration main dns fail");
        return err;
    }

    return ESP_OK;
}

esp_err_t GrepfaNetworkConnector::setStatic() {
    esp_err_t err = esp_netif_dhcpc_stop(ni);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "configuration static ip error - 0x%x", err);
        return err;
    }

    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));

    ip.ip.addr = option.static_ip_option.ip_address;
    ip.netmask.addr = option.static_ip_option.netmask_address;
    ip.gw.addr = option.static_ip_option.gateway_address;

    err = esp_netif_set_ip_info(ni, &ip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "configuration static ip error - 0x%x", err);
        return err;
    }
    return ESP_OK;
}
