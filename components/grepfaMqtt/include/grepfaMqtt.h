#pragma once

#include <mqtt_client.h>
#include <esp_log.h>
#include <grepfaCertification.h>
#include <ArduinoJson.h>


#define SHADOW_REQUEST_TOPIC "$aws/things/%s/shadow/name/channels/update/delta"
#define SHADOW_RESPONSE_TOPIC "$aws/things/%s/shadow/name/channels/update"

#define SHADOW_METADATA_TOPIC "$aws/things/%s/shadow/name/metadata/update"
#define SHADOW_CHANNEL_TYPES_TOPIC "$aws/things/%s/shadow/name/channels_info/update"

#define DEFAULT_QOS 1

typedef struct {
    const char* endpoint;
    const char* device_name;
} grepfa_mqtt_config_t;

typedef enum {
    req,
    resp,
    meta,
    channels_info,
    topic_type_max
} topic_type_t;

class GrepfaMqtt {
private:
    static void mqtt_data_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

    void mqtt_request_handler(const char* topic, const char* data);

    void (*modbus_handler)(JsonVariantConst doc, void* p);
    void* GrepfaModBusMasterClass;

    char topics[topic_type_max][256];

    esp_mqtt_client_handle_t client;
    grepfa_mqtt_config_t * option;
    esp_err_t topicSubscribe();

public:
    void SetMbHandler(void (*handler)(JsonVariantConst doc, void* GrepfaModBusMasterClass), void* GrepfaModBusMasterClass);

    int Connect(grepfa_mqtt_config_t* config);

    template<class T>
    esp_err_t Report(topic_type_t type, const char* key, T value);
    esp_err_t Send(topic_type_t type, JsonVariantConst doc);

    template<class T>
    esp_err_t Reports(topic_type_t type, char **keys, T* values, int num);

};