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
export CFLAGS=""
export LDFLAGS=""



##
## android support sources
jnipath=andriod/support/jni
libpath=`pwd`/$jnipath/../obj/local/armeabi-v7a
incpath=`pwd`/$jnipath/../include
if [ ! -f $libpath/libandroid_support.a ]; then
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
fi
if [ ! -f $libpath/libiconv.a ]; then
    ln -sf $libpath/libandroid_support.a $libpath/libiconv.a
fi
if [ ! -f $incpath/libintl.h ]; then
    (
    mkdir -p  $incpath
    cat >  $incpath/libintl.h << __EOF
#ifndef _LIBINTL_H_ANDROID_
#define _LIBINTL_H_ANDROID_
char *gettext(const char *msgid);
char *dgettext(const char *domainname, const char *msgid);
char *dcgettext(const char *domainname, const char *msgid, int category);
#endif
__EOF
    )
fi
if [ ! -f $libpath/libintl.a ]; then
    ln -sf $libpath/libandroid_support.a $libpath/libintl.a
    ln -sf $libpath/libandroid_support.a $libpath/libpthread.a
fi

export CFLAGS="$CFLAGS -DANDROID -I$ROOT/sources/android/support/include -I$incpath"
export LDFLAGS="$LDFLAGS -L$libpath"

##
## for libffi
PWD=`pwd`
export LIBFFI_CFLAGS="-I$PWD/../libffi-3.0.11/oldbld/libffi/include"
export LIBFFI_LIBS="-L$PWD/../libffi-3.0.11/oldbld/libffi/lib -lffi"
export LDFLAGS="$LDFLAGS $LIBFFI_LIBS"


##
## start configure
mkdir -p oldbld
cd oldbld

chmod a+w $CACHE_FILE
cat > $CACHE_FILE << __EOF
glib_cv_stack_grows=no
glib_cv_uscore=no
ac_cv_func_posix_getpwuid_r=no
ac_cv_func_posix_getgrgid_r=no
__EOF
chmod a-w $CACHE_FILE

PWD=`pwd`
PREFIX=$PWD/libglib

../configure \
    --prefix=$PREFIX \
    --host=arm-linux-androideabi  \
    --cache-file=$CACHE_FILE \
    --disable-selinux --disable-fam --disable-xattr
