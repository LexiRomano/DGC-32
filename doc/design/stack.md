# Stack

## Structure

The stack's geometry is defined by two registers: the Stack Base (SB) and the Stack Size (SS) registers. In addition, the top of the stack is kept track of with the Stack Pointer (SP) register. All three of these registers are accessable by regsel. The SP keeps track of the index of the next available space at the top of the stack, relative to SB, so it is zero when empty. This means that it is possible to expand or move the size of the stack while it is running if done carefully. The stack grows upwards into higher value memory addresses from SB.

### Interrupts

In the case of a pop on an empty stack or the remaining size of the stack reaching <=34 bytes, an interrupt will be triggered. See `interrupts.md` for more information.

## Instructions

### Push (0x0E)

Pushes one register to the stack. The register is selected using the regsel in the instruction (form 4).

### Pop (0x1E)

Pops one value from the stack into a register. The register is selected using the regsel in the instruction (form 4).

### Pushall (0x2E)

Pushes all general purpose registers plus the flags register to the stack. This is done in the order of G0, G1, G2, ... G7, FL. No arguments.

### Popall (0x3E)

Pops values from the stack into all general purpose registers plus the flags register. This is done in the order of FL, G7, G6, ... G0. No arguments.

### Peek (0x4E)

Peek will read the top value of the stack into a selected register without removing it. The register is selected using the regsel in the instruction (form 4).

### Return (0x5E)

Will jump to the location on the stack and remove it. Used in conjunction with a BRNC command which used the "push return to stack" flag in the instruction augment. No arguments.
