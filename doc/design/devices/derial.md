# Derial

A d-grade serial interface

## Memory map

| device data address | description                         | Access |
| ------------------- | ----------------------------------- | ------ |
|                   0 | Peripheral type identifier = 0x53   |     RO |
|                   1 | Serial protocol identifier = 0x01   |     RO |
|                   2 | Data inbound                        |     RO |
|                   3 | Data outbound                       |     WO |

## Input

**Data outbound:** The byte written to this register will be output from the serial port.

## Output

**Data inbound:** Contains the last byte received from the serial port.

## Interrupts

When a new byte is received, the serial port will trigger a peripheral event interrupt after placing the data in the inbound buffer.
