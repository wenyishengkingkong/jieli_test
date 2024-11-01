// binary representation
// attribute size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)

#ifndef _LE_TUYA_DATA_H
#define _LE_TUYA_DATA_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "matter_os_adapter.h"

static const uint8_t profile_data[] = {
    //////////////////////////////////////////////////////
    //
    // 0x0001 PRIMARY_SERVICE  1800
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,

    /* CHARACTERISTIC,  2a00, READ | DYNAMIC, */
    // 0x0002 CHARACTERISTIC 2a00 READ | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE 2a00 READ | DYNAMIC
    0x08, 0x00, 0x02, 0x01, 0x03, 0x00, 0x00, 0x2a,

    /* CHARACTERISTIC,  2a01, READ | DYNAMIC, */
    // 0x0004 CHARACTERISTIC 2a01 READ | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x04, 0x00, 0x03, 0x28, 0x02, 0x05, 0x00, 0x01, 0x2a,
    // 0x0005 VALUE 2a01 READ | DYNAMIC
    0x08, 0x00, 0x02, 0x01, 0x05, 0x00, 0x01, 0x2a,

    //////////////////////////////////////////////////////
    //
    // 0x0006 PRIMARY_SERVICE  1801
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x06, 0x00, 0x00, 0x28, 0x01, 0x18,

    /* CHARACTERISTIC,  2a05, INDICATE, */
    // 0x0007 CHARACTERISTIC 2a05 INDICATE
    0x0d, 0x00, 0x02, 0x00, 0x07, 0x00, 0x03, 0x28, 0x20, 0x08, 0x00, 0x05, 0x2a,
    // 0x0008 VALUE 2a05 INDICATE
    0x08, 0x00, 0x20, 0x00, 0x08, 0x00, 0x05, 0x2a,
    // 0x0009 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x09, 0x00, 0x02, 0x29, 0x00, 0x00,

    //////////////////////////////////////////////////////
    //
    // 0x000a PRIMARY_SERVICE  0xFFF6
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x00, 0x28, 0xf6, 0xff,

    /* CHARACTERISTIC,  a623, WRITE | DYNAMIC, */
    // 0x000b CHARACTERISTIC a623 WRITE | DYNAMIC
    0x1b, 0x00, 0x02, 0x00, 0x0b, 0x00, 0x03, 0x28, 0x08, 0x0c, 0x00, 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18,
    // 0x000c VALUE a623 WRITE | DYNAMIC
    0x16, 0x00, 0x08, 0x01, 0x0c, 0x00, 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18,

    /* CHARACTERISTIC,  fed6, READ | INDICATE | DYNAMIC, */
    // 0x000d CHARACTERISTIC fed6 READ | INDICATE | DYNAMIC
    0x1b, 0x00, 0x02, 0x00, 0x0d, 0x00, 0x03, 0x28, 0x22, 0x0e, 0x00, 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18,
    // 0x000e VALUE fed6 READ | INDICATE | DYNAMIC
    0x16, 0x00, 0x22, 0x01, 0x0e, 0x00, 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18,
    // 0x000f CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x0f, 0x00, 0x02, 0x29, 0x00, 0x00,

    // END
    0x00, 0x00,
};
//
// characteristics <--> handles
//
#define ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_00000001_0000_1001_8001_00805F9B07D0_01_VALUE_HANDLE 0x0006
#define ATT_CHARACTERISTIC_00000002_0000_1001_8001_00805F9B07D0_01_VALUE_HANDLE 0x0008
#define ATT_CHARACTERISTIC_00000002_0000_1001_8001_00805F9B07D0_01_CLIENT_CONFIGURATION_HANDLE 0x0009
#define ATT_CHARACTERISTIC_00000003_0000_1001_8001_00805F9B07D0_01_VALUE_HANDLE 0x000b

void bt_ble_module_init(void);
void bt_ble_adv_enable(u8 enable);
int ble_set_adv_data(unsigned char *data, unsigned int len);
int ble_set_dev_name(char *name, int len);
void ble_app_disconnect(void);


void matter_send_data(const unsigned char *data, const unsigned char len);

#ifdef __cplusplus
}
#endif

#endif

