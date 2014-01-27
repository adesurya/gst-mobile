if test -z $ROOT; then
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
fi
export CFLAGS=""
export LDFLAGS=""

PWD=`pwd`
export OGG_CFLAGS="-I$PWD/../groot/include"
export OGG_LIBS="-L$PWD/../groot/lib -logg"

##
## start configure
mkdir -p oldbld
cd oldbld

PREFIX=`pwd`/../../groot
mkdir -p $PREFIX

../configure --prefix=$PREFIX --host=arm \
    --enable-static \
    --disable-shared \
    --disable-oggtest \
    --disable-vorbistest \
    --disable-sdltest \
    --disable-examples

make && make install

exit 0
