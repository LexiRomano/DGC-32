# Derial

A d-grade serial interface

## Memory map

| device data address | description                         | Access |
| ------------------- | ----------------------------------- | ------ |
|                   0 | Peripheral type identifier = 0x53   |     RO |
|                   1 | Serial protocol identifier = 0x01   |     RO |
|                   2 | Inbound data                        |     RO |
|                   3 | Outbound data                       |     WO |
|                   4 | Flush                               |     WO |

## Receiving data

Data received will be placed in the inbound data register and an interrupt will be triggered (see below). This data must then be read before it is replaced by the next byte.

## Sending data

To send data, write sequential bytes to the outbound data buffer. These written bytes will be stored in an internal buffer up to a maximum of 64 bytes, after which written data will not be stored. The buffer must then be flushed by writing any byte to the flush register. After the flush is complete, an interrupt will be triggered (see below)

## Interrupts

***Received data:*** 0b00dddddd where d is the device ID of the device

***Flush complete:*** 0b01dddddd where d is the device ID of the device
