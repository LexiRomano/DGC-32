# High Level Design

## Intruduction

This document describes the broad design requirements of the D-Grade Computer (DGC-32). Within its containing folder, there are additional documents describing certain components and systems in greater detail.

## Design Goals

The DGC-32 will be designed with flexibility and expandability in mind. The architecture must not get in the way of any prospective software developer designing for this architecture. Learning from the F-Grade Computer (FGC-16), the DGC-32 must have many different ways to perform tasks in a resource efficient manner. It shall be designed with the end use case of an operating system in mind, providing a variety of supportive features to enable control and performance.

## Features

### Registers

The DGC-32 shall have eight general purpose registers (G0 through G7). In addition, there will be several utility registers accessable by software for addressing modes, stack description, and interrupts as described in this document.

### Addressing Modes

The DGC-32 shall have three different addressing modes as follows:

- Global/physical addressing
- Relative to command
- With an offset from one of three differnt offset registers (OA, OB, and OC)
    - Different offset registers should correspond to differnt permission levels such as kernel/user

### Stack

The DGC-32 shall have a stack which can be specified by software using the Stack Base (SB) address register, the Stack Size (SS)register, and the Stack Pointer (SP) register. Software shall be able to push, pop, pushall, popall, peek, and return. The hardware will manage these instructions according to the registers noted above.

### Interrupts

The DGC-32 shall have both hardware and software interrupts. The locations where these interrupts will be handled are identified by a table pointed to by the Interrupt Table Location (IL) register. Software shall be able to susspend, resume, trigger, and finish handling interrupts.

### Input/Output

The DGC-32 shall have memory-mapped I/O, including the following features:

- Disk read/writes
- Keyboard input, maybe with mouse information
- Two graphical modes: simple text and rendering
- A timer chip
- (snowball's chance in hell) some audio

The desciption of these I/O functions shall done in a manner condusive to expansion by other developers to add more devices and peripherals.

## Implementation

### Deployment

The DGC-32 as described in these documents shall be realized in the form of an emulator. This emulator shall be deployed in a self-contained executable format. This shall enable a user to add to a $PATH folder and execute in any arbitrary location. The emulator shall take in all required progam data (rom and disk files) as arguments at launch or search for default file names if none are explicity mentioned. 

### Testing

The DGC-32 shall have two forms of testing; self test and user test.

#### Self Test

Self test shall allow the emulator developer (Lexington Romanicus II) to verify there has been no regression to the functionality of the emulator. This may take the form of hard-coded tests in the executable (probably easier) or utilize external testing files (better long term).

#### User Test

User test shall allow the program developer (Probably also Lexington Romanicus II) to test out the functionality of their software written for the DGC-32. This may happen with different modes, include stepping through each instruction or adding/enabling special breakpoint instructions.
