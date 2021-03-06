#!/bin/bash

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

if [ "$1" == "clean" ]; then
    rm -rf build
fi

if [ ! -d "./build" ]; then
    mkdir build
    cd build
    cmake .. -G "Visual Studio 15 Win64"
    cd ../
fi

TYPE="Release"

if [ "$1" == "debug" ] || [ "$2" == "debug" ] ; then
    TYPE="Debug"
fi

cmake --build build --config "$TYPE"

if [[ $? > 0 ]]; then
    exit 1
fi

mkdir -p bin/Release
mkdir -p bin/Debug

cp -r assets bin/Release/
cp -r assets bin/Debug/

if [ "$1" == "debug" ] || [ "$2" == "debug" ] ; then
    echo ""
else
    cd bin/Release
    ./rl1.exe
    cd ../..
fi
