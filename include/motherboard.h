#ifndef __MOTHERBOARD_H__
#define __MOTHERBOARD_H__

#include <threads.h>

#include "dgc32.h"
#include "deviceManagerInterface.h"

// Add devices here:
#include "deviceManagers/dummyDeviceManager.h"

#define NUM_DEVICE_MANAGERS 5

void mb_initDevices();
void mb_teardownDevices();

#endif //__MOTHERBOARD_H__