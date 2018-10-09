#!/bin/bash
clear
touch ./.lock
cd ./game
cd ./DebugBin
g++ -shared ../src/game.cpp -I../../../../PakkiUtils -I../../shared -g -Wall -fexceptions -lm -lpthread -lrt -lm -ldl -fPIC -o game.lib
cd ../
cd ../
rm -f ./.lock
echo hello

