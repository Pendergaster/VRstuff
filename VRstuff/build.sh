#!/bin/bash
clear
cd ./DebugBin
g++ ../src/main.cpp -I../include -I../../../PakkiUtils -g -Wall -fexceptions -lm -lpthread -lglfw -lrt -lm -ldl -o ENGINE
cd ../
echo hello

