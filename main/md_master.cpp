static const char * TAG = "main";

#include <stdio.h>
#include <grepfaModBusMaster.h>
#include <nvs_flash.h>
#include "esp_event.h"
#include "esp_netif.h"
#include <grepfaNetwork.h>
#include <grepfaMqtt.h>

extern "C" void app_main(void)
{


    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    GrepfaNetworkConnector conn;
    grepfa_connect_option_t conn_opt = {
            .connection_type = WIFI,
            .ip_option = DYNAMIC_IP_STATIC_DNS,
            .dns_option = {
                    .main_dns_address = ipaddr_addr("8.8.8.8"),
                    .backup_dns_address = ipaddr_addr("1.1.1.1"),
            },
            .wifi_option = {
                    .ssid = "sys2.4G",
                    .password = "shin0114"
            },
            .connect_retry_num = 5
    };

    conn.Connect(&conn_opt);

    GrepfaMqtt mqtt;
    grepfa_mqtt_config_t mqtt_opt = {
            .endpoint = "mqtts://iot.grepfa.net",
            .device_name = "test_dev_2"
    };

    mqtt.Connect(&mqtt_opt);

    GrepfaModBusMaster x{};
    x.init(&mqtt);

    while (1) {
        vTaskDelay(portMAX_DELAY);
    }

}
