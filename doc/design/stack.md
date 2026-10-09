# Stack

## Structure

The stack's geometry is defined by two registers: the Stack Base (SB) and the Stack Size (SS) registers. In addition, the top of the stack is kept track of with the Stack Pointer (SP) register. All three of these registers are accessable by regsel. The SP keeps track of the index of the next available space at the top of the stack, relative to SB, so it is zero when empty. This means that it is possible to expand or move the size of the stack while it is running if done carefully. The stack grows upwards into higher value memory addresses from SB.

## Interrupts

In the case of a pop on an empty stack or the remaining size of the stack reaching <=8 bytes, an interrupt will be triggered. See `interrupts.md` for more information.
