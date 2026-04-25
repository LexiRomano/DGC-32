# Interrupts

## Structure

Interrupts are represented by 2 bytes of information. The first byte is the inerrupt type and the second is a paremeter for the interrupt. Interrupts are enqueued in hardware reserved memory (see `memory-mapping.md`) by two registers: Interrupt Head (IH) and Interrupt Tail (IT). These represent the index within the interrupt queue where the head and tail are. The queue will grow upwards and wrap around once it reaches the end.

### Interrupt Table

The location where an interrupt will be handled is determined by a software defined table. This table is pointed to by the Interupt Table Location (IL) register. The interrupt table is a 256 * 4 = 1024 byte table. Each entry is 4 bytes, the location of where to handle the interrupt where its index within the table is the interrupt code that it represents. For example, if IL is set to 0x9000, when a timer event (0x06) occurs, the proccessor will look at address 0x9018:901B for the memory address to handle the interrupt at.

## Handling

When an interrupt is triggered, the address of the previously executing program is saved into the Interrupt Return Address (IR) register. The "interrupt in progress" status flag is then set. This prevents any other interrupts from occuring - including critical interrupts. It is then the responsibility of the interrupt handler to save the status of the proccessor and handle the interrupt. Once the interrupt has been handled, the handler may either finish the interrupt and return to the original execuing location stored in IR, or abandon the original program and continue execution elsehwere. In the first case, this is done by using the respective "finish interrupt" command. This will retreive the address in IR and continue as before the interrupt was triggered. In the case where the interrupt may have signaled some critical issue with the program being executed (such as a memory violation or a stack overflow), it may not be safe to return to the original exeution location. To prevent this, invoke the "resume interrupts" instruction to clear the "interrupt in progress" status flag and the processor will continue normal execution.

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
| 0x01            | critical space: <=34 bytes remaining* |

\* The critical space interrupt is only triggered once on the transition into the critical space state.

### Timer Event (0x06)

The parameter corresponds to the type of timer that triggered the event. More information may be found in the timer's device data.

### Storage Event (0x07)

This interrupt is triggered when one of the following events occur:

| Parameter Value | Meaning                              |
| --------------- | ------------------------------------ |
| 0x00            | transfer complete                    |
| 0x20            | transfer failed                      |
| 0x30:70         | reserved                             |
| 0x80:F0         | device specific                      |

The index of the storage device from the device registry is inserted in bits 4:0 of the parameter.

### Graphical Event (0x08)

The parameter corresponds to the index of the device table entry from where the interrupt came from. Further information will be described by the specification of the originating graphical device.

### Memory Violation (0x09)

This interrupt is triggered when protected memory is trying to be accessed.
