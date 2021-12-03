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
# LIBSIGSEGV_LIB="/Users/Darwin64/lib/libsigsegv.a" \
# BUDDY_LIB="/Users/Darwin64/lib/libbdd.a" \
# YICES2_LIB="/Users/Darwin64/lib/libyices.a" \
# TECLA_LIBS="/Users/Darwin64/lib/libtecla.a /Users/Darwin64/lib/libtecla_r.a -lcurses" \
GMP_LIBS="/Users/Darwin64/lib/libgmpxx.a /Users/Darwin64/lib/libgmp.a" \
--prefix="${CWD}/dist"

cp config.h "$CWD"

make
make check
make install
