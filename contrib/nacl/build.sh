#!/bin/sh
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

for HOST in i686-nacl x86_64-nacl
do
    export PKG_CONFIG_PATH=$TOOLCHAIN/$HOST/usr/lib/pkgconfig
    export CC=$HOST-gcc
    $CC -O2 -Wall -W $ROOT/load81.c $NACL_ROOT/nacl.cc `pkg-config --cflags sdl` `pkg-config --libs sdl` -llua -lm -lppapi -lppapi_cpp -lstdc++ -lcrt_common -lnosys -o $NACL_ROOT/load81-$HOST.nexe
done

echo Built successfully
