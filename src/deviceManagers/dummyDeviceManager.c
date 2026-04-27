#include "deviceManagers/dummyDeviceManager.h"

int initDummyDeviceManager(void *arg)
{
    threadArg_t myThreadData = {0};

    memcpy(&myThreadData, arg, sizeof(threadArg_t));
    free(arg);

    printf("%hhu: I'm alive!\n", myThreadData.managerId);

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
            case dts_write:
            case dts_continue:
                break;
        }


        *(myThreadData.semaphore) = dts_continue;
    }
}