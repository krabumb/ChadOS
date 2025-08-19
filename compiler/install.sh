#!/bin/bash

sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

# Create installation directory
mkdir -p ~/opt/cross
export TARGET=x86_64-elf
export PREFIX="$HOME/opt/cross"
export PATH="$PREFIX/bin:$PATH"

# Download binutils
wget https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.gz
tar -xvf binutils-2.42.tar.gz
mkdir binutils-build
cd binutils-build
../binutils-2.42/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install
cd ..

# Download GCC
wget https://ftp.gnu.org/gnu/gcc/gcc-13.1.0/gcc-13.1.0.tar.gz
tar -xvf gcc-13.1.0.tar.gz
mkdir gcc-build
cd gcc-build
../gcc-13.1.0/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c --without-headers
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc
