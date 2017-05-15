#!/bin/bash
objects=main.cpp

compiler=g++

libPath=/usr/lib
incPath=/usr/include

objectName=agafb

compilerFlags="-Wall -Wextra -w -o $objectName"
linkerFlags="-lSDL2 -lSDL2_ttf"

$compiler $objects $compilerFlags $linkerFlags
