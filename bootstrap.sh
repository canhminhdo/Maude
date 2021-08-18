#!/bin/zsh

CWD="`pwd`"
mkdir build
cd build

../configure \
--with-yices2=yes \
--with-cvc4=no \
FLEX=/opt/local/bin/flex \
BISON=/usr/local/bin/bison \
CFLAGS="-g -Wall -O3 -fno-stack-protector -fstrict-aliasing" \
CXXFLAGS="-g -Wall -O3 -fno-stack-protector -fstrict-aliasing -std=c++11" \
CPPFLAGS="-I/Users/Darwin64/include" \
LDFLAGS="-L/Users/Darwin64/lib" \
GMP_LIBS="/Users/Darwin64/lib/libgmpxx.a /Users/Darwin64/lib/libgmp.a" \
--prefix="${CWD}/dist"

cp config.h "$CWD"

make
make check
make install
