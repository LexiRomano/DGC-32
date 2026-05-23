#ifndef __PERIPHERALS_H__
#define __PERIPHERALS_H__

#include <threads.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "deviceManagerInterface.h"

// Dboard
#define DBOARD_DDAT_SIZE 2
#define DBOARD_PERIPHERAL_TYPE_ID 0x4B
#define DBOARD_KEYBOARD_PROT_ID   0x01

#define DBOARD_SCANCODE_A             0x00
#define DBOARD_SCANCODE_B             0x01
#define DBOARD_SCANCODE_C             0x02
#define DBOARD_SCANCODE_D             0x03
#define DBOARD_SCANCODE_E             0x04
#define DBOARD_SCANCODE_F             0x05
#define DBOARD_SCANCODE_G             0x06
#define DBOARD_SCANCODE_H             0x07
#define DBOARD_SCANCODE_I             0x08
#define DBOARD_SCANCODE_J             0x09
#define DBOARD_SCANCODE_K             0x0A
#define DBOARD_SCANCODE_L             0x0B
#define DBOARD_SCANCODE_M             0x0C
#define DBOARD_SCANCODE_N             0x0D
#define DBOARD_SCANCODE_O             0x0E
#define DBOARD_SCANCODE_P             0x0F
#define DBOARD_SCANCODE_Q             0x10
#define DBOARD_SCANCODE_R             0x11
#define DBOARD_SCANCODE_S             0x12
#define DBOARD_SCANCODE_T             0x13
#define DBOARD_SCANCODE_U             0x14
#define DBOARD_SCANCODE_V             0x15
#define DBOARD_SCANCODE_W             0x16
#define DBOARD_SCANCODE_X             0x17
#define DBOARD_SCANCODE_Y             0x18
#define DBOARD_SCANCODE_Z             0x19
#define DBOARD_SCANCODE_0             0x1A
#define DBOARD_SCANCODE_1             0x1B
#define DBOARD_SCANCODE_2             0x1C
#define DBOARD_SCANCODE_3             0x1D
#define DBOARD_SCANCODE_4             0x1E
#define DBOARD_SCANCODE_5             0x1F
#define DBOARD_SCANCODE_6             0x20
#define DBOARD_SCANCODE_7             0x21
#define DBOARD_SCANCODE_8             0x22
#define DBOARD_SCANCODE_9             0x23
#define DBOARD_SCANCODE_MINUS         0x24
#define DBOARD_SCANCODE_EQUALS        0x25
#define DBOARD_SCANCODE_OPEN_BRACKET  0x26
#define DBOARD_SCANCODE_CLOSE_BRACKET 0x27
#define DBOARD_SCANCODE_BACKSLASH     0x28
#define DBOARD_SCANCODE_SEMICOLON     0x29
#define DBOARD_SCANCODE_QUOTE         0x2A
#define DBOARD_SCANCODE_COMMA         0x2B
#define DBOARD_SCANCODE_PERIOD        0x2C
#define DBOARD_SCANCODE_SLASH         0x2D
#define DBOARD_SCANCODE_GRAVE_ACCENT  0x2E
#define DBOARD_SCANCODE_LCTRL         0x2F
#define DBOARD_SCANCODE_LSHIFT        0x30
#define DBOARD_SCANCODE_LALT          0x31
#define DBOARD_SCANCODE_TAB           0x32
#define DBOARD_SCANCODE_RCTRL         0x33
#define DBOARD_SCANCODE_RSHIFT        0x34
#define DBOARD_SCANCODE_RALT          0x35
#define DBOARD_SCANCODE_ENTER         0x36
#define DBOARD_SCANCODE_BACKSPACE     0x37
#define DBOARD_SCANCODE_DELETE        0x38
#define DBOARD_SCANCODE_SPACE         0x39
#define DBOARD_SCANCODE_UP            0x3A
#define DBOARD_SCANCODE_DOWN          0x3B
#define DBOARD_SCANCODE_LEFT          0x3C
#define DBOARD_SCANCODE_RIGHT         0x3D
#define DBOARD_SCANCODE_ESC           0x3E

#define DBOARD_SCANCODE_NONE          0xFF

// Derial
#define DERIAL_DDAT_SIZE                  7
#define DERIAL_PERIPHERAL_TYPE_ID         0x53
#define DERIAL_SERIAL_PROT_ID             0x01
#define DERIAL_INBOUND_ADDRESS            2
#define DERIAL_OUTBOUND_ADDRESS           3
#define DERIAL_FLUSH_ADDRESS              4
#define DERIAL_CONFIG_ADDRESS             5
#define DERIAL_STATUS_ADDRESS             6

#define DERIAL_CONFIG_INT_ON_FLUSH_MASK   0b00000001
#define DERIAL_CONFIG_AUTO_FLUSH_MASK     0b00000010
#define DERIAL_CONFIG_INT_ON_RCV_MASK     0b00000100

#define DERIAL_STATUS_BUF_NOT_EMPTY_MASK  0b00000001
#define DERIAL_STATUS_BUF_FULL_MASK       0b00000010
#define DERIAL_STATUS_INBOUND_READY_MASK  0b00000100
#define DERIAL_STATUS_RCV_MISSED_MASK     0b00001000

#define DERIAL_OUTBOUND_BUF_SIZE          64

#define DERIAL_RECEIVED_INT_OVERLAY       0b00000000
#define DERIAL_FLUSH_COMPLETE_INT_OVERLAY 0b01000000

int pr_initDeviceManager(void *arg);

#endif //__PERIPHERALS_H__