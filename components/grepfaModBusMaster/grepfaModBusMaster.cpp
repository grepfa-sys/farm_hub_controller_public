const static char* TAG = "modbus";

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "grepfaModBusMaster.h"


esp_err_t GrepfaModBusMaster::init(GrepfaMqtt* client) {
    mqtt = client;
    client->SetMbHandler(mbHandler, this);

    int idx = 0;
    for (int i = MB_MOTOR_ADDR; i <= MB_MOTOR_ADDR_LAST; ++i) {
        memMap[idx] = i;
        ++idx;
    }
    for (int i = MB_SENSOR_ADDR; i <= MB_SENSOR_ADDR_LAST; ++i) {
        memMap[idx] = i;
        ++idx;
    }

    esp_err_t  err = mbc_master_init(MB_PORT_SERIAL_MASTER, &handler);
    if (handler == NULL || err != ESP_OK) {
        ESP_LOGE(TAG, "mb controller initialization fail.");
        return err;
    }

    uart_set_pin(UART_NUM_1, 35, 36, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    vTaskDelay(5);

    mb_communication_info_t comm_info = {
            .mode = MB_MODE_RTU,
            .port = UART_NUM_1,
            .baudrate = 9600,
            .parity = UART_PARITY_EVEN
    };
    err = mbc_master_setup(&comm_info);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mb configuration fail.");
        return err;
    }

    err = mbc_master_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mb start fail.");
        return err;
    }

    ESP_LOGI(TAG, "start scanner");

    sem = xSemaphoreCreateMutex();
    if (sem == nullptr) {
        ESP_LOGE(TAG, "no mem");
        return ESP_FAIL;
    }

    xTaskCreatePinnedToCore(scannerTask, "scanner", 4096, this, 2, nullptr, 1);

    return ESP_OK;
}



int GrepfaModBusMaster::motorOn(uint8_t addr) {
    mb_param_request_t req {
            .slave_addr = addr,
            .command = 5,
            .reg_start = COIL_MOTOR_ON_OFF,
            .reg_size = 1
    };
    uint16_t data = MB_TRUE;

    for (int i = 0; i < DEFAULT_RETRY; ++i) {
        xSemaphoreTake(sem, portMAX_DELAY);
        esp_err_t err = mbc_master_send_request(&req, (void*)&data);
        if(sem) xSemaphoreGive(sem);
        if (err == ESP_OK) {
            return ESP_OK;
        }
        else if (err == ESP_ERR_TIMEOUT) continue;
        else{
            ESP_LOGE(TAG, "error: 0x%x", err);
            return err;
        }
    }

    ESP_LOGI(TAG, "TIME OUT");

    return ESP_ERR_TIMEOUT;
}

int GrepfaModBusMaster::motorOff(uint8_t addr) {
    mb_param_request_t req {
            .slave_addr = addr,
            .command = 5,
            .reg_start = COIL_MOTOR_ON_OFF,
            .reg_size = 1
    };
    uint16_t data = MB_FALSE;
    for (int i = 0; i < DEFAULT_RETRY; ++i) {
        xSemaphoreTake(sem, portMAX_DELAY);
        esp_err_t err = mbc_master_send_request(&req, (void*)&data);
        if(sem) xSemaphoreGive(sem);
        if (err == ESP_OK) {
            return ESP_OK;
        }
        else if (err == ESP_ERR_TIMEOUT) continue;
        else{
            ESP_LOGE(TAG, "error: 0x%x", err);
            return err;
        }
    }
    return 0;
}

int GrepfaModBusMaster::motorOpen(uint8_t addr) {
    mb_param_request_t req {
            .slave_addr = addr,
            .command = 5,
            .reg_start = COIL_MOTOR_DIRECTION,
            .reg_size = 1
    };
    uint16_t data = MB_TRUE;
    for (int i = 0; i < DEFAULT_RETRY; ++i) {
        xSemaphoreTake(sem, portMAX_DELAY);
        esp_err_t err = mbc_master_send_request(&req, (void*)&data);
        if(sem) xSemaphoreGive(sem);
        if (err == ESP_OK) {
            return ESP_OK;
        }
        else if (err == ESP_ERR_TIMEOUT) continue;
        else{
            ESP_LOGE(TAG, "error: 0x%x", err);
            return err;
        }
    }

    ESP_LOGI(TAG, "TIME OUT");

    return ESP_ERR_TIMEOUT;
}

int GrepfaModBusMaster::motorClose(uint8_t addr) {
    mb_param_request_t req {
            .slave_addr = addr,
            .command = 5,
            .reg_start = COIL_MOTOR_DIRECTION,
            .reg_size = 1
    };
    uint16_t data = MB_FALSE;
    for (int i = 0; i < DEFAULT_RETRY; ++i) {
        xSemaphoreTake(sem, portMAX_DELAY);
        esp_err_t err = mbc_master_send_request(&req, (void*)&data);
        if(sem) xSemaphoreGive(sem);
        if (err == ESP_OK) {
            return ESP_OK;
        }
        else if (err == ESP_ERR_TIMEOUT) continue;
        else{
            ESP_LOGE(TAG, "error: 0x%x", err);
            return err;
        }
    }

    ESP_LOGI(TAG, "TIME OUT");

    return ESP_ERR_TIMEOUT;
}

int GrepfaModBusMaster::getMotorIsOn(uint8_t addr, bool *status) {
    mb_param_request_t req {
            .slave_addr = addr,
            .command = 1,
            .reg_start = COIL_MOTOR_ON_OFF,
            .reg_size = 1
    };
    uint16_t data;
    for (int i = 0; i < DEFAULT_RETRY; ++i) {
        xSemaphoreTake(sem, portMAX_DELAY);
        esp_err_t err = mbc_master_send_request(&req, (void*)&data);
        if(sem) xSemaphoreGive(sem);
        if (err == ESP_OK) {
            return ESP_OK;
        }
        else if (err == ESP_ERR_TIMEOUT) continue;
        else{
            ESP_LOGE(TAG, "error: 0x%x", err);
            return err;
        }
    }

    ESP_LOGI(TAG, "TIME OUT");
    *status = static_cast<mb_device_type_t>(data);
    return ESP_ERR_TIMEOUT;
}

int GrepfaModBusMaster::getMotorDirection(uint8_t addr, bool *status) {
    mb_param_request_t req {
            .slave_addr = addr,
            .command = 1,
            .reg_start = COIL_MOTOR_DIRECTION,
            .reg_size = 1
    };
    bool data;
    for (int i = 0; i < DEFAULT_RETRY; ++i) {
        xSemaphoreTake(sem, portMAX_DELAY);
        esp_err_t err = mbc_master_send_request(&req, (void*)&data);
        if(sem) xSemaphoreGive(sem);
        if (err == ESP_OK) {
            return ESP_OK;
        }
        else if (err == ESP_ERR_TIMEOUT) continue;
        else{
            ESP_LOGE(TAG, "error: 0x%x", err);
            return err;
        }
    }

    ESP_LOGI(TAG, "TIME OUT");
    *status = static_cast<mb_device_type_t>(data);
    return ESP_ERR_TIMEOUT;
}

int GrepfaModBusMaster::getDeviceType(uint8_t addr, mb_device_type_t *type) {
    mb_param_request_t req {
            .slave_addr = addr,
            .command = 4,
            .reg_start = INPUT_REG_TYPE,
            .reg_size = 1
    };
    uint16_t data;
    for (int i = 0; i < DEFAULT_RETRY; ++i) {
        xSemaphoreTake(sem, portMAX_DELAY);
        esp_err_t err = mbc_master_send_request(&req, (void*)&data);
        if(sem) xSemaphoreGive(sem);
        if (err == ESP_OK) {
            return ESP_OK;
        }
        else if (err == ESP_ERR_TIMEOUT) continue;
        else{
            ESP_LOGE(TAG, "error: 0x%x", err);
            return err;
        }
    }

    ESP_LOGI(TAG, "TIME OUT");
    *type = static_cast<mb_device_type_t>(data);
    return ESP_ERR_TIMEOUT;
}

int GrepfaModBusMaster::getDeviceVersion(uint8_t addr, uint16_t* version) {
    mb_param_request_t req {
            .slave_addr = addr,
            .command = 4,
            .reg_start = INPUT_REG_VERSION,
            .reg_size = 1
    };
    uint16_t data;
    for (int i = 0; i < DEFAULT_RETRY; ++i) {
        xSemaphoreTake(sem, portMAX_DELAY);
        esp_err_t err = mbc_master_send_request(&req, (void*)&data);
        if(sem) xSemaphoreGive(sem);
        if (err == ESP_OK) {
            return ESP_OK;
        }
        else if (err == ESP_ERR_TIMEOUT) continue;
        else{
            ESP_LOGE(TAG, "error: 0x%x", err);
            return err;
        }
    }

    ESP_LOGI(TAG, "TIME OUT");
    *version = static_cast<mb_device_type_t>(data);
    return ESP_ERR_TIMEOUT;
}

uint8_t GrepfaModBusMaster::nameParser(const char *name) {
    char* copy = strdup(name);
    char* first = strtok(copy, "-");
    char* second = strtok(nullptr, "-");
    int num = strtol(second, nullptr, 10);

//    ESP_LOGI(TAG, "========= %s, %d", second, num);

    if (!strcmp(first, MOTOR_PREFIX)) {
        int calc = num  + MB_MOTOR_ADDR - 1;
        if (calc > MB_MOTOR_ADDR_LAST) {
            free(copy);
            return 0;
        }
        free(copy);
        return calc;
    } else if(!strcmp(first, SENSOR_PREFIX)) {
        int calc = num + MB_SENSOR_ADDR - 1;
        if (calc > MB_SENSOR_ADDR_LAST) {
            free(copy);
            return 0;
        }
        free(copy);
        return calc;
    }

    free(copy);
    return 0;
}

int GrepfaModBusMaster::addrParser(uint8_t addr, char *buf) {
    if (addr >= MB_MOTOR_ADDR && addr <= MB_MOTOR_ADDR_LAST) {
        sprintf(buf, "motor-%d", addr - MB_MOTOR_ADDR + 1);
        return 0;
    } else if (addr >= MB_SENSOR_ADDR && addr <= MB_SENSOR_ADDR_LAST) {
        sprintf(buf, "sensor-%d", addr - MB_SENSOR_ADDR + 1);
        return 0;
    }
    return -1;
}

void GrepfaModBusMaster::scannerTask(void *param) {
    ESP_LOGI(TAG, "1");
    auto it = (GrepfaModBusMaster*) param;
    mb_param_request_t req{
            .slave_addr = 0,
            .command = 4,
            .reg_start = INPUT_REG_TYPE,
            .reg_size = 1
    };

    while (true)
    {
        ESP_LOGI(TAG, "1");
        memset(it->types, 0, it->num * sizeof(int));

        int idx = 0;
        for (int i = MB_MOTOR_ADDR; i <= MB_MOTOR_ADDR_LAST; ++i) {
            uint16_t data = 0;
            req.slave_addr = i;

            xSemaphoreTake(it->sem, portMAX_DELAY);
            esp_err_t err = mbc_master_send_request(&req, (void *) &data);
            if (it->sem) xSemaphoreGive(it->sem);
            if (err == ESP_OK) {
                it->types[idx] = data;
            }

            ESP_LOGI(TAG, "scan done - %d - %d", i, data);
            ++idx;

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        for (int i = MB_SENSOR_ADDR; i <= MB_SENSOR_ADDR_LAST; ++i) {
            uint16_t data = 0;
            req.slave_addr = i;

            esp_err_t err = mbc_master_send_request(&req, (void *) &data);
            if (err == ESP_OK) {
                it->types[idx] = data;
            }

            ESP_LOGI(TAG, "scan done - %d - %d", i, data);
            ++idx;

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        DynamicJsonDocument doc(3000);
        char key[100];

        for (int i = 0; i < it->num; i++) {
            it->addrParser(it->idxParser(i), key);
            auto t = it->types[i];
            if (t == 0) {
                doc["state"]["reported"][key] = nullptr;
            } else {
                doc["state"]["reported"][key] = t;
            }
        }

        it->mqtt->Send(channels_info, doc);
        ESP_LOGI(TAG, "module status send");
    }
}

int GrepfaModBusMaster::idxParser(int idx) {
    return memMap[idx];
}

void GrepfaModBusMaster::mbHandler(JsonVariantConst doc, void* p) {
    auto it = (GrepfaModBusMaster*) p;
    it->memberMbHandler(doc);
}

void GrepfaModBusMaster::memberMbHandler(JsonVariantConst doc) {
    auto state = doc["state"].as<JsonObjectConst>();
    DynamicJsonDocument rp(1024);

    for(JsonPairConst kv : state) {
        auto key = kv.key().c_str();
        auto value = kv.value().as<int>();
        ESP_LOGI(TAG, "command received %s - %d", key, value);

        auto addr = nameParser(key);

        ESP_LOGI(TAG, "addr %d parsed", addr);

        if (value == 0) {
            ESP_LOGI(TAG, "off command send");
            esp_err_t err = motorOff(addr);
            if (err == ESP_OK) {
                rp["state"]["reported"][key] = 0.0;
            }
        } else {
            esp_err_t err = motorOn(addr);
            if (err == ESP_OK) {
                if (value == 1) {
                    ESP_LOGI(TAG, "open command send");
                    err = motorOpen(addr);
                    if (err == ESP_OK) {
                        rp["state"]["reported"][key] = 1.0;
                    }
                }
                else if(value == -1) {
                    ESP_LOGI(TAG, "close command send");
                    err = motorClose(addr);
                    if (err == ESP_OK) {
                        rp["state"]["reported"][key] = -1.0;
                    }
                }
            }
        }
    }
    mqtt->Send(resp, rp);
}
