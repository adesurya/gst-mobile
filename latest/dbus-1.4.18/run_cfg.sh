ROOT=/opt/zdisk/zerox/android/android-ndk-r9

CCROOT=$ROOT/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86
SYSROOT=$ROOT/platforms/android-14/arch-arm/
HOST=arm-linux-androideabi
CACHE_FILE=./arm-linux.cache


#export AR="$CCROOT/bin/$HOST-ar --sysroot=$SYSROOT"
export CC="$CCROOT/bin/$HOST-gcc --sysroot=$SYSROOT"
export CXX="$CCROOT/bin/$HOST-g++ --sysroot=$SYSROOT"
export STRIP="$CCROOT/bin/$HOST-strip --sysroot=$SYSROOT"
#export RANLIB="$CCROOT/bin/$HOST-ranlib --sysroot=$SYSROOT"

PWD=`pwd`
export CFLAGS="$CFLAGS -DANDROID -I$PWD/../expat-2.0.1/oldbld/libexpat/include"
export LDFLAGS="$LDFLAGS -L$PWD/../expat-2.0.1/oldbld/libexpat/lib -lexpat"



##
## start configure
mkdir -p oldbld
cd oldbld
PREFIX=`pwd`/libdbus
mkdir -p $PREFIX

../configure --prefix=$PREFIX --host=arm 

make; make install
