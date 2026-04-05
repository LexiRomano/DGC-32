# Memory Mapping

## Endianness

For performance reasons, the endianness of the DGC-32 matches the endianness of the computer hosting the emulator. For an x86 processor, that would be little endian.

## ROM: 0x00000000 : 0x00003FFF

Boot location at 0x00000000. Not much else to be said :P

## Hardware Reserved: 0x00004000 : 0x00005FFF

### Interrupt Queue: 0x00004000 : 0x000040FF

128 interrupts, 2 bytes each

## I/O: 0x00006000 : 0x00007FFF

### Device Registry: 0x00006000 : 0x000060FF

32 entries, 8 bytes each

```
Device registry table format
  0x01234567 89ABCDEF
    \__/\__/ \______/
     |   |       Device data location (4bytes)
     |   Device data size (2bytes)
     Device type (2bytes)
```

#### Device Types
| ID   | Type        |
| ---- | ----------- |
| 0x00 | none        |
| 0x01 | system info |
| 0x02 | storage     |
| 0x03 | graphical   |
| 0x04 | timer       |
| 0x05 | peripheral  |
| 0xFF | table end   |

### Device Data: 0x00006100 : 0x00007FFF

The contents of each device's data field will be defined by its respective specification. See doc/design/devices for included basic devices.

## General Use: 0x00008000 : 0xFFFFFFFF
