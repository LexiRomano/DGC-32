#ifndef __DEVICE_MANAGER_INTERFACE_H__
#define __DEVICE_MANAGER_INTERFACE_H__

#include <threads.h>
#include <stdint.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>

#define MAX_INITIAL_STORAGE_DEVICES 2
#define NEW_DEVICE_REQUEST_FAILED 255
#define DEVICE_TABLE_END_MARKER 0xFF
#define DEVICE_TALBE_EMPTY_MARKER 0

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
    dts_handleReadWrite,
    dts_kill
} deviceThreadSemaphoreWakeReason_e;

typedef struct
{
    deviceThreadSemaphoreWakeReason_e wakeReason;
    uint8_t                           deviceId;
} deviceThreadSemaphore_t;

typedef struct
{
    char   *romFileName;
    char   *storageFileNames[MAX_INITIAL_STORAGE_DEVICES];
} externalFileInfo_t;

typedef struct
{
    GLFWwindow *window;
} glfwInfo_t;

typedef struct
{
    mtx_t                   *mutex;
    cnd_t                   *wakeCondition;
    deviceThreadSemaphore_t *semaphore;
    uint8_t                  managerId;
    externalFileInfo_t       externalFileInfo;
    glfwInfo_t               glfwInfo;
} threadArg_t;

typedef void (*handleReadFP_t)(uint8_t, uint16_t, uint8_t);
typedef void (*handleWriteFP_t)(uint8_t, uint16_t, uint8_t, void*);
typedef void (*handleTermInFP_t)(uint8_t);

// Motherboard functions
uint8_t dmi_requestNewDevice(uint8_t managerId, uint16_t size);
void dmi_removeDevice(uint8_t deviceId);
bool dmi_readDeviceData(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *buf);
bool dmi_writeDeviceData(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *buf);
bool dmi_enqueueInterrupt(uint8_t deviceId, interruptTypes_e interruptType, uint8_t interruptParameter);
bool dmi_writeToMemory(uint32_t address, uint8_t numBytes, void *data);
bool dmi_readFromMemory(uint32_t address, uint8_t numBytes, void *data);
bool dmi_bindHandleRead(uint8_t managerId, handleReadFP_t handleReadFunction);
bool dmi_bindHandleWrite(uint8_t managerId, handleWriteFP_t handleWriteFunction);
bool dmi_bindHandleTermIn(uint8_t managerId, handleTermInFP_t handleTermIn);

// Functions to implement:
//     void xx_handleRead(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes);
//     // This runs on the processor's thread, must not use any system calls or wait for a mutex
//
//     void xx_handleWrite(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *data);
//     // This runs on the processor's thread, must not use any system calls or wait for a mutex
//
//     // For peripheral only:
//     void xx_handleTermIn(uint8_t input);
//     // This runs on the motherboard's thread, no restrictions on performance
//
// Then call:
//     mb_bindHandleRead  (myThreadData.managerId, xx_handleRead);
//     mb_bindHandleWrite (myThreadData.managerId, xx_handleWrite);
//     mb_bindHandleTermIn(myThreadData.managerId, xx_handleTermIn);
//
//

#endif //__DEVICE_MANAGER_INTERFACE_H__