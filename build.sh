#!/bin/bash
objects=main.cpp

compiler=gcc

libPath=/usr/lib
incPath=/usr/include

objectName=agafb

compilerFlags="-w -o $objectName"
linkerFlags=-lSDL2

$compiler $objects $compilerFlags $linkerFlags
