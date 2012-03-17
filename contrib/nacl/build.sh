#!/bin/sh
# TODO use the common Makefile instead of duplicating it.
set -ex

if [ -z $NACL_SDK_ROOT ]
then
	echo "NACL_SDK_ROOT must be set."
	echo "example: NACL_SDK_ROOT=$HOME/nacl-sdk-update/pepper_15"
	exit 1
fi

TOOLCHAIN=$NACL_SDK_ROOT/toolchain/linux_x86_newlib
NACL_ROOT=$(dirname $(which $0))
ROOT=$(readlink -f $NACL_ROOT/../..)
PKGS="sdl SDL_gfx SDL_image libpng"

rm -f $NACL_ROOT/examples
ln -s $ROOT/examples $NACL_ROOT/examples

for HOST in i686-nacl x86_64-nacl
do
    cd $NACL_ROOT
    export PKG_CONFIG_PATH=$TOOLCHAIN/$HOST/usr/lib/pkgconfig
    export CC=$HOST-gcc
    BFDARCH=i386
    if [ $HOST == i686-nacl ]; then
        BFDNAME=elf32-nacl
    else
        BFDNAME=elf64-nacl
    fi
    CFLAGS="-O2 -Wall -W -D main=load81_main `pkg-config --cflags $PKGS`"
    LDFLAGS=""
    for X in read write open close seek mount; do
        LDFLAGS="$LDFLAGS -Xlinker --wrap -Xlinker $X"
    done
    LIBS="`pkg-config --libs $PKGS` -ljpeg -lz -llua -lm -lppapi -lppapi_cpp -lnacl-mounts -lstdc++ -lnosys"
    SRCS="$ROOT/load81.c $ROOT/editor.c $ROOT/framebuffer.c nacl.cc"
    $CC $CFLAGS $LDFLAGS $SRCS $LIBS -o load81-$HOST.nexe
    $HOST-strip load81-$HOST.nexe
done
echo Built successfully
