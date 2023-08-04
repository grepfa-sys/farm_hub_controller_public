#pragma once
#include "sdkconfig.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <vector>

#include "string.h"
#include "esp_log.h"
#include "mbcontroller.h"

#include <grepfaMqtt.h>

#define MB_PORT_NUM 1
#define MB_BAUDRATE 9600
#define DEFAULT_RETRY 5

#define MB_TRUE 0xFF00
#define MB_FALSE 0x0000

enum{
    COIL_MOTOR_ON_OFF,
    COIL_MOTOR_DIRECTION,
};

enum{
    INPUT_REG_TYPE,
    INPUT_REG_VERSION,
    INPUT_REG_SENSOR_VALUE_1,
    INPUT_REG_SENSOR_VALUE_2,
};

typedef enum {
    TYPE_UNKNOWN = -1,
    TYPE_NONE = 0,
    TYPE_MOTOR,
    TYPE_DC_RELAY,
    TYPE_AC_RELAY,
    TYPE_TEM_SENSOR
} mb_device_type_t;


enum {
    MB_ADDR_START = 10,
    MB_MOTOR_ADDR = 11,
    MB_MOTOR_ADDR_LAST = 110,
    MB_SENSOR_ADDR = 111,
    MB_SENSOR_ADDR_LAST = 130,
    MB_ADDR_MAX
};

#define MOTOR_PREFIX "motor"
#define SENSOR_PREFIX "sensor"

class GrepfaModBusMaster {
private:
    GrepfaMqtt* mqtt;
    SemaphoreHandle_t sem;
    int num = MB_MOTOR_ADDR_LAST - MB_MOTOR_ADDR + 1 + MB_SENSOR_ADDR_LAST - MB_SENSOR_ADDR + 1;
    int types[MB_MOTOR_ADDR_LAST - MB_MOTOR_ADDR + 1 + MB_SENSOR_ADDR_LAST - MB_SENSOR_ADDR + 1];
    int memMap[MB_MOTOR_ADDR_LAST - MB_MOTOR_ADDR + 1 + MB_SENSOR_ADDR_LAST - MB_SENSOR_ADDR + 1];

    static void scannerTask(void* param);
    static void mbHandler(JsonVariantConst doc, void* p);
    void memberMbHandler(JsonVariantConst doc);

    int addrParser(uint8_t addr, char* buf);
    int idxParser(int idx);
    uint8_t nameParser(const char* name);

    void* handler;
public:
    esp_err_t init(GrepfaMqtt* client);

    int motorOn(uint8_t addr);
    int motorOff(uint8_t addr);
    int motorOpen(uint8_t addr);
    int motorClose(uint8_t addr);

    int getMotorIsOn(uint8_t addr, bool* status);
    int getMotorDirection(uint8_t addr, bool* status);

    int getDeviceType(uint8_t addr, mb_device_type_t * type);
    int getDeviceVersion(uint8_t addr, uint16_t* version);
};