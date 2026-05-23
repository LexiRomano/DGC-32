#include "deviceManagers/dummyDeviceManager.h"

static void ddm_handleWrite(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *data)
{

}

int ddm_initDeviceManager(void *arg)
{
    threadArg_t myThreadData = {0};

    memcpy(&myThreadData, arg, sizeof(threadArg_t));
    free(arg);

    mtx_lock(myThreadData.mutex);

    // Request initial devices

    if (NEW_DEVICE_REQUEST_FAILED == dmi_requestNewDevice(myThreadData.managerId, 2))
    {
        myThreadData.semaphore->wakeReason = dts_kill;
        cnd_signal(myThreadData.wakeCondition);
        mtx_unlock(myThreadData.mutex);
        return -1;
    }

    // Bind the read handler
    dmi_bindHandleWrite(myThreadData.managerId, ddm_handleWrite);

    cnd_signal(myThreadData.wakeCondition);

    while (true)
    {
        cnd_wait(myThreadData.wakeCondition, myThreadData.mutex);

        switch (myThreadData.semaphore->wakeReason)
        {
            case dts_kill:
                return 0;
            case dts_handleReadWrite:
            case dts_continue:
                break;
        }

        myThreadData.semaphore->wakeReason = dts_continue;
    }
}