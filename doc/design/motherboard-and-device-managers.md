# Motherboard & Device Managers

## Motherboard Responsibilities

The motherboard shall interface with devices through device managers (described below). The motherboard shall be responsible for managing the connection and disconnection of all devices through requests made by each device manager. The motherboard shall be responsible for mapping all proccessor read/writes to I/O from memory space to the respective devices and pass them along to the device managers. This will abstract the logical memory address provided by the proccessor, showing only the device id being accessed and which device registers are involved in the access. The motherboard shall be responsible for managing the device registry by adding requested devices and deleting removed devices. The motherboard shall provide an interface for the devices to access general use memory.

## Device Manager Responsibilities

A device manager shall be responsible for requesting an additonal device registry when required. There shall be only one device manager per type of device (storage, graphical, etc.), but each device manager may request multiple device registry entries. For example, there can be many storage devices, but they are all managed through the singular storage device manager. Devices and device managers have direct access to general use memory, but are forbidden from writing to it without explicit command from the processor. Devices are, however, free to enqueue any interrupts related to its device type. For example, a peripheral device may enqueue peripheral event and key press/release interrupts, but cannot enqueue any other types of interrupt such as timer or storage.

## Device Registry

The device registry is a read-only region of memory as seen from the processor. It is managed entirely by the motherboard with changes being requested from device managers. It is shown as a 256-byte table with 32 entries, 8 bytes each. As devices are added, the table end marker gets moved towards the end of the registry space. When devices are removed, they are replaced with holes signifying no device represented by that entry.Future added devices may then fill that hole.  In the case where the last device in the registry is removed, the table end marker may be moved back to replace it. Each change to the device registry (excluding the initial discovery phase on boot) will be signified by a corresponding interrupt (see `interrupts.md`).

```
Device registry table format
  0x01234567 89ABCDEF
    \/\/\__/ \______/
    | |  |       Device data location (4bytes)
    | |  Device data size (2bytes)
    | Device type (1byte)
    Unused (1byte)
```

### Device Types

| ID   | Type        |
| ---- | ----------- |
| 0x00 | none/empty  |
| 0x01 | motherboard |
| 0x02 | storage     |
| 0x03 | graphical   |
| 0x04 | timer       |
| 0x05 | peripheral  |
| 0xFF | table end   |

***Note:*** the `motherboard` device type is an alias used by the motherboard to provide access to its information, this device should always exist with exactly one instance.
