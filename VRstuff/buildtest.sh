#!/bin/bash
clear
cd ./TestBin
g++ ../src/test.cpp -Iinclude -I../../../PakkiUtils -g -Wall -fexceptions -lm -lpthread -lglfw -lrt -lm -ldl -o testENGINE
cd ../
echo hello

