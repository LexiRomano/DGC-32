#include "deviceManagers/storage.h"

threadArg_t myThreadData = {0};

static uint8_t driveManagerId = 0;
static bool    insertNewDrive      = false;
static bool    insertExistingDrive = false;
static bool    removeDrive         = false;

static driveData_t *driveData[DARDRIVE_MAX_DRIVE_COUNT]  = {0};
static bool         driveRead[DARDRIVE_MAX_DRIVE_COUNT]  = {0};
static bool         driveWrite[DARDRIVE_MAX_DRIVE_COUNT] = {0};

// Returns 0xFF if not found
static uint8_t st_deviceIdToDriveIndex(uint8_t deviceId)
{
    for (uint8_t i = 0; i < DARDRIVE_MAX_DRIVE_COUNT; i++)
    {
        if (NULL     == driveData[i] ||
            deviceId != driveData[i]->deviceId)
        {
            continue;
        }

        return i;
    }

    return 0xFF;
}

static void st_handleWrite(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *data)
{
    uint8_t driveIndex = 0xFF;

    if (deviceId == driveManagerId)
    {
        if (0 == deviceDataAddress)
        {
            dmi_writeDeviceData(deviceId, 0, 1, (uint8_t[]) {DARDRIVE_M_SPI});
        }
        else if (DARDRIVE_M_INITIATE_ADDRESS == deviceDataAddress &&
                 1                           == numBytes)
        {
            switch (*((uint8_t*) data))
            {
                case 0:
                    insertNewDrive = true;
                    break;
                case 1:
                    insertExistingDrive = true;
                    break;
                case 2:
                    removeDrive = true;
                    break;
            }
        }
    }
    else
    {
        if (deviceDataAddress <= DARDRIVE_D_SECTOR_COUNT_ADDRESS)
        {
            driveIndex = st_deviceIdToDriveIndex(deviceId);
            dmi_writeDeviceData(deviceId, 0, 3, (uint8_t[]) {DARDRIVE_D_SPI,
                                                             driveData[driveIndex]->sectorSize,
                                                             driveData[driveIndex]->sectorCount});
        }
        else if (DARDRIVE_D_INITIATE_ADDRESS == deviceDataAddress &&
                 1                           == numBytes)
        {
            driveIndex = st_deviceIdToDriveIndex(deviceId);

            switch (*((uint8_t*) data))
            {
                case 0:
                    driveRead[driveIndex] = true;
                    break;
                case 1:
                    driveWrite[driveIndex] = true;
                    break;
            }
        }
    }
}

static void st_freeDriveData(driveData_t *driveData)
{
    if (NULL == driveData)
    {
        return;
    }

    if (NULL != driveData->fileName)
    {
        free(driveData->fileName);
    }

    free(driveData);
}

static bool st_insertNewDrive(char *fileName, uint8_t sectorSize, uint8_t sectorCount)
{
    FILE        *fd            = NULL;
    uint8_t      newDriveIndex = 0;
    driveData_t *newDriveData  = NULL;


    if (NULL == fileName ||
        strnlen(fileName, DARDRIVE_M_MAX_FILE_NAME_SIZE + 1) > DARDRIVE_M_MAX_FILE_NAME_SIZE)
    {
        return false;
    }

    if (0 == access(fileName, F_OK))
    {
        return false;
    }

    newDriveIndex = 0xFF;
    for (uint8_t i = 0; i < DARDRIVE_MAX_DRIVE_COUNT; i++)
    {
        if (NULL == driveData[i])
        {
            newDriveIndex = i;
        }
    }

    if (0xFF == newDriveIndex)
    {
        return false;
    }

    fd = fopen(fileName, "w");

    if (NULL == fd)
    {
        return false;
    }

    if (4 != fwrite((uint8_t[]) {DARDRIVE_M_SPI,
                                 DARDRIVE_D_SPI,
                                 sectorSize,
                                 sectorCount},
                    1, 4, fd))
    {
        fclose(fd);
        return false;
    }

    fclose(fd);

    newDriveData              = calloc(1, sizeof(driveData_t));
    newDriveData->fileName    = strcpy(calloc(strlen(fileName) + 1, sizeof(char)), fileName);;
    newDriveData->sectorSize  = sectorSize;
    newDriveData->sectorCount = sectorCount;
    newDriveData->deviceId    = dmi_requestNewDevice(myThreadData.managerId, DARDRIVE_D_DDAT_SIZE);

    if (NEW_DEVICE_REQUEST_FAILED == newDriveData->deviceId)
    {
        st_freeDriveData(newDriveData);
        return false;
    }

    if (false == dmi_writeDeviceData(newDriveData->deviceId,
                                     0,
                                     3,
                                     (uint8_t[]) {DARDRIVE_D_SPI,
                                                  sectorSize,
                                                  sectorCount}))
    {
        dmi_removeDevice(newDriveData->deviceId);
        st_freeDriveData(newDriveData);
        return false;
    }

    driveData[newDriveIndex] = newDriveData;

    return true;
}

static bool st_insertExistingDriveFile(char *fileName)
{
    FILE        *fd            = NULL;
    uint8_t      newDriveIndex = 0;
    driveData_t *newDriveData  = NULL;
    uint8_t      readBuf[4]    = {0};


    if (NULL == fileName ||
        strnlen(fileName, DARDRIVE_M_MAX_FILE_NAME_SIZE + 1) > DARDRIVE_M_MAX_FILE_NAME_SIZE)
    {
        return false;
    }

    if (0 != access(fileName, F_OK) ||
        0 != access(fileName, R_OK | W_OK))
    {
        return false;
    }

    newDriveIndex = 0xFF;
    for (uint8_t i = 0; i < DARDRIVE_MAX_DRIVE_COUNT; i++)
    {
        if (NULL == driveData[i])
        {
            newDriveIndex = i;
        }
    }

    if (0xFF == newDriveIndex)
    {
        return false;
    }

    fd = fopen(fileName, "r");

    if (NULL == fd)
    {
        return false;
    }

    if (4 != fread(readBuf, 1, 4, fd))
    {
        fclose(fd);
        return false;
    }

    fclose(fd);

    if (DARDRIVE_M_SPI != readBuf[0] ||
        DARDRIVE_D_SPI != readBuf[1])
    {
        return false;
    }

    newDriveData              = calloc(1, sizeof(driveData_t));
    newDriveData->fileName    = strcpy(calloc(strlen(fileName) + 1, sizeof(char)), fileName);
    newDriveData->sectorSize  = readBuf[2];
    newDriveData->sectorCount = readBuf[3];
    newDriveData->deviceId    = dmi_requestNewDevice(myThreadData.managerId, DARDRIVE_D_DDAT_SIZE);

    if (NEW_DEVICE_REQUEST_FAILED == newDriveData->deviceId)
    {
        st_freeDriveData(newDriveData);
        return false;
    }

    if (false == dmi_writeDeviceData(newDriveData->deviceId,
                                     0,
                                     3,
                                     (uint8_t[]) {DARDRIVE_D_SPI,
                                                  readBuf[2],
                                                  readBuf[3]}))
    {
        dmi_removeDevice(newDriveData->deviceId);
        st_freeDriveData(newDriveData);
        return false;
    }

    driveData[newDriveIndex] = newDriveData;

    return true;
}

static bool st_removeDrive(uint8_t driveDeviceId)
{
    uint8_t foundDriveIndex = st_deviceIdToDriveIndex(driveDeviceId);

    if (0xFF == foundDriveIndex)
    {
        return false;
    }

    dmi_removeDevice(driveData[foundDriveIndex]->deviceId);

    st_freeDriveData(driveData[foundDriveIndex]);

    driveData[foundDriveIndex] = NULL;

    return true;
}

static void st_handleManagerWriteWake()
{
    uint8_t  ddatBuf[DARDRIVE_M_DDAT_SIZE]                  = {0};
    uint8_t  fileNameBuf[DARDRIVE_M_MAX_FILE_NAME_SIZE + 1] = {0};
    uint32_t readAddress                                    = 0;

    if (false == insertNewDrive &&
        false == insertExistingDrive &&
        false == removeDrive)
    {
        return;
    }

    if (false == dmi_readDeviceData(driveManagerId, 0, DARDRIVE_M_DDAT_SIZE, ddatBuf))
    {
        return;
    }

    if (insertNewDrive)
    {
        insertNewDrive = false;

        memcpy(&readAddress, &(ddatBuf[DARDRIVE_M_NAME_POINTER_ADDRESS]), sizeof(uint32_t));

        if (false == dmi_readFromMemory(readAddress, DARDRIVE_M_MAX_FILE_NAME_SIZE + 1, fileNameBuf))
        {
            dmi_enqueueInterrupt(driveManagerId, it_storageEvent, DARDRIVE_M_FAILURE_OVERLAY);
            return;
        }
    
        if (true == st_insertNewDrive((char *) fileNameBuf,
                                      ddatBuf[DARDRIVE_M_SECTOR_SIZE_ADDRESS],
                                      ddatBuf[DARDRIVE_M_SECTOR_COUNT_ADDRESS]))
        {
            dmi_enqueueInterrupt(driveManagerId, it_storageEvent, 0);
        }
        else
        {
            dmi_enqueueInterrupt(driveManagerId, it_storageEvent, DARDRIVE_M_FAILURE_OVERLAY);
        }
    }
    else if (insertExistingDrive)
    {
        insertExistingDrive = false;

        memcpy(&readAddress, &(ddatBuf[DARDRIVE_M_NAME_POINTER_ADDRESS]), sizeof(uint32_t));

        if (false == dmi_readFromMemory(readAddress, DARDRIVE_M_MAX_FILE_NAME_SIZE + 1, fileNameBuf))
        {
            dmi_enqueueInterrupt(driveManagerId, it_storageEvent, DARDRIVE_M_FAILURE_OVERLAY);
            return;
        }

        if (true == st_insertExistingDriveFile((char *) fileNameBuf))
        {
            dmi_enqueueInterrupt(driveManagerId, it_storageEvent, 0);
        }
        else
        {
            dmi_enqueueInterrupt(driveManagerId, it_storageEvent, DARDRIVE_M_FAILURE_OVERLAY);
        }
    }
    else
    {
        removeDrive = false;

        if (true == st_removeDrive(ddatBuf[DARDRIVE_M_DRIVE_ID_ADDRESS]))
        {
            dmi_enqueueInterrupt(driveManagerId, it_storageEvent, 0);
        }
        else
        {
            dmi_enqueueInterrupt(driveManagerId, it_storageEvent, DARDRIVE_M_FAILURE_OVERLAY);
        }
    }
}

static void st_handleDiskWriteWake(uint8_t deviceId)
{
    uint8_t      driveIndex                    = 0xFF;
    uint8_t      ddatBuf[DARDRIVE_D_DDAT_SIZE] = {0};
    driveData_t *driveToHandle                 = NULL;
    FILE        *fd                            = NULL;
    bool         isRead                        = false;
    uint32_t     sectorSize                    = 0;
    uint32_t     maxSectorCount                = 0;
    uint32_t     selectedSector                = 0;
    uint64_t     addressOnFile                 = 0;
    uint32_t     addressInMemory               = 0;
    uint8_t      buf[128]                      = {0};
    uint8_t      bytesRead                     = 0;
    bool         eofReached                    = false;

    driveIndex = st_deviceIdToDriveIndex(deviceId);

    if (0xFF == driveIndex)
    {
        dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
        return;
    }

    if (false == driveRead[driveIndex] &&
        false == driveWrite[driveIndex])
    {
        return;
    }

    if (driveRead[driveIndex])
    {
        driveRead[driveIndex] = false;
        isRead = true;
    }
    else
    {
        driveWrite[driveIndex] = false;
    }

    if (false == dmi_readDeviceData(deviceId, 0, DARDRIVE_D_DDAT_SIZE, ddatBuf))
    {
        dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
        return;
    }

    driveToHandle = driveData[driveIndex];

    maxSectorCount = 1 << ddatBuf[DARDRIVE_D_SECTOR_COUNT_ADDRESS];

    memcpy(&selectedSector, &(ddatBuf[DARDRIVE_D_DRIVE_SECTOR_ADDRESS]), sizeof(uint32_t));

    if (NULL == driveToHandle->fileName ||
        selectedSector >= maxSectorCount)
    {
        dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
        return;
    }

    sectorSize = 1 << ddatBuf[DARDRIVE_D_SECTOR_SIZE_ADDRESS];

    addressOnFile  = sectorSize;
    addressOnFile *= selectedSector;
    addressOnFile += DARDRIVE_SECTORS_BEGIN;

    memcpy(&addressInMemory, &(ddatBuf[DARDRIVE_D_MEMORY_ADDRESS_ADDRESS]), sizeof(uint32_t));

    if (isRead)
    {
        fd = fopen(driveToHandle->fileName, "r");

        if (NULL == fd)
        {
            dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
            return;
        }

        if (-1 == fseek(fd, addressOnFile, SEEK_SET))
        {
            fclose(fd);
            dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
            return;
        }

        if (sectorSize < 128)
        {
            bytesRead = fread(buf, 1, sectorSize, fd);

            if (sectorSize != bytesRead)
            {
                if (0 == ferror(fd))
                {
                    fclose(fd);
                    dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
                    return;
                }

                memset(&(buf[bytesRead]), 0, sectorSize - bytesRead);
            }

            fclose(fd);

            if (false == dmi_writeToMemory(addressInMemory, sectorSize, buf))
            {
                dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
                return;
            }
        }
        else
        {
            for (uint32_t i = 0; i < sectorSize / 128; i++)
            {
                if (false == eofReached)
                {
                    bytesRead = fread(buf, 1, sectorSize, fd);

                    if (128 != bytesRead)
                    {
                        if (0 != ferror(fd))
                        {
                            fclose(fd);
                            dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
                            return;
                        }

                        memset(&(buf[bytesRead]), 0, 128 - bytesRead);
                        eofReached = true;
                    }

                    if (false == dmi_writeToMemory(addressInMemory, 128, buf))
                    {
                        fclose(fd);
                        dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
                        return;
                    }

                    if (eofReached)
                    {
                        memset(buf, 0, 128);
                    }
                }
                else
                {
                    if (false == dmi_writeToMemory(addressInMemory, 128, buf))
                    {
                        fclose(fd);
                        dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
                        return;
                    }
                }

                addressInMemory += 128;
            }

            fclose(fd);
        }
    }
    else
    {
        fd = fopen(driveToHandle->fileName, "r+");

        if (NULL == fd)
        {
            dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
            return;
        }

        if (-1 == fseek(fd, addressOnFile, SEEK_SET))
        {
            fclose(fd);
            dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
            return;
        }

        if (sectorSize < 128)
        {
            if (false == dmi_readFromMemory(addressInMemory, sectorSize, buf))
            {
                dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
                return;
            }

            if (sectorSize != fwrite(buf, 1, sectorSize, fd))
            {
                fclose(fd);
                dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
                return;
            }

            fclose(fd);
        }
        else
        {
            for (uint32_t i = 0; i < sectorSize / 128; i++)
            {
                if (false == dmi_readFromMemory(addressInMemory, 128, buf))
                {
                    fclose(fd);
                    dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
                    return;
                }

                if (128 != fread(buf, 1, sectorSize, fd))
                {
                    fclose(fd);
                    dmi_enqueueInterrupt(deviceId, it_storageEvent, DARDRIVE_D_FAILURE_OVERLAY);
                    return;
                }

                addressInMemory += 128;
            }

            fclose(fd);
        }
    }

    dmi_enqueueInterrupt(deviceId, it_storageEvent, 0);
}

static void st_teardown()
{
    for (uint8_t i = 0; i < DARDRIVE_MAX_DRIVE_COUNT; i++)
    {
        if (NULL != driveData[i])
        {
            st_freeDriveData(driveData[i]);
        }
    }
}

int st_initDeviceManager(void *arg)
{
    memcpy(&myThreadData, arg, sizeof(threadArg_t));
    free(arg);

    mtx_lock(myThreadData.mutex);

    // Request drive manager device
    driveManagerId = dmi_requestNewDevice(myThreadData.managerId, DARDRIVE_M_DDAT_SIZE);

    if (NEW_DEVICE_REQUEST_FAILED == driveManagerId ||
        false == dmi_writeDeviceData(driveManagerId,
                                     0,
                                     1,
                                     (uint8_t[]) {DARDRIVE_M_SPI}))
    {
        myThreadData.semaphore->wakeReason = dts_kill;
        cnd_signal(myThreadData.wakeCondition);
        mtx_unlock(myThreadData.mutex);
        return -1;
    }

    // Request devices for each initial drive provided
    for (uint8_t i = 0; i < MAX_INITIAL_STORAGE_DEVICES; i++)
    {
        if (NULL == myThreadData.externalFileInfo.storageFileNames[i])
        {
            break;
        }


        if (false == st_insertExistingDriveFile(myThreadData.externalFileInfo.storageFileNames[i]))
        {
            printf("Could not open %s as a dardrive\n\r", myThreadData.externalFileInfo.storageFileNames[i]);
            fflush(stdout);

            st_teardown();
            myThreadData.semaphore->wakeReason = dts_kill;
            cnd_signal(myThreadData.wakeCondition);
            mtx_unlock(myThreadData.mutex);
            return -1;
        }
    }

    // Bind the read handler
    dmi_bindHandleWrite(myThreadData.managerId, st_handleWrite);

    cnd_signal(myThreadData.wakeCondition);

    while (true)
    {
        cnd_wait(myThreadData.wakeCondition, myThreadData.mutex);

        switch (myThreadData.semaphore->wakeReason)
        {
            case dts_kill:
                return 0;
            case dts_handleWrite:
                if (myThreadData.semaphore->deviceId == driveManagerId)
                {
                    st_handleManagerWriteWake();
                }
                else
                {
                    st_handleDiskWriteWake(myThreadData.semaphore->deviceId);
                }
            case dts_continue:
                break;
        }

        myThreadData.semaphore->wakeReason = dts_continue;
    }
}