#!/bin/zsh
CWD="`pwd`"
mkdir build
cd build
../configure --prefix="${CWD}/dist"
make
make install
