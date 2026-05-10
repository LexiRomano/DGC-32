# Dboard

A d-grade keyboard manager

## Memory map

| device data address | description                         | Access |
| ------------------- | ----------------------------------- | ------ |
|                   0 | Peripheral type identifier = 0x4B   |     RO |
|                   1 | Keyboard protocol identifier = 0x01 |     RO |

## Input

The dboard has no inputs.

## Output

The dboard will trigger keyboard interrupts based on pressed keys with the following scancodes:

| scancode  | key       |
| --------  | --------  |
| 0x00-0x19 | a-z       |
| 0x1A-0x23 | 0-9       |
| 0x24      | -         |
| 0x25      | =         |
| 0x26      | [         |
| 0x27      | ]         |
| 0x28      | \         |
| 0x29      | ;         |
| 0x2A      | '         |
| 0x2B      | ,         |
| 0x2C      | .         |
| 0x2D      | /         |
| 0x2E      | `         |
| 0x3F      | LCTRL     |
| 0x30      | LSHIFT    |
| 0x31      | LALT      |
| 0x32      | TAB       |
| 0x33      | RCTRL     |
| 0x34      | RSHIFT    |
| 0x35      | RALT      |
| 0x36      | ENTER     |
| 0x37      | BACKSPACE |
| 0x38      | DELETE    |
| 0x39      | SPACE     |
| 0x3A      | UP        |
| 0x3B      | DOWN      |
| 0x3C      | LEFT      |
| 0x3D      | RIGHT     |
| 0x3E      | ESC       |
