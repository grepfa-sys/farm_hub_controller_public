const static char* TAG = "mqtt";
#include "grepfaMqtt.h"

int GrepfaMqtt::Connect(grepfa_mqtt_config_t *config) {
    ESP_LOGI(TAG, "start connect mqtt server");

    option = config;

    esp_mqtt_client_config_t cfg = {
            .broker = {
                    .address = {
                            .uri = config->endpoint
                    },
                    .verification = {
                            .certificate = GrepfaCerts::RootCA()
                    }
            },
            .credentials = {
                    .authentication = {
                            .certificate = GrepfaCerts::ClientCRT(),
#ifndef CONFIG_ESP_SECURE_CERT_DS_PERIPHERAL
                            .key = GrepfaCerts::ClientKEY(),
#else
                            .key = nullptr,
                            .ds_data = GrepfaCerts::dsContext(),
#endif
                    }
            }
    };


    client = esp_mqtt_client_init(&cfg);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DATA, mqtt_data_handler, this);
    auto err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "start mqtt fail");
        return err;
    }

    err = topicSubscribe();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mqtt subscribe fail - 0x%x", err);
        return err;
    }

    Report(meta, "last_connected", time(nullptr));

    return err;
}

esp_err_t GrepfaMqtt::topicSubscribe() {
    sprintf(topics[req], SHADOW_REQUEST_TOPIC, option->device_name);
    sprintf(topics[resp], SHADOW_RESPONSE_TOPIC, option->device_name);
    sprintf(topics[meta], SHADOW_METADATA_TOPIC, option->device_name);
    sprintf(topics[channels_info], SHADOW_CHANNEL_TYPES_TOPIC, option->device_name);


    int mid;
    mid = esp_mqtt_client_subscribe(client, topics[req], DEFAULT_QOS);
    if (mid == -1) {
        ESP_LOGI(TAG, "request topic subscribe fail");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "subscribed topic: %s", topics[req]);

    return ESP_OK;
}

esp_err_t GrepfaMqtt::Send(topic_type_t type, JsonVariantConst doc) {
    char *buf = static_cast<char *>(malloc(2048));
    serializeJson(doc, buf, 2048);
    int mid = esp_mqtt_client_publish(client, topics[type], buf, 0, DEFAULT_QOS, 0);
    ESP_LOGI(TAG, "payload %s", buf);
    free(buf);
    if(mid == -1) {
        ESP_LOGE(TAG, "fail mqtt publish\ntopic - %s\npayload - %s", topics[type], buf);
    }

    return 0;
}

void GrepfaMqtt::mqtt_data_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    auto it = (GrepfaMqtt*) handler_args;
    auto event = static_cast<esp_mqtt_event_handle_t>(event_data);
    esp_mqtt_client_handle_t client = event->client;

    if (!strcmp(event->topic, it->topics[req])) {
        it->mqtt_request_handler(event->topic, event->data);
    }
}

void GrepfaMqtt::mqtt_request_handler(const char *topic, const char *data) {
    if (modbus_handler == nullptr) {
        ESP_LOGE(TAG, "modbus handler not set");
        return;
    }

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, data);

    modbus_handler(doc, GrepfaModBusMasterClass);
}

void GrepfaMqtt::SetMbHandler(void (*handler)(JsonVariantConst, void* p), void* p) {
    modbus_handler = handler;
    GrepfaModBusMasterClass = p;
}

template<class T>
esp_err_t GrepfaMqtt::Reports(topic_type_t type, char **keys, T *values, int num) {
    DynamicJsonDocument doc(512 + num * 100);
    auto temp = doc["state"]["reported"];

    for (int i = 0; i < num; ++i) {
        temp[keys[i]] = values[i];
    }

    char buf[512 + num * 100];
    serializeJson(doc, buf, 512 + num * 100);

    ESP_LOGI(TAG, "payload - %s", buf);

    int mid = esp_mqtt_client_publish(client, topics[type], buf, 0, DEFAULT_QOS, 0);
    if(mid == -1) {
        ESP_LOGE(TAG, "fail mqtt publish\ntopic - %s\npayload - %s", topics[type], buf);
    }
    return 0;
}

template<class T>
esp_err_t GrepfaMqtt::Report(topic_type_t type, const char *key, T value) {
    DynamicJsonDocument doc(512);
    auto temp = doc["state"];
    temp["reported"][key] = value;
//    temp["desired"][key] = value;


    char buf[512];
    serializeJson(doc, buf, 512);

    ESP_LOGI(TAG, "payload - %s", buf);

    int mid = esp_mqtt_client_publish(client, topics[type], buf, 0, DEFAULT_QOS, 0);
    if(mid == -1) {
        ESP_LOGE(TAG, "fail mqtt publish\ntopic - %s\npayload - %s", topics[type], buf);
    }
    return 0;
}
