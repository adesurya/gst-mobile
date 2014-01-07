ROOT=/opt/zdisk/zerox/android/android-ndk-r9

CCROOT=$ROOT/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86
SYSROOT=$ROOT/platforms/android-14/arch-arm/
HOST=arm-linux-androideabi
CACHE_FILE=./arm-linux.cache

export AR="$CCROOT/bin/$HOST-ar --sysroot=$SYSROOT"
export CC="$CCROOT/bin/$HOST-gcc --sysroot=$SYSROOT"
export CXX="$CCROOT/bin/$HOST-g++ --sysroot=$SYSROOT"
export STRIP="$CCROOT/bin/$HOST-strip --sysroot=$SYSROOT"
export RANLIB="$CCROOT/bin/$HOST-ranlib --sysroot=$SYSROOT"
export CFLAGS=""
export LDFLAGS=""


PWD=`pwd`
export GLIB_CFLAGS="-I$PWD/../glib2.0-2.32.1/oldbld/libglib/include/glib-2.0"
export GLIB_LIBS="-L$PWD/../glib2.0-2.32.1/oldbld/libglib/lib -lglib-2.0"
export GIO_CFLAGS="-I$PWD/../glib2.0-2.32.1/oldbld/libglib/include/glib-2.0"
export GIO_LIBS="-L$PWD/../glib2.0-2.32.1/oldbld/libglib/lib -lgio-2.0"

export CFLAGS="$CFLAGS $GLIB_CFLAGS $GIO_CFLAGS"
export LDFLAGS="$LDFLAGS $GLIB_LIBS $GIO_LIBS"

##
## start configure
mkdir -p oldbld
cd oldbld

PWD=`pwd`
PREFIX=$PWD/libgstreamer

# --disable-registry 
#--disable-plugin 
../configure \
    --prefix=$PREFIX \
    --host=arm-linux-androideabi  \
    --disable-nls --disable-rpath \
    --disable-gst-debug \
    --disable-parse --disable-option-parsing \
    --disable-trace --disable-alloc-trace \
    --disable-debug \
    --disable-valgrind \
    --disable-examples \
    --enable-static-plugins \
    --disable-tests --disable-failing-tests --disable-benchmarks \
    --disable-tools --disable-largefile \
    --disable-check --disable-Bsymbolic \
    --without-libiconv-prefix \
    --without-libintl-prefix 
