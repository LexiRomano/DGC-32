# Derial

A d-grade serial interface

## Memory map

### Exposed registers

| device data address | description                         | Access |
| ------------------- | ----------------------------------- | ------ |
|                   0 | Peripheral type identifier = 0x53   |     RO |
|                   1 | Serial protocol identifier = 0x01   |     RO |
|                   2 | Inbound data                        |     RO |
|                   3 | Outbound data                       |     WO |
|                   4 | Flush                               |     WO |
|                   5 | Config                              |     RW |
|                   6 | Status                              |     RO |

### Internal

There is a 64 byte `outbound buffer` which will store all outbound data until it is flushed (either manually or automatically).

## Configuration

**Bit 0 - interrupt on flush complete:**

When this bit is set to 1, the `flush complete` interrupt will be triggered once a buffer flush is complete. Default to 0 (off).

**Bit 1 - auto flush:**

When this bit is set to 1, the `outbound buffer` will flush immediately upon the first byte being written to the `outbound data` register. If enabled when data is already in the `outbound buffer`, the buffer will not flush until another byte is written. Default to 0 (off).

**Bit 2 - interrupt on data received:**

When this bit is set to 1, the `data received` interrupt will be triggered when there is data available to be read in the `inbound data` register. Defaults to 1 (on).

## Status

**Bit 0 - buffer not empty:**

This bit is set to 1 when 1 or more bytes are waiting to be transmitted in the `outbound buffer`. 

**Bit 1 - buffer full:**

This bit is set when the `outbound buffer` has reached maximum capacity. It is cleared when a flush has completed.

**Bit 2 - inbound data ready:**

This bit is set to 1 when a byte has been received and is waiting in the `inbound data` register. It is cleared when the `inbound data` register is read.

**Bit 3 - missed received byte:**

This bit is set to 1 when two consecutive bytes have been receivedwithout the `inbound data` register being read. It is cleared when the `inbound data` register is read.

## Receiving data

Data received will be placed in the `inbound data` register and the `inbound data ready` status bit will be set to 1. If the `interrupt on data received` config bit is set, the `data received` interrupt will be triggered. This data must then be read from the `inbound data` register before another byte is received, or else the data will be lost and the `missed received byte` will be set to 1. There is a minimum 10 microsecond delay between consecutively received characters.

## Sending data

### Auto flush

To send data, write a single byte to the `outbound data` register. This byte will then be automatically flushed. After the flush is complete, the `buffer not empty` and `buffer full` status bits will be cleared and the `flush complete` interrupt will be triggered if it is enabled by the `interrupt on flush complete` config bit.

### Manual flush

To send data, write sequential bytes to the `outbound data` register. These written bytes will be stored in the `outbound buffer`. The buffer must then be flushed by writing any byte to the flush register. After the flush is complete, the `buffer not empty` and `buffer full` status bits will be cleared and the `flush complete` interrupt will be triggered if it is enabled by the `interrupt on flush complete` config bit.

## Interrupts

***Data received:*** 0b00dddddd where d is the device ID of the device

***Flush complete:*** 0b01dddddd where d is the device ID of the device
