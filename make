#!/usr/bin/bash

if [ 0 -eq $# ]; then
    echo "Making dgc32..."
    gcc ./src/*.c ./src/deviceManagers/*.c -Iinclude ./include/*.h -Iinclude ./include/deviceManagers/*.h -o ./dgc32 -Wall -Werror && echo "Success!"
elif [ $1 = "selftest" ]; then
    echo "Making dgc32-st..."
    gcc ./src/*.c ./src/deviceManagers/*.c -Iinclude ./include/*.h -Iinclude ./include/deviceManagers/*.h -DSELF_TEST -o ./dgc32-st -Wall -Werror && echo "Success!"
elif [ $1 = "binwriter" ]; then
    echo "Making binwriter..."
    gcc ./src/binwriter/binwriter.c -o ./binwriter -Wall -Werror && echo "Success!"
else
    echo "Invalid argument"
fi