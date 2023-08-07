idf_component_register(SRCS "grepfaNetwork.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_wifi esp_netif esp_eth wifi_provisioning arduinojson
        )
