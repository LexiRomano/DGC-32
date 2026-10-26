# Motherboard

An interface to communicate with the motherboard

## Memory map

### Exposed registers

| device data address | description                         | Access |
| ------------------- | ----------------------------------- | ------ |
|                   0 | Motherboard type identifier = 0x01  |     RO |
|                   1 | Shutdown                            |     WO |
|                 2-3 | Year                                |     RO |
|                   4 | Month                               |     RO |
|                   5 | Day                                 |     RO |
|                   6 | Hour                                |     RO |
|                   7 | Minute                              |     RO |
|                   8 | Second                              |     RO |
|                 9-C | Uptime (seconds)                    |     RO |
