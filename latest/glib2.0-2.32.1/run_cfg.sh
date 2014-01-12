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

##
## android support sources
jnipath=andriod/support/jni
libpath=$jnipath/../obj/local/armeabi-v7a
if [ ! -f $PWD/../groot/lib/libandroid_support.a ]; then
    (
    mkdir -p $jnipath
    cd $jnipath
    cat > Application.mk << __EOF
APP_MODULES := android_support
APP_ABI := armeabi-v7a
APP_PLATFORM := android-14
__EOF
    cat > Android.mk << __EOF
include $ROOT/sources/android/support/Android.mk
__EOF
    $ROOT/ndk-build
    )

    if [ -f $libpath/libandroid_support.a ]; then
        cp -f $libpath/libandroid_support.a $PWD/../groot/lib/
        cp -f $libpath/libandroid_support.a $PWD/../groot/lib/libintl.a
        cp -f $libpath/libandroid_support.a $PWD/../groot/lib/libiconv.a
        cp -f $libpath/libandroid_support.a $PWD/../groot/lib/libpthread.a
        cp -f $libpath/libandroid_support.a $PWD/../groot/lib/librt.a
    fi
fi

if [ ! -f $PWD/../groot/android/include/libintl.h ]; then
    (
    cat >  $PWD/../groot/android/include/libintl.h << __EOF
#ifndef _LIBINTL_H_ANDROID_
#define _LIBINTL_H_ANDROID_
char *gettext(const char *msgid);
char *dgettext(const char *domainname, const char *msgid);
char *dcgettext(const char *domainname, const char *msgid, int category);
#endif
__EOF
    )
fi

export CFLAGS="$CFLAGS -DANDROID -I$ROOT/sources/android/support/include -I$PWD/../groot/android/include"
export LDFLAGS="$LDFLAGS -L$PWD/../groot/lib"

##
## for libffi
export LIBFFI_CFLAGS="-I$PWD/../groot/lib/libffi-3.0.11/include"
export LIBFFI_LIBS="-L$PWD/../groot/lib -lffi"
export LDFLAGS="$LDFLAGS $LIBFFI_LIBS $DBUS1_LIBS"


##
## start configure
mkdir -p oldbld
cd oldbld

PREFIX=`pwd`/../../groot
mkdir -p $PREFIX

chmod a+w $CACHE_FILE
cat > $CACHE_FILE << __EOF
glib_cv_stack_grows=no
glib_cv_uscore=no
ac_cv_func_posix_getpwuid_r=no
ac_cv_func_posix_getgrgid_r=no
__EOF
chmod a-w $CACHE_FILE

../configure \
    --prefix=$PREFIX \
    --host=arm-linux-androideabi  \
    --cache-file=$CACHE_FILE \
    --enable-static \
    --disable-selinux --disable-fam --disable-xattr

make && make install
