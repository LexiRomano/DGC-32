# Interrupts

## Structure

Interrupts are represented by 2 bytes of information. The first byte is the inerrupt type and the second is a paremeter for the interrupt. Interrupts are enqueued in hardware reserved memory (see `memory-mapping.md`) by two registers: Interrupt Head (IH) and Interrupt Tail (IT). These represent the index within the interrupt queue where the head and tail are. The queue will grow upwards and wrap around once it reaches the end.

## Interrupt Types

| Code      | Description           | 
| --------- | --------------------- |
| 0x00      | none                  |
| 0x01      | key pressed           |
| 0x02      | key released          |
| 0x03      | peripheral event      |
| 0x04      | device table update   |
| 0x05      | critical stack event* |
| 0x06      | timer event           |
| 0x07      | storage event         |
| 0x08      | graphical event       |
| 0x09      | memory violation*     |
| 0x0A:0x3F | reserved              |
| 0x40:0xFF | system calls          |

\* These events are considered critical. They triggered immediately and regardless of if interrupts are paused.

### Key Pressed (0x01)

The parameter corresponds to the ascii of the key that was pressed.

### Key Released (0x02)

The parameter corresponds to the ascii of the key that was released.

### Peripheral Event (0x03)

The parameter corresponds to the index of the device table entry from where the interrupt came from. Further information will be described by the specification of the originating peripheral.

### Device Table Update (0x04)

The parameter corresponds to the index of the device table entry that has changed. This can be either an addition or a deletion. 

### Critical Stack Event (0x05)

This interrupt is triggered when something in the stack experiences one of the following critical events:

| Parameter Value | Meaning                               |
| --------------- | ------------------------------------- |
| 0x00            | empty pop                             |
| 0x01            | critical space: <=64 bytes remaining* |

\* The critical space interrupt is only triggered once on the transition into the critical space state.

### Timer Event (0x06)

The parameter corresponds to the type of timer that triggered the event. More information may be found in the timer's device data.

### Storage Event (0x07)

This interrupt is triggered when one of the following events occur:

| Parameter Value | Meaning                              |
| --------------- | ------------------------------------ |
| 0x00            | transfer complete                    |
| 0x00            | transfer complete                    |
| 0x03:0x10       | reserved                             |
| 0x00            | device specific                      |

### Graphical Event (0x08)

The parameter corresponds to the index of the device table entry from where the interrupt came from. Further information will be described by the specification of the originating graphical device.

### Memory Violation (0x09)

This interrupt is triggered when protected memory is trying to be accessed.
