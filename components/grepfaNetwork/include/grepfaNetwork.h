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
    char ssid[32];
    char password[64];
}grepfa_connect_wifi_t;

typedef struct {
    grepfa_connect_type_t connection_type;              // WiFi, Ethernet
    grepfa_connect_ip_option_t ip_option;               // Dynamic vs Static
    grepfa_connect_static_ip_option_t static_ip_option; // Only use on static
    grepfa_connect_dns_option_t dns_option;             // DNS Option
    grepfa_connect_wifi_t wifi_option;                  // Wi-Fi Option

    int connect_retry_num;
}grepfa_connect_option_t;


class GrepfaNetworkConnector {
private:
    grepfa_connect_option_t option = {};
    EventGroupHandle_t connector_event_group = nullptr;

    esp_netif_t * ni = nullptr;
    int retry_num = 0;

    esp_err_t commonInit();
    esp_err_t wifiInit();
    esp_err_t ethInit();

    esp_err_t setStatic();
    esp_err_t setDNS();

    static void wifi_ev_handler (void* arg, esp_event_base_t event_base,
                                 int32_t event_id, void* event_data);

public:
    esp_err_t Connect(grepfa_connect_option_t* opt);
    esp_err_t ReConnect(grepfa_connect_option_t* opt);
    esp_err_t DisConnect();
};