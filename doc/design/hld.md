# High Level Design

## Features

### General purpose registers

The DGC-32 shall have 8 general purpose registers.

### Addressing modes

The DGC-32 shall have several different addressing modes, including a mixutre of the following:

- Global/physical addressing
- Relative to command
- With an offset from several differnt offset registers
    - Different offset registers should correspond to differnt permission levels such as kernel/user

### Stack

The DGC-32 shall have a stack which can be specified by software using a base address register and a size register. Software shall be able to init, push, pop, pushall, popall, peek, and return with hardware managing according to the exposed registers.

### Interrupts

The DGC-32 shall have both hardware and software interrupts wich can be defined by a table specified by a location register. Software shall be able to susspend, resume, and trigger interrupts.

### I/O

The DGC-32 shall have memory-mapped I/O, including the following features:

- Disk read/writes
- Keyboard input, maybe with mouse information
- Two graphical modes: simple text and rendering
- A timer chip
- (snowball's chance in hell) some audio

## Deployment

The DGC-32 shall be realized in a portable executable format useable in any location by adding it to $PATH if desired. It shall take certain formatted files such as .rom and .dsk. It shall scan for default file names if none are provided.

## Testing

The DGC-32 shall have two forms of testing; self test and user test.

### Self test

Self test shall allow the emulator developer (Lexington Romanicus II) to regress emulator functionality. This may take the form of hard-coded tests in the executable (probably easier) or utilize external testing files (better long term).

### User test

Suer test shall allow the program developer (Probably also Lexington Romanicus II) to test out the functionality of their software written for the DGC-32. This may happen with different modes, include stepping through each instruction or adding/enabling special breakpoint instructions