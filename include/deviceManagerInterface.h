#ifndef __DEVICE_MANAGER_INTERFACE_H__
#define __DEVICE_MANAGER_INTERFACE_H__

#include <threads.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_INITIAL_STORAGE_DEVICES 2
#define NEW_DEVICE_REQUEST_FAILED 255
#define DEVICE_TABLE_END_MARKER 0xFF

typedef enum
{
    dt_none = 0,
    dt_motherboard,
    dt_storage,
    dt_graphical,
    dt_timer,
    dt_peripheral
} deviceTypes_e;

typedef enum
{
    it_keyPressed,
    it_keyReleased,
    it_peripheralEvent,
    it_motherboardEvent,
    it_timerEvent,
    it_storageEvent,
    it_graphicalEvent
} interruptTypes_e;

typedef enum
{
    dts_continue,
    dts_handleWrite,
    dts_kill
} deviceThreadSemaphore_e;

typedef struct
{
    char   *romFileName;
    char   *storageFileNames[MAX_INITIAL_STORAGE_DEVICES];
} externalFileInfo_t;

typedef struct
{
    mtx_t                   *mutex;
    cnd_t                   *wakeCondition;
    deviceThreadSemaphore_e *semaphore;
    uint8_t                  managerId;
    externalFileInfo_t       externalFileInfo;
} threadArg_t;

typedef void (*handleWriteFP_t)(uint8_t, uint16_t, uint8_t, void*);

// Motherboard functions
uint8_t dmi_requestNewDevice(uint8_t managerId, uint16_t size);
void dmi_removeDevice(uint8_t deviceId);
bool dmi_readDeviceData(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *buf);
bool dmi_writeDeviceData(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *buf);
bool dmi_enqueueInterrupt(uint8_t deviceId, interruptTypes_e interruptType, uint8_t interruptParameter);
bool dmi_writeToMemory(uint32_t address, uint8_t numBytes, void *data);
bool dmi_readFromMemory(uint32_t address, uint8_t numBytes, void *data);
bool dmi_bindHandleWrite(uint8_t deviceHandlerId, handleWriteFP_t handleWriteFunction);

// Functions to implement:
//     void xx_handleWrite(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *data);
//
// Then call:
//     mb_bindHandleWrite(myThreadData.managerId, xx_handleWrite);

#endif //__DEVICE_MANAGER_INTERFACE_H__