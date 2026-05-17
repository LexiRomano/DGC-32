#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <threads.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "deviceManagerInterface.h"

typedef struct
{
    char   *fileName;
    uint8_t deviceId;
    uint8_t sectorSize;
    uint8_t sectorCount;
} driveData_t;

// Dardrive generic
#define DARDRIVE_MAX_DRIVE_COUNT 16
#define DARDRIVE_SECTORS_BEGIN   4

// Dardrive drive manager
#define DARDRIVE_M_SPI       0x01
#define DARDRIVE_M_DDAT_SIZE 0x9

#define DARDRIVE_M_DRIVE_ID_ADDRESS     0x1
#define DARDRIVE_M_SECTOR_SIZE_ADDRESS  0x2
#define DARDRIVE_M_SECTOR_COUNT_ADDRESS 0x3
#define DARDRIVE_M_NAME_POINTER_ADDRESS 0x4
#define DARDRIVE_M_INITIATE_ADDRESS     0x8

#define DARDRIVE_M_INITIATE_NEW      0
#define DARDRIVE_M_INITIATE_EXISTING 1
#define DARDRIVE_M_INITIATE_REMOVE   2

#define DARDRIVE_M_MAX_FILE_NAME_SIZE 254

#define DARDRIVE_M_FAILURE_OVERLAY 0xC0

// Dardrive individual drives
#define DARDRIVE_D_SPI       0x02
#define DARDRIVE_D_DDAT_SIZE 0xC

#define DARDRIVE_D_SECTOR_SIZE_ADDRESS    0x1
#define DARDRIVE_D_SECTOR_COUNT_ADDRESS   0x2
#define DARDRIVE_D_MEMORY_ADDRESS_ADDRESS 0x3
#define DARDRIVE_D_DRIVE_SECTOR_ADDRESS   0x7
#define DARDRIVE_D_INITIATE_ADDRESS       0xB

#define DARDRIVE_D_INITIATE_READ  0
#define DARDRIVE_D_INITIATE_WRITE 1

#define DARDRIVE_D_FAILURE_OVERLAY 0xC0

int st_initDeviceManager(void *arg);

#endif //__STORAGE_H__