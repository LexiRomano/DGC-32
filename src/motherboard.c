#include "motherboard.h"

thrd_t *deviceThreads[NUM_DEVICE_MANAGERS] = {0};

mtx_t deviceThreadMutex[NUM_DEVICE_MANAGERS] = {0};

cnd_t deviceThreadWake[NUM_DEVICE_MANAGERS] = {0};

deviceThreadSemaphore_e deviceThreadSemaphore[NUM_DEVICE_MANAGERS] = {0};

void mb_initDevices()
{
    threadArg_t *tmpThreadArg = NULL;

    // Add each device manager as required
    thrd_start_t initFunctions[NUM_DEVICE_MANAGERS] = 
    {
        initDummyDeviceManager,
        initDummyDeviceManager,
        initDummyDeviceManager,
        initDummyDeviceManager,
        initDummyDeviceManager
    };

    for (uint8_t i = 0; i < NUM_DEVICE_MANAGERS; i++)
    {
        if (NULL == initFunctions[i])
        {
            continue;
        }

        tmpThreadArg = calloc(1, sizeof(threadArg_t));

        tmpThreadArg->mutex         = &(deviceThreadMutex[i]);
        tmpThreadArg->wakeCondition = &(deviceThreadWake[i]);
        tmpThreadArg->semaphore     = &(deviceThreadSemaphore[i]);
        tmpThreadArg->managerId     = i;

        deviceThreads[i] = calloc(1, sizeof(thrd_t));
        thrd_create(deviceThreads[i], initFunctions[i], (void*) tmpThreadArg);
    }
}

void mb_teardownDevices()
{
    for (uint8_t i = 0; i < NUM_DEVICE_MANAGERS; i++)
    {
        if (NULL != deviceThreads[i])
        {
            mtx_lock(&(deviceThreadMutex[i]));
            deviceThreadSemaphore[i] = dts_kill;
            mtx_unlock(&(deviceThreadMutex[i]));

            cnd_signal(&(deviceThreadWake[i]));
            thrd_join(*(deviceThreads[i]), NULL);
            free(deviceThreads[i]);
        }
    }
}