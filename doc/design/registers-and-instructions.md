# Registers & Instructions

## Registers

Exposed registers:
- G0:G7 - 8 general purpose registers - 32bit
- OA - Offset register A - 32bit
- OB - Offset register B - 32bit
- OC - Offset register C - 32bit
- SB - Stack base - 32bit
- SS - Stack size - 16bit
- IL - Interrupt table location - 32bit

Internal Registers:
- PC - Program counter - 32bit
- IR - Instruction register - 32bit
- IA - Instruction augment - 8bit
- SP - Stack pointer - 16bit
- Fl - Flags register - 4bit

## Instruction & Flags Register Layout

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

### Flags register
```
  0101
  ||||
  |||Overflow
  ||Carry
  |Negative
  Zero
```

## Instructions

### Regsel
- 0000:0111 - G0:G7
- 1000 - OA
- 1001 - OB
- 1010 - OC
- 1011 - SB
- 1100 - SS
- 1101 - IL

### Op codes

\* Multiple variants, see footnote

\+ Loads instruction augment, see section.

| Name | Op select code | Form | Arg augment? |
| ---- | -------------- | ---- | ------------ |
| NOOP | 0x00           | 6    | N            |
| LOAD | 0x02+          | 5    | N            |
| .    | 0x03+          | 4    | Y            |
| STOR | 0x04+          | 5    | N            |
| .    | 0x05+          | 4    | Y            |
| MOVE | 0x06           | 2    | N            |
| .    | 0x07           | 3    | N            |
| ADD  | 0x18           | 1    | N            |
| .    | 0x19           | 3    | N            |
| SUB  | 0x28           | 1    | N            |
| .    | 0x29           | 3    | N            |
| AND  | 0x38           | 1    | N            |
| .    | 0x39           | 3    | N            |
| OR   | 0x48           | 1    | N            |
| .    | 0x49           | 3    | N            |
| XOR  | 0x58           | 1    | N            |
| .    | 0x59           | 3    | N            |
| NOT  | 0x68           | 2    | N            |
| BSLT | 0x78           | 1    | N            |
| .    | 0x79           | 3    | N            |
| BSRT | 0x88           | 1    | N            |
| .    | 0x89           | 3    | N            |
| BSLC | 0x98           | 1    | N            |
| .    | 0x99           | 3    | N            |
| BSRC | 0xA8           | 1    | N            |
| .    | 0xA9           | 3    | N            |
| JUMP | 0x0A+          | 4    | N            |
| .    | 0x0B+          | 6    | Y            |
| BRNC | 0xGC*+         | 4    | N            |
| .    | 0xGD*+         | 6    | Y            |
| STCK | 0xHE*          | 4/6  | N            |
| TERM | 0xFF           | 6    | N            |

***\*BRNCH variants***: where G represents the flags required to trigger the branch

***\*STCK variants***: where H=
- 0000 - push (form 4)
- 0001 - pop (form 4)
- 0010 - pushall (form 6)
- 0011 - popall (form 6)
- 0100 - peek (form 4)
- 0101 - init (form 4)
- 0110 - return (form 6)


### Instruction Augment

To support several different addressing modes, word sizes, and stack options, the instruction augment is added after a LOAD, STOR, JUMP, or BRNC instruction.

```
  11001100
  ||\|\|\|
  || | | Offset reg select
  || | Word size
  || Word offset
  |Push return to stack
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

***Push return to stack***: Whether or not to push the return location to the stack. For JUMP and BRNC only.

***Relative address***: Whether or not the specified address is relative to the location of the command. Supersedes offset reg select. For all augmented commands.