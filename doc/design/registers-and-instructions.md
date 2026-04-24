# Registers & Instructions

## Registers

Exposed registers:
- G0:G7 - 8 general purpose registers - 32bit
- OA - Offset register A - 32bit
- OB - Offset register B - 32bit
- OC - Offset register C - 32bit
- SB - Stack base - 32bit
- SS - Stack size - 16bit
- SP - Stack pointer - 16bit
- IL - Interrupt table location - 32bit
- Fl - Flags register - 8bit

Internal Registers:
- PC - Program counter - 32bit
- IR - Instruction register - 32bit
- AA - Argument augment - 32bit
- IA - Instruction augment - 8bit
- IR - Interrupt return address - 32bit
- IH - Interrupt head - 8bit
- IT - Interrupt tail - 8bit
- ST - Status register - 8bit


## Register Layouts

### Instruction Register
```
Form 1:
  11110000111100001111000011110000
  \______/\__/\__/\__/\__________/
     |     |   |   |     Unused (12b)
     |     |   |   Source 2 regsel (4b)
     |     |   Source 1 regsel (4b)
     |     Destination regsel (4b)
     Op code (8b)
```
```
Form 2:
  11110000111100001111000011110000
  \______/\__/\__/\______________/
     |     |   |        Unused (16b)
     |     |   Source regsel (4b)
     |     Destination regsel (4b)
     Op code (8b)
```
```
Form 3:
  11110000111100001111000011110000
  \______/\__/\__/\______________/
     |     |   |        Arg (16b)
     |     |   Source regsel (4b)
     |     Destination regsel (4b)
     Op code (8b)
```
```
Form 4:
  11110000111100001111000011110000
  \______/\__/\__________________/
     |     |        Unused (20b)
     |     Regsel (4b)
     Op code (8b)
```
```
Form 5:
  11110000111100001111000011110000
  \______/\__/\__________________/
     |     |        Arg (20b)
     |     Regsel (4b)
     Op code (8b)
```
```
Form 6:
  11110000111100001111000011110000
  \______/\______________________/
     |           Unused (24b)
     Op code (8b)
```
```
Form 7:
  11110000111100001111000011110000
  \______/\______________________/
     |           Arg (24b)
     Op code (8b)
```

### Status Register
```
  01010101
  \____/||
    |   |Interrupt in progress
    |   Interrupts susspended
    Unused
```


### Flags Register
```
  01010101
  \__/||||
   |  |||Overflow (V)
   |  ||Carry (C)
   |  |Negative (N)
   |  Zero (Z)
   Unused
```

#### Flag Conditions

| Code | Function            | Flags          |
| ---- | ------------------- | -------------- |
| 0000 | always              | none           |
| 0001 | equal               | Z==1           |
| 0010 | not equal           | Z==0           |
| 0011 | usig higher         | C==1 && Z==0   |
| 0100 | usig higher or same | C==1           |
| 0101 | usig lower or same  | C==0 \|\| Z==1 |
| 0110 | usig lower          | C==0           |
| 0111 | sig greater         | Z==0 && N==V   |
| 1000 | sig greater or same | N==V           |
| 1001 | sig less or same    | Z==1 \|\| N!=V |
| 1010 | sig less            | N!=V           |
| 1011 | negative            | N==1           |
| 1100 | positive or zero    | N==0           |
| 1101 | signed overflow     | V==1           |
| 1110 | no signed overflow  | V==0           |


## Instructions

### Regsel
- 0000:0111 - G0:G7
- 1000 - OA
- 1001 - OB
- 1010 - OC
- 1011 - SB
- 1100 - SS
- 1101 - SP
- 1110 - IL
- 1111 - FL

### Op codes

\* Multiple variants, see footnote

| Name | Description                                | Op select code | Form  | Arg augment? | Instruction augment? |
| ---- | ------------------------------------------ | -------------- | ----- | ------------ | -------------------- |
| NOOP | do nothing                                 | 0x00           | 6     |              |                      |
| INTR | interrupt utilities                        | 0xZ1*          | 4/6/7 |              |                      |
| LOAD | load from memory to a register             | 0x02           | 2     |              | Y                    |
| .    | .                                          | 0x03           | 4     | Y            | Y                    |
| STOR | store from a register to memory            | 0x04           | 2     |              | Y                    |
| .    | .                                          | 0x05           | 4     | Y            | Y                    |
| MOVE | move from one register to another          | 0x06           | 2     |              |                      |
| .    | move a value directly int a register       | 0x07           | 5     |              |                      |
| ADD  | add two registers together                 | 0x18           | 1     |              |                      |
| .    | add a value to a register                  | 0x19           | 3     |              |                      |
| SUB  | subtract one register from another         | 0x28           | 1     |              |                      |
| .    | subtract a value from a register           | 0x29           | 3     |              |                      |
| AND  | bitwise and two registers together         | 0x38           | 1     |              |                      |
| .    | bitwise and a register with a value        | 0x39           | 3     |              |                      |
| OR   | bitwise or two registers together          | 0x48           | 1     |              |                      |
| .    | bitwise or a register with a value         | 0x49           | 3     |              |                      |
| XOR  | bitwise xor two registers together         | 0x58           | 1     |              |                      |
| .    | bitwise xor a register with a value        | 0x59           | 3     |              |                      |
| NOT  | invert each bit in a register              | 0x68           | 2     |              |                      |
| BSLT | bitshift a register left and truncate      | 0x78           | 1     |              |                      |
| .    | .                                          | 0x79           | 3     |              |                      |
| BSRT | bitshift a register right and truncate     | 0x88           | 1     |              |                      |
| .    | .                                          | 0x89           | 3     |              |                      |
| BSLC | bitshift a retister left and carry around  | 0x98           | 1     |              |                      |
| .    | .                                          | 0x99           | 3     |              |                      |
| BSRC | bitshift a register right and carry around | 0xA8           | 1     |              |                      |
| .    | .                                          | 0xA9           | 3     |              |                      |
| COMP | compare two registers to each other        | 0x0A           | 2     |              |                      |
| .    | compare a register to a value              | 0x0B           | 4     | Y            |                      |
| BRNC | branch to another location on conditions   | 0xGC*          | 4     |              | Y                    |
| .    | .                                          | 0xGD*          | 6     | Y            | Y                    |
| STCK | stack utilities                            | 0xHE*          | 4/6   |              |                      |
| TERM | terminate the execution                    | 0xFF           | 6     |              |                      |

***\*BRNCH variants***: where G represents the condition required to jump (see flags register section).

***\*STCK variants***: where H=
- 0000 - push (form 4)
- 0001 - pop (form 4)
- 0010 - pushall (form 6)
- 0011 - popall (form 6)
- 0100 - peek (form 4)
- 0101 - return (form 6)

***\*INTR variants***: where Z=
- 0000 - susspend interrupts (form 6)
- 0001 - resume interrupts (form 6)
- 0010 - trigger interrupt (form 4)
- 0011 - trigger interrupt (form 7)
- 0100 - finish interrupt (form 6)
- 0101 - get interrupt parameter (form 4)

### Argument Augment

An argument augment is provided when a command requires more infromation than can be fit in the space remaining in the instruction for an argument. This is 8 bytes and is added after the instruction or the instruction augment if present.

### Instruction Augment

To support several different addressing modes, word sizes, and stack options, the instruction augment is added after a LOAD, STOR, or BRNC instruction. This is 1 byte and is added after the instruction but before the instruction augment if present.

```
  11001100
  ||\|\|\|
  || | | Offset reg select
  || | Word size
  || Word offset
  |Push return to stack / get absolute address
  Relative address
```

***Offset reg select***: Which offset registere to add to the location argument. Ignored if relative address is set. For all augmented commands.
- 00 - None
- 01 - OA
- 10 - OB
- 11 - OC

***Word size***: How many bytes to operate on. For LOAD and STOR only.
- 00 - 4 bytes
- 01 - 2 bytes
- 10 - 1 byte
- 11 - undefined

***Word offset***: How many bytes to shift to the left in the target register. For LOAD and STOR with word sizes less than 4 bytes only.
- 00 - no offset
- 01 - 1 byte left
- 10 - 2 bytes left
- 11 - 3 bytes left

***Push return to stack***: Whether or not to push the return location to the stack. For BRNC only.

***Get absolute address***: Instead of loading the contents of a memory location, this will instead load the address of that memory location. This takes into account the relative address or offset register. For LOAD only.

***Relative address***: Whether or not the specified address is relative to the location of the command. Supersedes offset reg select. For all augmented commands.
