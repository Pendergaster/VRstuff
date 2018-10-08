#!/bin/bash
clear
cd ./game
cd ./DebugBin
g++ -shared ../src/game.cpp -I../../../../PakkiUtils -g -Wall -fexceptions -lm -lpthread -lrt -lm -ldl -o game.lib
cd ../
cd ../
echo hello

