#include "deviceManagers/dummyDeviceManager.h"

void ddm_handleWrite(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *data)
{

}

int ddm_initDeviceManager(void *arg)
{
    threadArg_t myThreadData = {0};

    memcpy(&myThreadData, arg, sizeof(threadArg_t));
    free(arg);

    printf("%hhu: I'm alive!\n", myThreadData.managerId);

    // Request initial devices

    if (NEW_DEVICE_REQUEST_FAILED == dmi_requestNewDevice(myThreadData.managerId, 32))
    {
        *(myThreadData.semaphore) = dts_kill;
        cnd_signal(myThreadData.wakeCondition);
        printf("Could not initialize device\n");
        return false;
    }

    // Bind the read handler
    dmi_bindHandleWrite(myThreadData.managerId, ddm_handleWrite);

    cnd_signal(myThreadData.wakeCondition);
    mtx_lock(myThreadData.mutex);

    while (true)
    {
        printf("%hhu: Going to sleep...\n", myThreadData.managerId);

        cnd_wait(myThreadData.wakeCondition, myThreadData.mutex);

        printf("%hhu: Woke up\n", myThreadData.managerId);

        switch (*(myThreadData.semaphore))
        {
            case dts_kill:
                printf("%hhu: Goodbye, cruel world!\n", myThreadData.managerId);
                return 0;
            case dts_handleWrite:
            case dts_continue:
                break;
        }


        *(myThreadData.semaphore) = dts_continue;
    }
}