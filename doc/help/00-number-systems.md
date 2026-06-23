# Number Systems

The very foundation.

## Overview

In essence, all computers do is manipulate numbers. Barring any fancy real-world interactions that peripherals provide, a computer is just a super fancy calculator. That being said, the way computers interpret and manipulate numbers under the hood matters much more than your old Casio. In this document, we will explore the intricacies of different number systems, their representation, encoding, and uses.

## Bases and Base Ten (Decimal)

Let's start with the number system (hopefully) everyone is familiar with: base ten. This is usually the default number system everywhere in the world in almost every field unless explicitly marked otherwise. This number system uses ten symbols, being `0123456789`. To represent numbers greater than ten, we use place values to denote powers of ten being added. Each place value is equal to the number times ten to the power of the number's index - starting from the right. Take the number `23`:

```
  23
= 20 + 3
= 2*10^1 + 3*10^0
```

We sometimes reffer to where a digit is in a number as the `N's column`, where `N` is the power to multiply the digit by. So for example, in `23`, the `2` is in the ten's column and 3 is in the one's column. For something bigger like `712`, the `7` would be in the hundred's column, one further to the left would be the thousand's column, and so on. 

You might notice that in this positional notation, the number of symbols for digits matches the base of the power we multiply each digit by. So what happens if we change the number from ten to something else? Well then we get a different base! For example:

## Base Two (Binary/Bin)

Let's try creating a positional number system with a base of two. This means that we are only allowed to use two symbols for a digit: `0` and `1`. This turns out to be the smallest integer base you can have while still being useful (citation needed). So then how do we represent the number two if we can't use `2`? That would look like `10`. That may look like ten, and it would be if we were in base ten, but the second column is no longer the ten's column. The second column is now instead the two's column! To avoid confusion between the different bases, we usually add a prefix to any non-decimal bases. In the case of base two - or binary - we prefix with `0b`, making our two look like `0b10`. There, now it's a bit harder to confuse with ten. This is why I have been spelling out numbers instead of writing them with digits most of the time. Technically, every base is base 10 (read as one-zero) since 10 is always interpreted as the number of symbols in the base which is in turn the name of that base. Let's go back to the place values and see how `0b10` gets picked apart:

```
  0b10
= 0b10  + 0b0
= 1*2^1 + 0*2^0
= 2     + 0
= 2
```
That's quite a simple example, let's ramp things up a bit. What does thirty nine look like in binary?

```
  0b100111
= 0b100000 + 0b00000 + 0b0000 + 0b100 + 0b10  + 0b1
= 1*2^5    + 0*2^4   + 0*2^3  + 1*2^2 + 1*2^1 + 1*2^0
= 32       + 0       + 0      + 4     + 2     + 1
= 39
```

## Base Sixteen (Hexadecimal/Hex)

Just as we can have number systems with bases smaller than ten, we can also have number systems with bases greater than ten. One of the most common larger bases is base sixteen. We reuse the ten symbols from decimal and use the first six letters of the alphabet for the remaining six. This gives us the symbols `0123456789ABCDEF`. The capitalization of the letters is almost never significant, but I personally preffer capital letters. Now just as in binary, the second column is no longer the tens column, and `10` is no longer ten. Instead we have the sixteen's column with `10` evaluating to sixteen! But once again, we need a prefix to distinguish that we are not using decimal. For base sixteen - or hexadecimal - we usually use `0x`, making sixteen look like `0x10`. Note that sometimes an `h` suffix is used instead, making in `10h`, but that isn't recognized by the C compiler and will not be used here. A great advantage of hexadecimal is that it compresses numbers into fewer digets. For example, let's look at the number thirty-two thousand five-hundred and eighty eight (32588).

```
  0x7F4C
= 0x7000 + 0xF00   + 0x40   + 0xC
= 7*16^3 + 15*16^2 + 4*16^1 + 12*16^0
= 28672  + 3840    + 64     + 12
= 32588
```

In that example, the hex number uses just four digits while the decimal number takes five! ..ignoring the fact that the `0x` prefix brings the total characters for the hex number up to six. This really shines compared to the same number in binary which would be fifteen digits long! The reason why base sixteen is of particular interest to us is that sixteen is a power of two. This means that it is super easy to convert between binary and hexadecimal. Watch this magic!

```
0x7F4C

0x7 = 0b0111
0xF = 0b1111
0x4 = 0b0100
0xC = 0b1100

  0x7F4C
= 0x7    F    4    C
= 0b0110 1111 0100 1100
= 0b0110111101001100
```
Each hex digit is equivalent to four binary digits (bits). Because of this, hexadecimal is a great way of representing a number which may be physically represented in binary, but can be shortened to fit in a reasonable amount of space.

## Math

Math in different bases works about the same as in decimal. There are, however, some additional operations which we can do when we deal with the realm of binary: bitwise operators. Let's take a look at some:

### Bitwise AND

A bitwise AND compares each column of a binary number to another and checks if both bits are equal to `1`. If they are, the final result in that column is a `1`, otherwise it is a `0`. The truth table looks like this:

| A & B | 0 | 1 |
| ----- | - | - |
|     0 | 0 | 0 |
|     1 | 0 | 1 |

For example, the bitwise AND of nine and twelve looks like this:

```
  0b1001
& 0b1100
= 0b1000
```

### Bitwise OR

A bitwise OR compares each column of a binary number to another and checks if either bit is equal to `1`. If they are, the final result in that column is a `1`, otherwise it is a `0`. The truth table looks like this:

| A \| B | 0 | 1 |
| ------ | - | - |
|      0 | 0 | 1 |
|      1 | 1 | 1 |

For example, the bitwise OR of nine and twelve looks like this:

```
  0b1001
| 0b1100
= 0b1101
```

### Bitwise XOR

TODO

### Bitwise NOT

TODO

## Application

TODO talk about voltage n' stuff
