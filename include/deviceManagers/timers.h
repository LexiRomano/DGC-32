#ifndef __TIMERS_H__
#define __TIMERS_H__

#include <threads.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "deviceManagerInterface.h"

#define DIMER_DDAT_SIZE 4
#define DIMER_TPI              0x01
#define DIMER_CONFIG_ADDRESS   1
#define DIMER_DURATION_ADDRESS 2

#define DIMER_CONFIG_REPEAT_MASK    0b00000001
#define DIMER_CONFIG_INTERRUPT_MASK 0b00000010

int tm_initDeviceManager(void *arg);

#endif //__TIMERS_H__
