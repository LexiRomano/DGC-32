#!/usr/bin/bash

if [ 0 -eq $# ]; then
    echo "Making dgc32..."
    gcc ./src/*.c -Iinclude ./include/*.h -o ./dgc32 -Wall -Werror && echo "Success!"
elif [ $1 = "binwriter" ]; then
    echo "Making binwriter..."
    gcc ./src/binwriter/binwriter.c -o ./binwriter -Wall -Werror && echo "Success!"
else
    echo "Invalid argument"
fi