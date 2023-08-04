idf_component_register(SRCS "grepfaMqtt.cpp"
                    INCLUDE_DIRS "include"
        PRIV_REQUIRES mqtt grepfaCertification esp_secure_cert_mgr arduinojson
        )
