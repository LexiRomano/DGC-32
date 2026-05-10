#include "motherboard.h"

// Multithreading framework for device managers
static thrd_t                 *managerThreads[NUM_DEVICE_MANAGERS]              = {0};
static mtx_t                   managerThreadMutex[NUM_DEVICE_MANAGERS]          = {0};
static cnd_t                   managerThreadWake[NUM_DEVICE_MANAGERS]           = {0};
static deviceThreadSemaphore_e managerThreadSemaphore[NUM_DEVICE_MANAGERS]      = {0};
static bool                    managerWakerTodo[NUM_DEVICE_MANAGERS]            = {0};
static handleWriteFP_t         managerHandleWriteFunctions[NUM_DEVICE_MANAGERS] = {0};

// Waker thread infrastructure
static thrd_t *wakerThread     = NULL;
static bool    wakerThreadLive = false;

// CPU-owned read/write functions for general use memory
static memTransFP_t  writeMem         = NULL;
static memTransFP_t  readMem          = NULL;
static interruptFP_t enqueueInterrupt = NULL;

// For keeping track of devices and mapping them into memory
static mb_device_t *firstDeviceInMem                   = NULL;
static mb_device_t *deviceMappingData[MAX_NUM_DEVICES] = {0};
static mtx_t        deviceRegistryMutex                = {0};

// Bindings for device managers. Add as required
thrd_start_t managerInitFunctions[NUM_DEVICE_MANAGERS] = 
{
    ddm_initDeviceManager,
    pr_initDeviceManager,
    ddm_initDeviceManager,
    ddm_initDeviceManager,
    ddm_initDeviceManager
};
deviceTypes_e managerDeviceType[NUM_DEVICE_MANAGERS] = 
{
    dt_none,
    dt_peripheral,
    dt_none,
    dt_none,
    dt_none
};

// GLFW variables
glfwInfo_t *glfwInfo = NULL;

// Misc
static bool powerState = false;

static bool isInterruptTypeAllowed(uint8_t deviceId, interruptTypes_e interruptType)
{
    if (NULL == deviceMappingData[deviceId])
    {
        return false;
    }

    switch (interruptType)
    {
        case it_keyPressed:
            if (dt_peripheral == deviceMappingData[deviceId]->deviceType)
            {
                return true;
            }

            return false;

        case it_keyReleased:
            if (dt_peripheral == deviceMappingData[deviceId]->deviceType)
            {
                return true;
            }

            return false;

        case it_peripheralEvent:
            if (dt_peripheral == deviceMappingData[deviceId]->deviceType)
            {
                return true;
            }

            return false;

        case it_motherboardEvent:
            if (dt_motherboard == deviceMappingData[deviceId]->deviceType)
            {
                return true;
            }

            return false;

        case it_timerEvent:
            if (dt_timer == deviceMappingData[deviceId]->deviceType)
            {
                return true;
            }

            return false;
            
        case it_storageEvent:
            if (dt_storage == deviceMappingData[deviceId]->deviceType)
            {
                return true;
            }

            return false;

        case it_graphicalEvent:
            if (dt_graphical == deviceMappingData[deviceId]->deviceType)
            {
                return true;
            }

            return false;
    }

    return false;
}

/*******************************************************************************
* Requests to enqueue an interrupt. Validated based on allowed interrupt types
* for the device requesting and if the parameter is properly formatted.
* Returns success.
*******************************************************************************/
bool dmi_enqueueInterrupt(uint8_t deviceId, interruptTypes_e interruptType, uint8_t interruptParameter)
{
    uint16_t interruptData = 0;

    if (deviceId >= MAX_NUM_DEVICES             ||
        NULL     == deviceMappingData[deviceId] ||
        false    == isInterruptTypeAllowed(deviceId, interruptType))
    {
        return false;
    }

    // Get interrupt data
    switch (interruptType)
    {
        case it_keyPressed:
            interruptData = INTERRUPT_CODE_KEY_PRESSED;
            break;

        case it_keyReleased:
            interruptData = INTERRUPT_CODE_KEY_RELEASED;
            break;

        case it_peripheralEvent:
            interruptData = INTERRUPT_CODE_PERIPHERAL_EVENT;
            break;

        case it_motherboardEvent:
            interruptData = INTERRUPT_CODE_MOTHERBOARD_EVENT;
            break;

        case it_timerEvent:
            interruptData = INTERRUPT_CODE_TIMER_EVENT;
            break;

        case it_storageEvent:
            interruptData = INTERRUPT_CODE_STORAGE_EVENT;
            break;

        case it_graphicalEvent:
            interruptData = INTERRUPT_CODE_GRAPHICAL_EVENT;
            break;

        default:
            return false;
    }

    // Add device ID if applicable
    switch(interruptType)
    {
        case it_peripheralEvent:
        case it_timerEvent:
        case it_storageEvent:
        case it_graphicalEvent:
            break;
            if (0 != (interruptParameter & 0b111))
            {
                return false;
            }
            interruptData = interruptData | (deviceId << 8);
            break;

        default:
            break;
    }

    // Add parameter
    interruptData = interruptData | (interruptParameter << 8);

    enqueueInterrupt(interruptData);

    return false;
}

/*******************************************************************************
* Requests a new device to be added to the device registry. Returns the new
* device ID, or NEW_DEVICE_REQUEST_FAILED if it failed.
*******************************************************************************/
uint8_t dmi_requestNewDevice(uint8_t managerId, uint16_t size)
{
    mb_device_t *newDevice       = NULL;
    uint8_t      newDeviceId     = NEW_DEVICE_REQUEST_FAILED;
    bool         isAtEndOfTable  = false;

    mtx_lock(&deviceRegistryMutex);

    if (size      >  MEMBOUND_DDAT_END - MEMBOUND_DDAT_START + 1 ||
        managerId >= NUM_DEVICE_MANAGERS)
    {
        mtx_unlock(&deviceRegistryMutex);
        return NEW_DEVICE_REQUEST_FAILED;
    }

    // Find first available device id
    for (uint8_t i = 0; i < MAX_NUM_DEVICES; i++)
    {
        if (NULL == deviceMappingData[i])
        {
            newDeviceId = i;
            break;
        }
    }

    if (NEW_DEVICE_REQUEST_FAILED == newDeviceId)
    {
        mtx_unlock(&deviceRegistryMutex);
        return NEW_DEVICE_REQUEST_FAILED;
    }

    // Find next available patch of memory
    if (NULL == firstDeviceInMem)
    {
        newDevice = calloc(1, sizeof(*newDevice));

        firstDeviceInMem               = newDevice;

        newDevice->startLocation = MEMBOUND_DDAT_START;
    }
    else if (MEMBOUND_DDAT_START + size - 1 < firstDeviceInMem->startLocation)
    {
        newDevice = calloc(1, sizeof(*newDevice));

        newDevice->nextInMem = firstDeviceInMem;

        firstDeviceInMem               = newDevice;

        newDevice->startLocation = MEMBOUND_DDAT_START;
    }
    else
    {
        for (mb_device_t *p = firstDeviceInMem; NULL != p; p = p->nextInMem)
        {
            if ((p->startLocation + p->size + size - 1) < 
                (NULL == p->nextInMem ? MEMBOUND_DDAT_END + 1 : p->startLocation))
            {
                newDevice = calloc(1, sizeof(*newDevice));

                newDevice->nextInMem = p->nextInMem;

                p->nextInMem = newDevice;

                newDevice->startLocation = p->startLocation + p->size;
                break;
            }
        }

        if (NULL == newDevice)
        {
            mtx_unlock(&deviceRegistryMutex);
            return NEW_DEVICE_REQUEST_FAILED;
        }
    }

    // Initialize the rest of the data
    newDevice->size       = size;
    newDevice->deviceType = managerDeviceType[managerId];
    newDevice->deviceId   = newDeviceId;
    newDevice->managerId  = managerId;

    // Add to mappings
    deviceMappingData[newDeviceId] = newDevice;

    writeMem(MEMBOUND_DREG_START + (newDeviceId * 8) + 1, 1, &(newDevice->deviceType));
    writeMem(MEMBOUND_DREG_START + (newDeviceId * 8) + 2, 2, &size);
    writeMem(MEMBOUND_DREG_START + (newDeviceId * 8) + 4, 4, &(newDevice->startLocation));

    // Adding table end marker if this was added to the end of the table
    isAtEndOfTable = true;
    for (uint8_t i = newDeviceId + 1; i < MAX_NUM_DEVICES; i++)
    {
        if (NULL != deviceMappingData[i])
        {
            isAtEndOfTable = false;
            break;
        }
    }

    if (isAtEndOfTable)
    {
        writeMem(MEMBOUND_DREG_START + (newDeviceId * 8) + 6, 1, (uint8_t[]) {DEVICE_TABLE_END_MARKER});
    }

    mtx_unlock(&deviceRegistryMutex);

    // Trigger device table update interrupt
    if (powerState)
    {
        enqueueInterrupt(INTERRUPT_CODE_MOTHERBOARD_EVENT + (newDeviceId << 8));
    }
    
    return newDeviceId;
}

/*******************************************************************************
* Removes the specified device from the registry.
*******************************************************************************/
void dmi_removeDevice(uint8_t deviceId)
{
    mb_device_t *deviceToDelete = NULL;
    bool         isAtEndOfTable = false;

    mtx_lock(&deviceRegistryMutex);

    if (NULL == deviceMappingData[deviceId])
    {
        mtx_unlock(&deviceRegistryMutex);
        return;
    }

    deviceToDelete = deviceMappingData[deviceId];

    deviceMappingData[deviceId] = NULL;

    if (firstDeviceInMem == deviceToDelete)
    {
        firstDeviceInMem = firstDeviceInMem->nextInMem;

        free(deviceToDelete);

        mtx_unlock(&deviceRegistryMutex);
        return;
    }

    for (mb_device_t *p = firstDeviceInMem; NULL != p; p = p->nextInMem)
    {
        if (p->nextInMem == deviceToDelete)
        {
            p->nextInMem = deviceToDelete->nextInMem;
            break;
        }
    }

    isAtEndOfTable = true;
    for (uint8_t i = deviceId + 1; i < MAX_NUM_DEVICES; i++)
    {
        if (NULL != deviceMappingData[i])
        {
            isAtEndOfTable = false;
            break;
        }
    }

    if (isAtEndOfTable)
    {
        writeMem(MEMBOUND_DREG_START + (deviceId * 8) + 1, 1, (uint8_t[]) {DEVICE_TABLE_END_MARKER});
    }
    else
    {   
        writeMem(MEMBOUND_DREG_START + (deviceId * 8) + 1, 1, (uint8_t[]) {DEVICE_TALBE_EMPTY_MARKER});
    }

    free(deviceToDelete);

    mtx_unlock(&deviceRegistryMutex);
}

/*******************************************************************************
* Interface for devices to read from their own data. Undefined behaviour if the
* respective device is deleted mid-read.
*******************************************************************************/
bool dmi_readDeviceData(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *buf)
{
    if (NULL == deviceMappingData[deviceId] ||
        NULL == buf)
    {
        return false;
    }

    if (deviceDataAddress + numBytes - 1 >= deviceMappingData[deviceId]->size)
    {
        return false;
    }

    if (0 == numBytes)
    {
        return true;
    }

    readMem(deviceDataAddress + deviceMappingData[deviceId]->startLocation, numBytes, buf);

    return true;
}

/*******************************************************************************
* Interface for devices to write to their own data. Undefined behaviour if the
* respective device is deleted mid-write.
*******************************************************************************/
bool dmi_writeDeviceData(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *buf)
{
    if (NULL == deviceMappingData[deviceId] ||
        NULL == buf)
    {
        return false;
    }

    if (deviceDataAddress + numBytes - 1 >= deviceMappingData[deviceId]->size)
    {
        return false;
    }

    if (0 == numBytes)
    {
        return true;
    }

    writeMem(deviceDataAddress + deviceMappingData[deviceId]->startLocation, numBytes, buf);

    return true;
}

/*******************************************************************************
* Interface for device managers to read from general use memory.
*******************************************************************************/
bool dmi_readFromMemory(uint32_t address, uint8_t numBytes, void *data)
{
    if (address < MEMBOUND_GEN_START ||
        (uint64_t) address + numBytes - 1 > MEMBOUND_GEN_END ||
        0    == numBytes ||
        NULL == data);
    {
        return false;
    }

    readMem(address, numBytes, data);

    return true;
}

/*******************************************************************************
* Interface for device managers to write to general use memory.
*******************************************************************************/
bool dmi_writeToMemory(uint32_t address, uint8_t numBytes, void *data)
{
    if (address < MEMBOUND_GEN_START ||
        (uint64_t) address + numBytes - 1 > MEMBOUND_GEN_END ||
        0    == numBytes ||
        NULL == data);
    {
        return false;
    }

    writeMem(address, numBytes, data);

    return true;
}

/*******************************************************************************
* Interface for device managers to bind a function which will get invoked
* whenever the processor writes to the data of one of its devices.
*******************************************************************************/
bool dmi_bindHandleWrite(uint8_t managerId, handleWriteFP_t handleWriteFunction)
{
    if (managerId >= NUM_DEVICE_MANAGERS ||
        NULL == handleWriteFunction)
    {
        return false;
    }

    managerHandleWriteFunctions[managerId] = handleWriteFunction;

    return true;
}

/*******************************************************************************
* Interface for the processor to handle writes to device data.
*******************************************************************************/
void mb_writeToDeviceData(uint32_t address, uint8_t size, void *data)
{
    mb_device_t *foundDevice = NULL;

    if (address < MEMBOUND_DDAT_START ||
        address > MEMBOUND_DDAT_END)
    {
        // Not writing to device data
        return;
    }

    if (address + size - 1 > MEMBOUND_DDAT_END)
    {
        // Write splits region boundary
        enqueueInterrupt(INTERRUPT_CODE_MOTHERBOARD_EVENT + 0b00001000);
        return;
    }

    mtx_lock(&deviceRegistryMutex);
    for (mb_device_t *p = firstDeviceInMem; NULL != p; p = p->nextInMem)
    {
        if (address >= p->startLocation &&
            address + size - 1 <= p->startLocation + p->size - 1)
        {
            foundDevice = p;
            break;
        }

        if (address + size - 1 > p->startLocation + p->size - 1)
        {
            break;
        }
    }

    if (NULL == foundDevice)
    {
        // Invalid memory region
        enqueueInterrupt(INTERRUPT_CODE_MOTHERBOARD_EVENT + 0b00001000);
        mtx_unlock(&deviceRegistryMutex);
        return;
    }

    if (NULL != managerHandleWriteFunctions[foundDevice->managerId])
    {
        managerHandleWriteFunctions[foundDevice->managerId](foundDevice->deviceId, 
                                                            address - foundDevice->startLocation,
                                                            size,
                                                            data);
    }

    managerWakerTodo[foundDevice->managerId] = true;

    mtx_unlock(&deviceRegistryMutex);
}

/*******************************************************************************
* Waker thread to check fo if device managers need to be woken up.
*******************************************************************************/
static int wakerThreadFunction(void *arg)
{
    while (wakerThreadLive)
    {
        thrd_yield();

        for (uint8_t i = 0; i < MAX_NUM_DEVICES; i++)
        {
            if (false == managerWakerTodo[i])
            {
                continue;
            }

            managerWakerTodo[i] = false;

            mtx_lock(&(managerThreadMutex[i]));
            managerThreadSemaphore[i] = dts_handleWrite;
            mtx_unlock(&(managerThreadMutex[i]));

            cnd_signal(&(managerThreadWake[i]));
        }

        glfwPollEvents();

        if (glfwWindowShouldClose(glfwInfo->window))
        {
            powerState = false;
        }
    }

    return 0;
}

/*******************************************************************************
* Interface for the processor to query if the power is still on.
*******************************************************************************/
bool mb_powerState()
{
    return powerState;
}

/*******************************************************************************
* Initialize the motherboard and all attached device managers.
*******************************************************************************/
bool mb_init(externalFileInfo_t *externalFileInfo, glfwInfo_t *glfw, memTransFP_t read, memTransFP_t write, interruptFP_t interrupt)
{
    threadArg_t *tmpThreadArg = NULL;

    // Bind processor access functions
    readMem          = read;
    writeMem         = write;
    enqueueInterrupt = interrupt;
    glfwInfo         = glfw;

    powerState = false;

    mtx_init(&deviceRegistryMutex, mtx_plain);

    // Launch each device manager
    for (uint8_t i = 0; i < NUM_DEVICE_MANAGERS; i++)
    {
        if (NULL == managerInitFunctions[i])
        {
            continue;
        }

        // Initialize thread data
        tmpThreadArg = calloc(1, sizeof(threadArg_t));

        mtx_init(&(managerThreadMutex[i]), mtx_plain);
        cnd_init(&(managerThreadWake[i]));

        managerThreadSemaphore[i] = dts_continue;

        tmpThreadArg->mutex         = &(managerThreadMutex[i]);
        tmpThreadArg->wakeCondition = &(managerThreadWake[i]);
        tmpThreadArg->semaphore     = &(managerThreadSemaphore[i]);
        tmpThreadArg->managerId     = i;

        memcpy(&(tmpThreadArg->externalFileInfo), externalFileInfo, sizeof(externalFileInfo_t));

        memcpy(&(tmpThreadArg->glfwInfo), glfwInfo, sizeof(glfwInfo_t));

        managerThreads[i] = calloc(1, sizeof(thrd_t));

        // Launch the thread with this sequencing:
        // 1 - The motherboard grabs the manager's thread before launch
        // 2 - The manager's thread gets launched
        // 3 - The motherboard waits for the manager to initialize
        // 4 - The manager will initialize itself, including:
        //         Requesting its initial devices
        //         Binding its handle write functions
        // 5 - The manager signals the motherboard it has completed its init
        // 6 - The motherboard verifies that the manager came up properly
        // 7 - The motherboard releases the manager's mutex
        // 8 - The manager takes ownership of its mutex
        mtx_lock(&(managerThreadMutex[i]));
        thrd_create(managerThreads[i], managerInitFunctions[i], (void*) tmpThreadArg);
        cnd_wait(&(managerThreadWake[i]), &(managerThreadMutex[i]));

        // Check the success of the manager's initialization
        if (NULL == managerHandleWriteFunctions[i])
        {
            managerThreadSemaphore[i] = dts_kill;
        }

        if (dts_kill == managerThreadSemaphore[i])
        {
            printf("Failed to initialize device[%hhu]\n", i);
            mtx_unlock(&(managerThreadMutex[i]));
            return false;
        }

        mtx_unlock(&(managerThreadMutex[i]));
    }

    // Launch waker thread
    wakerThreadLive = true;
    wakerThread     = calloc(1, sizeof(thrd_t));
    thrd_create(wakerThread, wakerThreadFunction, NULL);


    powerState = true;

    return true;
}

/*******************************************************************************
* Teardown all device manager threads.
*******************************************************************************/
void mb_teardown()
{
    // Kill waker thread:
    wakerThreadLive = false;

    // Kill all device handlers:
    for (uint8_t i = 0; i < NUM_DEVICE_MANAGERS; i++)
    {
        if (NULL != managerThreads[i])
        {
            mtx_lock(&(managerThreadMutex[i]));
            managerThreadSemaphore[i] = dts_kill;
            mtx_unlock(&(managerThreadMutex[i]));

            cnd_signal(&(managerThreadWake[i]));
            thrd_join(*(managerThreads[i]), NULL);
            free(managerThreads[i]);

            mtx_destroy(&(managerThreadMutex[i]));
            cnd_destroy(&(managerThreadWake[i]));
        }
    }
}