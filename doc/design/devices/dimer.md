# Dimer

A d-grade timer

## Memory map

| device data address | description                         | Access |
| ------------------- | ----------------------------------- | ------ |
|                   0 | Timer protocol identifier = 0x01    |     RO |
|                   1 | Configuration                       |     RW |
|                 2-3 | Duration                            |     RW |

## Configuration

**Bit 0 - Repeat:**

When this bit is set, the timer will automatically restart after completing using the contents of the duration register (repeat mode). Otherwise, it will stop and clear the duration register (one-shot mode). Default to 0.

**Bit 1 - Interrupt:**

When this bit is set, an interrupt will be triggered whenever the timer completes. This is necessary to tel when a repeating timer completes, but is not enforced. Default to 0.

## Starting

To start the timer, write the duration of the timer (in microseconds) to the duration register. The timer will then run according to the configuration described above. If a one-shot timer is already running, this will have no effect. If a repeat timer is running, the next restart of the timer will use the new duration.

## Stopping

To stop the timer, write all zeros to the duration register.
