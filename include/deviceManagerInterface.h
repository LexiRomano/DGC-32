#ifndef __DEVICE_MANAGER_INTERFACE_H__
#define __DEVICE_MANAGER_INTERFACE_H__

#include <threads.h>
#include <stdint.h>

typedef enum
{
    dts_continue,
    dts_write,
    dts_kill
} deviceThreadSemaphore_e;

typedef struct
{
    mtx_t                   *mutex;
    cnd_t                   *wakeCondition;
    deviceThreadSemaphore_e *semaphore;
    uint8_t                  managerId;
    void                    *initParam;
} threadArg_t;

#endif //__DEVICE_MANAGER_INTERFACE_H__