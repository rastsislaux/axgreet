#! /usr/bin/env bash

set -e

NOB_LINK=https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h

if [ ! -f "build/nob.h" ]; then
    printf "bootstrap.sh: nob.h not found, downloading...\n"
    mkdir -p build
    curl -L $NOB_LINK -o build/nob.h
else
    printf "bootstrap.sh: nob.h found, using existing...\n"
fi

if [ ! -f "nob" ]; then
    printf "bootstrap.sh: nob not found, building...\n"
    cc -o nob nob.c

    printf "bootstrap.sh: nob built successfully\n"
    printf "bootstrap.sh: nob is located at $(pwd)/nob\n"
    printf "bootstrap.sh: from now on run ./nob to build the project\n"
else
    printf "bootstrap.sh: nob found, just run it to build the project\n"
fi
