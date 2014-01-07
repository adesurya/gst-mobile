ROOT=/opt/zdisk/zerox/android/android-ndk-r9

CCROOT=$ROOT/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86
SYSROOT=$ROOT/platforms/android-14/arch-arm/
HOST=arm-linux-androideabi

export CC="$CCROOT/bin/$HOST-gcc --sysroot=$SYSROOT"
export CXX="$CCROOT/bin/$HOST-g++ --sysroot=$SYSROOT"
export STRIP="$CCROOT/bin/$HOST-strip --sysroot=$SYSROOT"
export CFLAGS=""
export LDFLAGS=""

##
## android support sources
jnipath=andriod/support/jni
libpath=`pwd`/$jnipath/../obj/local/armeabi-v7a
incpath=`pwd`/$jnipath/../include
if [ ! -f $libpath/libandroid_support.a ]; then
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
fi
if [ ! -f $libpath/libiconv.a ]; then
    ln -sf $libpath/libandroid_support.a $libpath/libiconv.a
fi
if [ ! -f $incpath/libintl.h ]; then
    mkdir -p  $incpath
    cat >  $incpath/libintl.h << __EOF
#ifndef _LIBINTL_H_ANDROID_
#define _LIBINTL_H_ANDROID_
char *gettext(const char *msgid);
char *dgettext(const char *domainname, const char *msgid);
char *dcgettext(const char *domainname, const char *msgid, int category);
#endif
__EOF
fi
if [ ! -f $libpath/libintl.a ]; then
    ln -sf $libpath/libandroid_support.a $libpath/libintl.a
fi

export CFLAGS="$CFLAGS -I$ROOT/sources/android/support/include -I$incpath"
export LDFLAGS="$LDFLAGS -L$libpath"

##
## start configure
mkdir -p oldbld
cd oldbld
../configure \
    --host=arm-linux-androideabi  \
    --disable-selinux --disable-fam --disable-xattr
