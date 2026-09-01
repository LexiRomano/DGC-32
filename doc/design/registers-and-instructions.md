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
- OP - Op code register - 8bit
- R1 - Regsel argument register 1 - 8bit
- R2 - Regsel argument register 2 - 8bit
- AG - Instrucion argument register - 32bit
- IR - Interrupt return address - 32bit
- CI - Current interrupt - 16bit
- IH - Interrupt head - 8bit
- IT - Interrupt tail - 8bit
- ST - Status register - 8bit


## Register Layouts

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

### Instruction Forms

Hex digits replaced with a `z` are don't-cares

```
Form 1:
  0xAB -> Op code
```
```
Form 2:
  0xAB -> Op code
  0x1z -> Single regsel
```
```
Form 3:
  0xAB -> Op code
  0x12 -> Dual regsel
```
```
Form 4:
  0xAB -> Op code
  0x12 -> Dual regsel
  0x3z -> Single regsel
```
```
Form 5:
  0xAB -> Op code
  0xCz -> Flag condition
```
```
Form 6:
  0xAB -> Op code
  0x1C -> Single regsel / Flag condition
```
```
Form 7:
  0xAB -> Op code
  0x12 -> Dual regsel
  0xCz -> Flag condition
```
```
Form 8:
  0xAB -> Op code
  0x12 -> Dual regsel
  0x3C -> Single regsel / Flag condition
```

Form numbers followed by `c`, `h`, and `w` have an 8bit, 16bit, or 32bit argument respectively appended. For example:

```
Form 2h:
  0xAB   -> Op code
  0xX1   -> Single regsel
  0x1234 -> Instruction argument
```

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

#### 0x00: noop
- Form 1
- Does nothing

#### 0x01: move register
- Form 3
- Moves a value from the second regsel to the first regsel

#### 0x02: move immediate 32
- Form 2w
- Moves the word argument into the regsel

#### 0x03: move immediate 16
- Form 2h
- Moves the short argument into the regsel

#### 0x04: move immediate 16 sig
- Form 2h
- Moves the short argument into the regsel with sign extension

#### 0x05: move immediate 8
- Form 2c
- Moves the char argument into the regsel

#### 0x06: move immediate 8 sig
- Form 2c
- Moves the char argument into the regsel with sign extension

#### 0x07: load 32 register abs
- Form 3
- Loads the word pointed to in memory by the second regsel into the first regsel
- Absolute addressing mode

#### 0x08: load 32 register PC rel
- Form 3
- Loads the word pointed to in memory by the second regsel into the first regsel
- Program counter relative addressing mode

#### 0x09: load 32 register OA rel
- Form 3
- Loads the word pointed to in memory by the second regsel into the first regsel
- Offset register A relative addressing mode

#### 0x0A: load 32 register OB rel
- Form 3
- Loads the word pointed to in memory by the second regsel into the first regsel
- Offset register B relative addressing mode

#### 0x0B: load 32 register OC rel
- Form 3
- Loads the word pointed to in memory by the second regsel into the first regsel
- Offset register C relative addressing mode

#### 0x0C: load 16 register abs
- Form 3
- Loads the short pointed to in memory by the second regsel into the first regsel
- Absolute addressing mode

#### 0x0D: load 16 register PC rel
- Form 3
- Loads the short pointed to in memory by the second regsel into the first regsel
- Program counter relative addressing mode

#### 0x0E: load 16 register OA rel
- Form 3
- Loads the short pointed to in memory by the second regsel into the first regsel
- Offset register A relative addressing mode

#### 0x0F: load 16 register OB rel
- Form 3
- Loads the short pointed to in memory by the second regsel into the first regsel
- Offset register B relative addressing mode

#### 0x10: load 16 register OC rel
- Form 3
- Loads the short pointed to in memory by the second regsel into the first regsel
- Offset register C relative addressing mode

#### 0x11: load 16 sig register abs
- Form 3
- Loads the short pointed to in memory by the second regsel into the first regsel with sign extension
- Absolute addressing mode

#### 0x12: load 16 sig register PC rel
- Form 3
- Loads the short pointed to in memory by the second regsel into the first regsel with sign extension
- Program counter relative addressing mode

#### 0x13: load 16 sig register OA rel
- Form 3
- Loads the short pointed to in memory by the second regsel into the first regsel with sign extension
- Offset register A relative addressing mode

#### 0x14: load 16 sig register OB rel
- Form 3
- Loads the short pointed to in memory by the second regsel into the first regsel with sign extension
- Offset register B relative addressing mode

#### 0x15: load 16 sig register OC rel
- Form 3
- Loads the short pointed to in memory by the second regsel into the first regsel with sign extension
- Offset register C relative addressing mode

#### 0x16: load 8 register abs
- Form 3
- Loads the char pointed to in memory by the second regsel into the first regsel
- Absolute addressing mode

#### 0x17: load 8 register PC rel
- Form 3
- Loads the char pointed to in memory by the second regsel into the first regsel
- Program counter relative addressing mode

#### 0x18: load 8 register OA rel
- Form 3
- Loads the char pointed to in memory by the second regsel into the first regsel
- Offset register A relative addressing mode

#### 0x19: load 8 register OB rel
- Form 3
- Loads the char pointed to in memory by the second regsel into the first regsel
- Offset register B relative addressing mode

#### 0x1A: load 8 register OC rel
- Form 3
- Loads the char pointed to in memory by the second regsel into the first regsel
- Offset register C relative addressing mode

#### 0x1B: load 8 sig register abs
- Form 3
- Loads the char pointed to in memory by the second regsel into the first regsel with sign extension
- Absolute addressing mode

#### 0x1C: load 8 sig register PC rel
- Form 3
- Loads the char pointed to in memory by the second regsel into the first regsel with sign extension
- Program counter relative addressing mode

#### 0x1D: load 8 sig register OA rel
- Form 3
- Loads the char pointed to in memory by the second regsel into the first regsel with sign extension
- Offset register A relative addressing mode

#### 0x1E: load 8 sig register OB rel
- Form 3
- Loads the char pointed to in memory by the second regsel into the first regsel with sign extension
- Offset register B relative addressing mode

#### 0x1F: load 8 sig register OC rel
- Form 3
- Loads the char pointed to in memory by the second regsel into the first regsel with sign extension
- Offset register C relative addressing mode

#### 0x20: load 32 immediate abs
- Form 2w
- Loads the word pointed to in memory by the argument into the regsel
- Absolute addressing mode

#### 0x21: load 32 immediate PC rel
- Form 2w
- Loads the word pointed to in memory by the argument into the regsel
- Program counter relative addressing mode

#### 0x22: load 32 immediate OA rel
- Form 2w
- Loads the word pointed to in memory by the argument into the regsel
- Offset register A relative addressing mode

#### 0x23: load 32 immediate OB rel
- Form 2w
- Loads the word pointed to in memory by the argument into the regsel
- Offset register B relative addressing mode

#### 0x24: load 32 immediate OC rel
- Form 2w
- Loads the word pointed to in memory by the argument into the regsel
- Offset register C relative addressing mode

#### 0x25: load 16 immediate abs
- Form 2w
- Loads the short pointed to in memory by the argument into the regsel
- Absolute addressing mode

#### 0x26: load 16 immediate PC rel
- Form 2w
- Loads the short pointed to in memory by the argument into the regsel
- Program counter relative addressing mode

#### 0x27: load 16 immediate OA rel
- Form 2w
- Loads the short pointed to in memory by the argument into the regsel
- Offset register A relative addressing mode

#### 0x28: load 16 immediate OB rel
- Form 2w
- Loads the short pointed to in memory by the argument into the regsel
- Offset register B relative addressing mode

#### 0x29: load 16 immediate OC rel
- Form 2w
- Loads the short pointed to in memory by the argument into the regsel
- Offset register C relative addressing mode

#### 0x2A: load 16 sig immediate abs
- Form 2w
- Loads the short pointed to in memory by the argument into the regsel with sign extension
- Absolute addressing mode

#### 0x2B: load 16 sig immediate PC rel
- Form 2w
- Loads the short pointed to in memory by the argument into the regsel with sign extension
- Program counter relative addressing mode

#### 0x2C: load 16 sig immediate OA rel
- Form 2w
- Loads the short pointed to in memory by the argument into the regsel with sign extension
- Offset register A relative addressing mode

#### 0x2D: load 16 sig immediate OB rel
- Form 2w
- Loads the short pointed to in memory by the argument into the regsel with sign extension
- Offset register B relative addressing mode

#### 0x2E: load 16 sig immediate OC rel
- Form 2w
- Loads the short pointed to in memory by the argument into the regsel with sign extension
- Offset register C relative addressing mode

#### 0x2F: load 8 immediate abs
- Form 2w
- Loads the char pointed to in memory by the argument into the regsel
- Absolute addressing mode

#### 0x30: load 8 immediate PC rel
- Form 2w
- Loads the char pointed to in memory by the argument into the regsel
- Program counter relative addressing mode

#### 0x31: load 8 immediate OA rel
- Form 2w
- Loads the char pointed to in memory by the argument into the regsel
- Offset register A relative addressing mode

#### 0x32: load 8 immediate OB rel
- Form 2w
- Loads the char pointed to in memory by the argument into the regsel
- Offset register B relative addressing mode

#### 0x33: load 8 immediate OC rel
- Form 2w
- Loads the char pointed to in memory by the argument into the regsel
- Offset register C relative addressing mode

#### 0x34: load 8 sig immediate abs
- Form 2w
- Loads the char pointed to in memory by the argument into the regsel with sign extension
- Absolute addressing mode

#### 0x35: load 8 sig immediate PC rel
- Form 2w
- Loads the char pointed to in memory by the argument into the regsel with sign extension
- Program counter relative addressing mode

#### 0x36: load 8 sig immediate OA rel
- Form 2w
- Loads the char pointed to in memory by the argument into the regsel with sign extension
- Offset register A relative addressing mode

#### 0x37: load 8 sig immediate OB rel
- Form 2w
- Loads the char pointed to in memory by the argument into the regsel with sign extension
- Offset register B relative addressing mode

#### 0x38: load 8 sig immediate OC rel
- Form 2w
- Loads the char pointed to in memory by the argument into the regsel with sign extension
- Offset register C relative addressing mode

#### 0x39: store 32 register abs
- Form 3
- Stores the word in the first regsel in the memory location pointed to by the second regsel
- Absolute addressing mode

#### 0x3A: store 32 register PC rel
- Form 3
- Stores the word in the first regsel in the memory location pointed to by the second regsel
- Program counter relative addressing mode

#### 0x3B: store 32 register OA rel
- Form 3
- Stores the word in the first regsel in the memory location pointed to by the second regsel
- Offset register A relative addressing mode

#### 0x3C: store 32 register OB rel
- Form 3
- Stores the word in the first regsel in the memory location pointed to by the second regsel
- Offset register B relative addressing mode

#### 0x3D: store 32 register OC rel
- Form 3
- Stores the word in the first regsel in the memory location pointed to by the second regsel
- Offset register C relative addressing mode

#### 0x3E: store 16 register abs
- Form 3
- Stores the short in the first regsel in the memory location pointed to by the second regsel
- Absolute addressing mode

#### 0x3F: store 16 register PC rel
- Form 3
- Stores the short in the first regsel in the memory location pointed to by the second regsel
- Program counter relative addressing mode

#### 0x40: store 16 register OA rel
- Form 3
- Stores the short in the first regsel in the memory location pointed to by the second regsel
- Offset register A relative addressing mode

#### 0x41: store 16 register OB rel
- Form 3
- Stores the short in the first regsel in the memory location pointed to by the second regsel
- Offset register B relative addressing mode

#### 0x42: store 16 register OC rel
- Form 3
- Stores the short in the first regsel in the memory location pointed to by the second regsel
- Offset register C relative addressing mode

#### 0x43: store 8 register abs
- Form 3
- Stores the char in the first regsel in the memory location pointed to by the second regsel
- Absolute addressing mode

#### 0x44: store 8 register PC rel
- Form 3
- Stores the char in the first regsel in the memory location pointed to by the second regsel
- Program counter relative addressing mode

#### 0x45: store 8 register OA rel
- Form 3
- Stores the char in the first regsel in the memory location pointed to by the second regsel
- Offset register A relative addressing mode

#### 0x46: store 8 register OB rel
- Form 3
- Stores the char in the first regsel in the memory location pointed to by the second regsel
- Offset register B relative addressing mode

#### 0x47: store 8 register OC rel
- Form 3
- Stores the char in the first regsel in the memory location pointed to by the second regsel
- Offset register C relative addressing mode

#### 0x48: store 32 immediate abs
- Form 2w
- Stores the word in the regsel in the memory location pointed to by the argument
- Absolute addressing mode

#### 0x49: store 32 immediate PC rel
- Form 2w
- Stores the word in the regsel in the memory location pointed to by the argument
- Program counter relative addressing mode

#### 0x4A: store 32 immediate OA rel
- Form 2w
- Stores the word in the regsel in the memory location pointed to by the argument
- Offset register A relative addressing mode

#### 0x4B: store 32 immediate OB rel
- Form 2w
- Stores the word in the regsel in the memory location pointed to by the argument
- Offset register B relative addressing mode

#### 0x4C: store 32 immediate OC rel
- Form 2w
- Stores the word in the regsel in the memory location pointed to by the argument
- Offset register C relative addressing mode

#### 0x4D: store 16 immediate abs
- Form 2w
- Stores the short in the regsel in the memory location pointed to by the argument
- Absolute addressing mode

#### 0x4E: store 16 immediate PC rel
- Form 2w
- Stores the short in the regsel in the memory location pointed to by the argument
- Program counter relative addressing mode

#### 0x4F: store 16 immediate OA rel
- Form 2w
- Stores the short in the regsel in the memory location pointed to by the argument
- Offset register A relative addressing mode

#### 0x50: store 16 immediate OB rel
- Form 2w
- Stores the short in the regsel in the memory location pointed to by the argument
- Offset register B relative addressing mode

#### 0x51: store 16 immediate OC rel
- Form 2w
- Stores the short in the regsel in the memory location pointed to by the argument
- Offset register C relative addressing mode

#### 0x52: store 8 immediate abs
- Form 2w
- Stores the char in the regsel in the memory location pointed to by the argument
- Absolute addressing mode

#### 0x53: store 8 immediate PC rel
- Form 2w
- Stores the char in the regsel in the memory location pointed to by the argument
- Program counter relative addressing mode

#### 0x54: store 8 immediate OA rel
- Form 2w
- Stores the char in the regsel in the memory location pointed to by the argument
- Offset register A relative addressing mode

#### 0x55: store 8 immediate OB rel
- Form 2w
- Stores the char in the regsel in the memory location pointed to by the argument
- Offset register B relative addressing mode

#### 0x56: store 8 immediate OC rel
- Form 2w
- Stores the char in the regsel in the memory location pointed to by the argument
- Offset register C relative addressing mode

#### 0x57: add int register
- Form 4
- R1 = R2 + R3 (integers)

#### 0x58: add int immediate 32
- Form 3w
- R1 = R2 + Word argument

#### 0x59: add int immediate 16
- Form 3h
- R1 = R2 + Short argument

#### 0x5A: add int immediate 16 sig
- Form 3h
- R1 = R2 + Sign-extended short argument

#### 0x5B: add int immediate 8
- Form 3c
- R1 = R2 + Char argument

#### 0x5C: add int immediate 8 sig
- Form 3c
- R1 = R2 + Sign-extended char argument

#### 0x5D: add float register
- Form 4
- R1 = R2 + R3 (floats)

#### 0x5E: add float immediate
- Form 3w
- R1 = R2 + 32bit float argument

#### 0x5F: subtract int register
- Form 4
- R1 = R2 - R3 (integers)

#### 0x60: subtract int immediate 32
- Form 3w
- R1 = R2 + Word argument

#### 0x61: subtract int immediate 16
- Form 3h
- R1 = R2 + Short argument

#### 0x62: subtract int immediate 16 sig
- Form 3h
- R1 = R2 + Sign-extended short argument

#### 0x63: subtract int immediate 8
- Form 3c
- R1 = R2 + Char argument

#### 0x64: subtract int immediate 8 sig
- Form 3c
- R1 = R2 + Sign-extended char argument

#### 0x65: subtract float register
- Form 4
- R1 = R2 + R3 (floats)

#### 0x66: subtract float immediate
- Form 3w
- R1 = R2 + 32bit float argument

#### 0x67: multiply int register
- Form 4
- R1 = R2 * R3 (integers)

#### 0x68: multiply int immediate 32
- Form 3w
- R1 = R2 * Word argument

#### 0x69: multiply int immediate 16
- Form 3h
- R1 = R2 * Short argument

#### 0x6A: multiply int immediate 16 sig
- Form 3h
- R1 = R2 * Sign-extended short argument

#### 0x6B: multiply int immediate 8
- Form 3c
- R1 = R2 * Char argument

#### 0x6C: multiply int immediate 8 sig
- Form 3c
- R1 = R2 * Sign-extended char argument

#### 0x6D: multiply float register
- Form 4
- R1 = R2 * R3 (floats)

#### 0x6E: multiply float immediate
- Form 3w
- R1 = R2 * 32bit float argument

#### 0x6F: divide int register
- Form 4
- R1 = R2 / R3 (integers)

#### 0x70: divide int immediate 32
- Form 3w
- R1 = R2 / Word argument

#### 0x71: divide int immediate 16
- Form 3h
- R1 = R2 / Short argument

#### 0x72: divide int immediate 16 sig
- Form 3h
- R1 = R2 / Sign-extended short argument

#### 0x73: divide int immediate 8
- Form 3c
- R1 = R2 / Char argument

#### 0x74: divide int immediate 8 sig
- Form 3c
- R1 = R2 / Sign-extended argument

#### 0x75: divide float register
- Form 4
- R1 = R2 / R3 (floats)

#### 0x76: divide float immediate
- Form 3w
- R1 = R2 / 32bit float argument

#### 0x77: modulo register
- Form 4
- R1 = R2 % R3 (integers)

#### 0x78: modulo immediate 32
- Form 3w
- R1 = R2 % Word argument

#### 0x79: modulo immediate 16
- Form 3h
- R1 = R2 % Short argument

#### 0x7A: modulo immediate 16 sig
- Form 3h
- R1 = R2 % Sign-extended short argument

#### 0x7B: modulo immediate 8
- Form 3c
- R1 = R2 % Char argument

#### 0x7C: modulo immediate 8 sig
- Form 3c
- R1 = R2 % Sign-extended char argument

#### 0x7D: and register
- Form 4
- R1 = R2 & R3

#### 0x7E: and immediate 32
- Form 3w
- R1 = R2 & Word argument

#### 0x7F: and immediate 16
- Form 3h
- R1 = R2 & Short argument
- Top 16 argument bits are zero-extended

#### 0x80: and immediate 8
- Form 3c
- R1 = R2 & Char argument
- Top 24 argument bits are zero-extended

#### 0x81: or register
- Form 4
- R1 = R2 | R3

#### 0x82: or immediate 32
- Form 3w
- R1 = R2 | Word argument

#### 0x83: or immediate 16
- Form 3h
- R1 = R2 | Short argument
- Top 16 argument bits are zero-extended

#### 0x84: or immediate 8
- Form 3c
- R1 = R2 | Char argument
- Top 24 argument bits are zero-extended

#### 0x85: xor register
- Form 4
- R1 = R2 ^ R3

#### 0x86: xor immediate 32
- Form 3w
- R1 = R2 ^ Word argument

#### 0x87: xor immediate 16
- Form 3h
- R1 = R2 ^ Short argument
- Top 16 argument bits are zero-extended

#### 0x88: xor immediate 8
- Form 3h
- R1 = R2 ^ Char argument
- Top 24 argument bits are zero-extended

#### 0x89: bslt register
- Form 4
- R1 = R2 bitshifted left R3 places
- Bits shifted out of the register are truncated
- Zeros are shifted into the register to fill empty space

#### 0x8A: bslt immediate 8
- Form 3c
- R1 = R2 bitshifted left by the argument's count
- Bits shifted out of the register are truncated
- Zeros are shifted into the register to fill empty space

#### 0x8B: bslc register
- Form 4
- R1 = R2 bitshifted left R3 places
- Bits shifted out of the register are shifted into the cary flag on each iteration
- The cary flag gets shifted into the rightmost bit on each iteration

#### 0x8C: bslc immediate 8
- Form 3c
- R1 = R2 bitshifted left by the argument's count
- Bits shifted out of the register are shifted into the cary flag on each iteration
- The cary flag gets shifted into the rightmost bit on each iteration

#### 0x8D: bsrt register
- Form 4
- R1 = R2 bitshifted right R3 places
- Bits shifted out of the register are truncated
- Zeros are shifted into the register to fill empty space

#### 0x8E: bsrt immediate 8
- Form 3c
- R1 = R2 bitshifted right by the argument's count
- Bits shifted out of the register are shifted into the cary flag on each iteration
- The cary flag gets shifted into the rightmost bit on each iteration

#### 0x8F: bsrc register
- Form 4
- R1 = R2 bitshifted right R3 places
- Bits shifted out of the register are shifted into the cary flag on each iteration
- The cary flag gets shifted into the rightmost bit on each iteration

#### 0x90: bsrc immediate 8
- Form 3c
- R1 = R2 bitshifted right by the argument's count
- Bits shifted out of the register are shifted into the cary flag on each iteration
- The cary flag gets shifted into the rightmost bit on each iteration

#### 0x91: not register
- Form 3
- R1 = ~R2

#### 0x92: compare int register
- Form 3
- R1 - R2 (integers)
- Flags updated but no result saved

#### 0x93: compare int immediate 32
- Form 2w
- R1 - Word argument
- Flags updated but no result saved

#### 0x94: compare int immediate 16
- Form 2h
- R1 - Short argument
- Flags updated but no result saved

#### 0x95: compare int immediate 16 sig
- Form 2h
- R1 - Sign extended short argument
- Flags updated but no result saved

#### 0x96: compare int immediate 8
- Form 2c
- R1 - Char argument
- Flags updated but no result saved

#### 0x97: compare int immediate 8 sig
- Form 2c
- R1 - Sign-extended char argument
- Flags updated but no result saved

#### 0x98: compare float register
- Form 3
- R1 - R2 (floats)
- Flags updated but no result saved

#### 0x99: compare float immediate
- Form 2w
- R1 - 32bit float argument
- Flags updated but no result saved

#### 0x9A: evaluate int register
- Form 8
- R2 - R3 (integers)
- Flags updated
- Boolean result of flag condition stored in R1

#### 0x9B: evaluate int immediate 32
- Form 7w
- R2 - Word argument
- Flags updated
- Boolean result of flag condition stored in R1

#### 0x9C: evaluate int immediate 16
- Form 7h
- R2 - Short argument
- Flags updated
- Boolean result of flag condition stored in R1

#### 0x9D: evaluate int immediate 16 sig
- Form 7h
- R2 - Sign-extended short argument
- Flags updated
- Boolean result of flag condition stored in R1

#### 0x9E: evaluate int immediate 8
- Form 7c
- R2 - Char argument
- Flags updated
- Boolean result of flag condition stored in R1

#### 0x9F: evaluate int immediate 8 sig
- Form 7c
- R2 - Sign-extended char argument
- Flags updated
- Boolean result of flag condition stored in R1

#### 0xA0: evaluate float register
- Form 8
- R2 - R3 (floats)
- Flags updated
- Boolean result of flag condition stored in R1

#### 0xA1: evaluate float immediate
- Form 7w
- R2 - 32bit float argument
- Flags updated
- Boolean result of flag condition stored in R1

#### 0xA2: branch register abs
- Form 6
- Branch to the address pointed to by the regsel according to the flag condition
- Absolute addressing mode

#### 0xA3: branch register PC relative
- Form 6
- Branch to the address pointed to by the regsel according to the flag condition
- Program counter relative addressing mode

#### 0xA4: branch register OA relative
- Form 6
- Branch to the address pointed to by the regsel according to the flag condition
- Offset register A relative addressing mode

#### 0xA5: branch register OB relative
- Form 6
- Branch to the address pointed to by the regsel according to the flag condition
- Offset register B relative addressing mode

#### 0xA6: branch register OC relative
- Form 6
- Branch to the address pointed to by the regsel according to the flag condition
- Offset register C relative addressing mode

#### 0xA7: branch register push abs
- Form 6
- Branch to the address pointed to by the regsel according to the flag condition
- Absolute addressing mode
- Push return address to the stack if branch is taken

#### 0xA8: branch register push PC relative
- Form 6
- Branch to the address pointed to by the regsel according to the flag condition
- Program counter relative addressing mode
- Push return address to the stack if branch is taken

#### 0xA9: branch register push OA relative
- Form 6
- Branch to the address pointed to by the regsel according to the flag condition
- Offset register A relative addressing mode
- Push return address to the stack if branch is taken

#### 0xAA: branch register push OB relative
- Form 6
- Branch to the address pointed to by the regsel according to the flag condition
- Offset register B relative addressing mode
- Push return address to the stack if branch is taken

#### 0xAB: branch register push OC relative
- Form 6
- Branch to the address pointed to by the regsel according to the flag condition
- Offset register C relative addressing mode
- Push return address to the stack if branch is taken

#### 0xAC: branch immediate abs
- Form 5w
- Branch to the address pointed to by the word argument according to the flag condition
- Absolute addressing mode

#### 0xAD: branch immediate PC relative
- Form 5w
- Branch to the address pointed to by the word argument according to the flag condition
- Program counter relative addressing mode

#### 0xAE: branch immediate OA relative
- Form 5w
- Branch to the address pointed to by the word argument according to the flag condition
- Offset register A relative addressing mode

#### 0xAF: branch immediate OB relative
- Form 5w
- Branch to the address pointed to by the word argument according to the flag condition
- Offset register B relative addressing mode

#### 0xB0: branch immediate OC relative
- Form 5w
- Branch to the address pointed to by the word argument according to the flag condition
- Offset register C relative addressing mode

#### 0xB1: branch immediate push abs
- Form 5w
- Branch to the address pointed to by the word argument according to the flag condition
- Absolute addressing mode
- Push return address to the stack if branch is taken

#### 0xB2: branch immediate push PC relative
- Form 5w
- Branch to the address pointed to by the word argument according to the flag condition
- Program counter relative addressing mode
- Push return address to the stack if branch is taken

#### 0xB3: branch immediate push OA relative
- Form 5w
- Branch to the address pointed to by the word argument according to the flag condition
- Offset register A relative addressing mode
- Push return address to the stack if branch is taken

#### 0xB4: branch immediate push OB relative
- Form 5w
- Branch to the address pointed to by the word argument according to the flag condition
- Offset register B relative addressing mode
- Push return address to the stack if branch is taken

#### 0xB5: branch immediate push OC relative
- Form 5w
- Branch to the address pointed to by the word argument according to the flag condition
- Offset register C relative addressing mode
- Push return address to the stack if branch is taken

#### 0xB6: int to bool
- Form 3
- If any bits in R2 are set, set R1 to 1
- Else set R1 to 0

#### 0xB7: float to int
- Form 3
- Convert R2 from a float to an integer and store in R1

#### 0xB8: int to float
- Form 3
- Convert R2 from an integer to a float and store in R1

#### 0xB9: push
- From 2
- Push the register to the stack

#### 0xBA: pop
- Form 2
- Pop from the stack into the register

#### 0xBB: pushall
- Form 1
- Push registers G0:G7 and FL to the stack

#### 0xBC: popall
- Form 1
- Pop registers G0:G7 and FL from the stack

#### 0xBD: peek
- Form 2
- Peek from the stack into a register

#### 0xBE: return
- Form 1
- Pop from the stack into the program counter to return from a branch-push

#### 0xBF: susspend interrupts
- Form 1
- Prevents any non-critical interrupts from being handled

#### 0xC0: resume interrupts
- Form 1
- Resumes allowing non-critical interrupts from being handled
- If currently handling an interrupt, will stop handling the interrupt without returning to the original execution branch

#### 0xC1: trigger interrupt register
- Form 2
- Triggers an interrupt with the data provided in a register

#### 0xC2: trigger interrupt immediate 16
- Form 1h
- Triggers an interrupt with the data provided in the argument

#### 0xC3: get interrupt parameter
- Form 2
- Puts the interrupt parameter in the selected register

#### 0xC4: get return address
- Form 2
- Puts the interrupt return address in the selected register

#### 0xC5: set return address register
- Form 2
- Sets the interrupt return address from the selected register

#### 0xC6: set return address immediate
- Form 1w
- Sets the interrupt return address with the argument

#### 0xC7: debug
- Form 2
- Prints the value in the specified register to the terminal
- For debug emulator only

#### 0xC8: terminate
- Form 1
- Terminates execution
