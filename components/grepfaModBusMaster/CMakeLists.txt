idf_component_register(SRCS "grepfaModBusMaster.cpp"
                    INCLUDE_DIRS "include"
        PRIV_REQUIRES esp-modbus grepfaMqtt mqtt arduinojson esp_secure_cert_mgr grepfaCertification
        )
