#ifndef __DUMMY_DEVICE_MANAGER_H__
#define __DUMMY_DEVICE_MANAGER_H__

#include <threads.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "deviceManagerInterface.h"

int initDummyDeviceManager(void *arg);

#endif //__DUMMY_DEVICE_MANAGER_H__