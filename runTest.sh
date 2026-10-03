#!/usr/bin/bash

./make.sh selftest && \
./make.sh binwriter && \
./binwriter ./test/testcode.txt ./test/rom.bin && \
./dgc32-st ./test/testfile.txt ./test/rom.bin
