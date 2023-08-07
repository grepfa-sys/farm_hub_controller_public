const char* TAG = "GrepfaConnector";

#include <lwip/netdb.h>
#include <esp_netif_sntp.h>
#include <protocomm_security.h>
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_softap.h>
#include "grepfaNetwork.h"
#include <ArduinoJson.h>


#define CONNECTED BIT0
#define FAIL BIT1

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

GrepfaConnector::GrepfaConnector() {
    get_device_service_name(service_name, sizeof(service_name));
    esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &wifi_prov_handler, this);
    esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, &protocomm_event_handler, this);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, this);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, this);
    esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &ethernet_event_handler, this);

    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // TODO:: ETH Config

    wifi_prov_mgr_config_t  config = {
            .scheme = wifi_prov_scheme_softap,
            .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };

    wifi_prov_mgr_init(config);
}

esp_err_t GrepfaConnector::resetProvisioning() {
    return wifi_prov_mgr_reset_provisioning();
}

esp_err_t GrepfaConnector::doProvisioning() {
    wifi_prov_mgr_is_provisioned(&provisioned);

    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning");
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
        wifi_prov_security1_params_t *sec_params = pop;

        // ethernet, static ip/dns address configuration
        wifi_prov_mgr_endpoint_create("advanced");

        wifi_prov_mgr_start_provisioning(security, (const void *) sec_params, service_name, service_key);

        wifi_prov_mgr_endpoint_register("advanced", advanced_ip_prov_data_handler, this);
    } else {
        ESP_LOGI(TAG, "Already provisioned");
        wifi_prov_mgr_deinit();

        // TODO:: Do connect
    }

    return 0;
}

void GrepfaConnector::wifi_prov_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    auto id = (wifi_prov_cb_event_t) event_id;
    switch (id) {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "Provisioning start");
            break;
        case WIFI_PROV_CRED_RECV:
        {
            auto *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "Received Wi-Fi credentials"
                          "\n\tSSID     : %s\n\tPassword : %s",
                     (const char *) wifi_sta_cfg->ssid,
                     (const char *) wifi_sta_cfg->password);
        }
            break;
        case WIFI_PROV_CRED_FAIL:
        {
            auto *reason = (wifi_prov_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                          "\n\tPlease reset to factory and retry provisioning",
                     (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
                     "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
            break;
        }
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "Provisioning successful");
            break;
        case WIFI_PROV_END:
            wifi_prov_mgr_deinit();
            break;
        default:
            break;
    }
}


void GrepfaConnector::wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    auto id = (wifi_event_t) event_id;

    switch (id) {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
            esp_wifi_connect();
            break;
            case WIFI_EVENT_AP_STACONNECTED:
                ESP_LOGI(TAG, "SoftAP transport: Connected!");
                break;
            case WIFI_EVENT_AP_STADISCONNECTED:
                ESP_LOGI(TAG, "SoftAP transport: Disconnected!");
                break;
        default:
            break;
    }
}

void GrepfaConnector::ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    auto* it = (GrepfaConnector*) arg;
    auto* event = (ip_event_got_ip_t*) event_data;
    auto id = (ip_event_t) event_id;
    ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(it->connector_event_group, CONNECTED);
}

esp_err_t GrepfaConnector::advanced_ip_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                                         uint8_t **outbuf, ssize_t *outlen, void *priv_data) {
    if (inbuf) {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);;
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return 0;
}

void
GrepfaConnector::protocomm_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    auto id = (protocomm_security_session_event_t) event_id;
    switch (id) {

        case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
            ESP_LOGI(TAG, "Secured session established!");
            break;
        case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
            ESP_LOGE(TAG, "Received invalid security parameters for establishing secure session!");
            break;
        case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
            ESP_LOGE(TAG, "Received incorrect username and/or PoP for establishing secure session!");
            break;
    }
}

void
GrepfaConnector::ethernet_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {

}


