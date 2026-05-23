# Dardrive

A D-grade hard drive

## Drive Manager

One device entry will be made for the drive manager. This is a workaround in the emulator to be able to simulate physically inserting and removing drives. This would not exist in real hardware (if real hardware existed)

### Memory map

| device data address | description                         | Access |
| ------------------- | ----------------------------------- | ------ |
|                   0 | Storage protocol identifier = 0x01  |     RO |
|                   1 | Drive ID                            |     RW |
|                   2 | Sector size (2's exponent)          |     RW |
|                   3 | Sector count (2's exponent)         |     RW |
|                 4-7 | Name pointer                        |     RW |
|                   8 | Initiate                            |     RW |
|                   9 | Config                              |     RW |
|                   A | Status                              |     RO |

### Configuration

**Bit 0 - enable interrupts**

When this bit is set to 1, interrupts will be triggered when a transaction completes. Default to 1 (enabled)

### Status

**Bit 0 - transaction in progress**

This bit is set when a transaction is initiated. It is cleared when a transaction completes.

**Bit 1 - transaction failed**

This bit is set when a transaction fails. It is cleared when a new transaction is initiated.

### Inserting a new drive

- Wait for any previous transactions to complete.
- Store a pointer to the null-terminated file name of the new drive file (outside the emulator) in the name pointer register. A maximum of 254 bytes plus the null terminator will be read, otherwise the creation will fail.
- Configure the sector size register (eg. 9 for 512 bytes).
- Configure the sector count register (eg. 16 for 65536 sectors).
- Write a 0 to the initiate register.

### Inserting an existing drive

- Wait for any previous transactions to complete.
- Store a pointer to the null-terminated file name of the existing drive file (outside the emulator) in the name pointer register. A maximum of 254 bytes plus the null terminator will be read, otherwise scan will fail.
- Write a 1 to the initiate register.

### Removing a drive

- Wait for any previous transactions to complete.
- Store the device ID of the drive to be removed in the drive ID register.
- Write a 2 to the initiate register.

### Interrupts

**Only applicable if the `enable interrupts` config bit is set**

Upon succesful drive insertion/removal, an interrupt is generated with the parameter corresponding to the device ID of the drive manager or'd with the device ID of the inserted/removed drive, shifted 3 bits left. The motherboard's device table update interrupt will occur before this interrupt, but in the case of an inserted drive, it will not be ready for use until this interrupt is enqueued.

Upon unsuccessful drive insertion/removal, an interrupt is generated with the parameter corresponding to the device ID of the drive manager or'd with 0xC0.

### Emulation

#### Drive file format

The manager's storage protocol identifier, drive storage protocol identifier, sector size, and sector count data is stored at bytes 0, 1, 2, and 3 of the drive file. The sectors then begin at byte 4. If the file is smaller than the size of the emulated drive, all unspecified space is assumed to be filled with zeros. The drive file will then grow once writes occur to said unspecified space. To create a new drive before starting the emulator, use the following source code in `dssembly`:

```
.set 1 1  // Manager storage protocol identifier, constant
.set 1 2  // Drive storage protocol identifier, constant
.set 1 9  // Sector size,  9  -> 512
.set 1 16 // Sector count, 16 -> 65536
```

#### Boot behaviour

If any of the file names specified to the emulator do not conform to the drive file format, the device manager will fail to initialize and the emulator will not launch.

## Individual Drives

A device entry will be created for each individual drive that exists on the system.

### Memory map

| device data address | description                         | Access |
| ------------------- | ----------------------------------- | ------ |
|                   0 | Storage protocol identifier = 0x02  |     RO |
|                   1 | Sector size (2's exponent)          |     RO |
|                   2 | Sector count (2's exponent)         |     RO |
|                 3-6 | Memory address                      |     RW |
|                 7-A | Drive sector                        |     RW |
|                   B | Initiate                            |     RW |
|                   C | Config                              |     RW |
|                   D | Status                              |     RO |

### Configuration

**Bit 0 - enable interrupts**

When this bit is set to 1, interrupts will be triggered when a transaction completes. Default to 1 (enabled)

### Status

**Bit 0 - transaction in progress**

This bit is set when a transaction is initiated. It is cleared when a transaction completes.

**Bit 1 - transaction failed**

This bit is set when a transaction fails. It is cleared when a new transaction is initiated.

### Drive read procedure

- Wait for any previous drive transactions to complete
- Write the location in memory where the sector will be loaded into to the memory address register 
- Write which sector will be loaded to the drive sector register
- Write a 0 to the initiate register

### Drive write procedure

- Wait for any previous drive transactions to complete
- Write the location in memory where the data to be written resides to the memory address register
- Write which sector will to be written to to the drive sector register.
- Write a 1 to the initiate register

### Interrupts

**Only applicable if the `enable interrupts` config bit is set**

Upon successful completion of a drive transaction, an interrupt will be triggered with the parameter corresponding to the device ID of the drive that just completed.

Upon a failed drive transaction, an interrupt will be triggered with the parameter corresponding to the device ID of the drive, or'd with 0xC0.
