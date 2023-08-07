#include <freertos/FreeRTOS.h>

#include <lwip/ip4_addr.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <stdio.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_eth.h>

typedef enum {
    WIFI,
    ETHERNET
}grepfa_connect_type_t;

typedef enum {
    DYNAMIC_IP,             // Use DHCP Client
    DYNAMIC_IP_STATIC_DNS,  // Static DNS
    STATIC_IP               // Static IP, DNS
}grepfa_connect_ip_option_t;

typedef struct {
    uint32_t ip_address;
    uint32_t gateway_address;
    uint32_t netmask_address;   // Subnet mask
}grepfa_connect_static_ip_option_t;

typedef struct {
    uint32_t main_dns_address;
    uint32_t backup_dns_address;
}grepfa_connect_dns_option_t;


typedef struct {
    const char * sntp_server1;
    const char * sntp_server2;

}grepfa_sntp_config_t;

typedef struct {
    grepfa_connect_type_t connection_type;              // WiFi, Ethernet
    grepfa_connect_ip_option_t ip_option;               // Dynamic vs Static
    grepfa_connect_static_ip_option_t static_ip_option; // Only use on static
    grepfa_connect_dns_option_t dns_option;             // DNS Option

    grepfa_sntp_config_t sntp_config;

    int connect_retry_num;
}grepfa_connect_option_t;


class GrepfaConnector {
private:
    bool provisioned = false;
    char service_name[12];

    // TODO:: Read from nvs
    const char* pop = "avcd1234";
    const char* username = "grepfa";
    const char* service_key = "grepfa1234";

    EventGroupHandle_t connector_event_group;

    static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data);
    static void ethernet_event_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data);
    static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data);
    static void protocomm_event_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data);
    static void wifi_prov_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data);


    static esp_err_t advanced_ip_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                       uint8_t **outbuf, ssize_t *outlen, void *priv_data);

public:
    GrepfaConnector();
    esp_err_t doProvisioning();
    esp_err_t resetProvisioning();
};