# D-Grade Computer (DGC-32)

## Introduction

*Note: Although the language used in these documents may suggest otherwise, a lot of the functionality described is a work in progress. Expect some things to be missing or changed in the future.*

The DGC-32 is a 32-bit "D-Grade" computer architecture. It is developed as a successor to the F-Grade Computer (FGC-16), a 16-bit computer architecture also designed by myself. The philosophy behind the FGC-16 and its parent project (F-Suite) was to push the bounds of my technical knowledge and attempt to build an entire computing system from an architecture all the way up to an operating system. Understandibly, I fell just short of a kernel before realizing the flaws in the underlying architecture and had to stop due to both performance and sanity issues. The DGC-32 is the second iteration of designs, heeding the warnings of its predecessor. It has ben designed with the lessons learned from the FGC-16, my university courses, and Ben Eater osmosis. Here's hoping that this goes somewhere.

## Audience

This project is designed with the intention to be accessible to those who cannot read my mind. Doing so is a challenge as I am burdened with the unending responsibility of reading my own mind. Please reffer to the Resources section for more information.

### Contributions

I will be developing the core architecture and emulator on my own. I do, however, welcome contributions in the way that information is presented. As mentionned above, I want this to be accessable to as many people as possible and that requires feedback. I can accept that feedback in any conceivable manner, including pull requests, direct messages, and carrier pigeons. You are also encouraged to tinker around with the emulator's code and write devices and peripherals. I have tried to do this in a modular way so you can swap out my devices for your own. Just be sure not to collide with the "standards" that I have set out - you can come up with your own too if you'd like!!

## Usage

*WIP, some makes do not exist yet*

Here are the different ways to make the executables in this project:
```
bash$ ./make             # Regular dgc32 executable
bash$ ./make usertest    # User testing version of the executable (dgc32-ut)
bash$ ./make selftest    # Self testing version of the executable (dgc32-st - for regression testing. You can ignore this one)
bash$ ./make binwriter   # Binwriter tool (see Tools section below)
```

## Tools

### Dssembly

*WIP, does not exist yet*

Dssembly is the assembly language and assembler corresponding to the DGC-32 proccessor. It is in its own repository labeled as such and is developed in parallel with this emulator. It is the suggested way to write programs for the DGC-32.

### Binwriter

Binwriter is a very basic text to binary converter to be able to provide very basic input to the DGC-32. It is highly recommended to use Dssembly (mentionned above).

Use the binwriter as follows:
```
bash$ ./binwriter inputFileName outputFileName  # Parses ascii hex from inputFileName into a binary outputFileName
bash$ ./binwriter                               # Prints usage
```

## Resources

Specifications of the DGC-32 architecture can be found in doc/design. Start with the HLD an then explore other design documents to understand how this architecture works.

I aim to share this project with all sorts of people, from novices to experts in low-level computing (I'm not claiming to be an expert with that statement). For those who are new to this field, please consult doc/help. It contains all of the knowledge I posses relating to computer architectures which may help you understand this project better. You could definitely get more out of a university course on the topic, but it's a start. If you know your shit, you can safely skip it. Questions and comments are welcome, I aim to continuously develop those documents to better share this hobby with others!
