#include "deviceManagers/timers.h"

static uint8_t  dimerDeviceId        = 0;
static bool     dimerRepeat          = false;
static bool     dimerInterrupt       = false;
static uint16_t dimerDuration        = 0;
static cnd_t    dimerThreadCondition = {0};
static mtx_t    dimerThreadMutex     = {0};
static thrd_t   dimerThread          = {0};
static bool     dimerDie             = false;

static bool dimerCanWriteMap[] =
{
    false,
    true,
    true, true
};

static int tm_dimerThread(void *arg)
{
    (void)arg;

    mtx_lock(&dimerThreadMutex);

    while (false == dimerDie)
    {
        cnd_wait(&dimerThreadCondition, &dimerThreadMutex);

        if (dimerDie)
        {
            return 0;
        }

        if (0 == dimerDuration)
        {
            continue;
        }

        do
        {
            usleep(dimerDuration);
            if (0 != dimerDuration && dimerInterrupt)
            {
                dmi_enqueueInterrupt(dimerDeviceId, it_timerEvent, 0);
            }
        }
        while (dimerRepeat && 0 != dimerDuration);

        dmi_writeDeviceData(dimerDeviceId, DIMER_DURATION_ADDRESS, 2, (uint16_t[]) {0});
    }

    return 0;
}

static void tm_handleWrite(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *data)
{
    if (dimerDeviceId != deviceId)
    {
        return;
    }

    if (DIMER_CONFIG_ADDRESS == deviceDataAddress &&
        1                    == numBytes)
    {
        dimerRepeat = 0 != ((*((uint8_t*)data)) & 0b00000001);
    }
    else if (DIMER_DURATION_ADDRESS == deviceDataAddress &&
             2                      == numBytes)
    {
        dimerDuration = *((uint16_t*)data);
        cnd_signal(&dimerThreadCondition);
    }
}

int tm_initDeviceManager(void *arg)
{
    threadArg_t myThreadData = {0};

    memcpy(&myThreadData, arg, sizeof(threadArg_t));
    free(arg);

    mtx_lock(myThreadData.mutex);

    // Request initial devices

    dimerDeviceId = dmi_requestNewDevice(myThreadData.managerId,
                                         DIMER_DDAT_SIZE,
                                         dimerCanWriteMap);

    if (NEW_DEVICE_REQUEST_FAILED == dimerDeviceId ||
        false == dmi_writeDeviceData(dimerDeviceId,
                                     0,
                                     1,
                                     (uint8_t[]) {DIMER_TPI}))
    {
        myThreadData.semaphore->wakeReason = dts_kill;
        cnd_signal(myThreadData.wakeCondition);
        mtx_unlock(myThreadData.mutex);
        return -1;
    }

    mtx_init   (&dimerThreadMutex, mtx_plain);
    cnd_init   (&dimerThreadCondition);
    thrd_create(&dimerThread, tm_dimerThread, NULL);


    // Bind the read handler
    dmi_bindHandleWrite(myThreadData.managerId, tm_handleWrite);

    cnd_signal(myThreadData.wakeCondition);

    while (true)
    {
        cnd_wait(myThreadData.wakeCondition, myThreadData.mutex);

        switch (myThreadData.semaphore->wakeReason)
        {
            case dts_kill:
                dimerDie = true;
                cnd_signal(&dimerThreadCondition);
                thrd_join(dimerThread, NULL);
                return 0;
            case dts_handleReadWrite:
            case dts_continue:
                break;
        }

        myThreadData.semaphore->wakeReason = dts_continue;
    }
}