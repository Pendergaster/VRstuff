#!/bin/bash
clear
cd ./DebugBin
gcc ../main.c -Iinclude -I../../PakkiUtils -g -Wall -fexceptions -lm -lpthread -lrt -lm -ldl -o ENGINE
cd ../
echo hello

