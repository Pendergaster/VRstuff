#!/bin/bash
clear
g++ ./src/main.cpp -Iinclude -I../../PakkiUtils -g -Wall -fexceptions -lm -lpthread -lglfw -lrt -lm -ldl -o testENGINE
echo hello

