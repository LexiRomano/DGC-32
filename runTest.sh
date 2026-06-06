#!/usr/bin/bash

./make.sh selftest && dssembly -r ./test/testcode.txt ./test/rom.bin && ./dgc32-st ./test/testfile.txt ./test/rom.bin
