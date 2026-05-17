#ifndef __MOTHERBOARD_H__
#define __MOTHERBOARD_H__

#include <threads.h>
#include <termios.h>
#include <unistd.h>

#include "dgc32.h"
#include "deviceManagerInterface.h"

// Add device managers here:
#include "deviceManagers/dummyDeviceManager.h"
#include "deviceManagers/peripherals.h"
#include "deviceManagers/storage.h"

#define NUM_DEVICE_MANAGERS 5
#define MAX_NUM_DEVICES 32

typedef struct mb_device_t
{
    uint8_t             deviceId;
    uint8_t             managerId;
    deviceTypes_e       deviceType;
    uint32_t            startLocation;
    uint16_t            size;
    struct mb_device_t *nextInMem;
} mb_device_t;

bool mb_init(externalFileInfo_t *externalFileInfo, glfwInfo_t *glfw, memTransFP_t read, memTransFP_t write, interruptFP_t interrupt);
void mb_writeToDeviceData(uint32_t address, uint8_t numBytes, void *data);
bool mb_powerState();
void mb_teardown();

#endif //__MOTHERBOARD_H__