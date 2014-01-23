# platform="linux|android|ios|mac"
# thelibs="1 2 "
# target="name"
make_archive ()
{
    if [ $platform = "linux" ] || [ $platform = "android" ]; then
        rm -rf tmpobj; mkdir tmpobj
        cd tmpobj
        for lib in $thelibs; do
            lib=../$lib
            if [ ! -e $lib ]; then
                echo "Can not find $lib!"
                continue
            fi
            $AR x $lib
            $AR t $lib | xargs $AR qc lib$target.a
        done
        echo "Adding symbol table to archive."
        $AR sv lib$target.a
        mv lib$target.a ../
        cd -
        rm -rf tmpobj
    else
        libtool -static -arch_only i386 -o lib$target.a ${thelibs[@]:0}
    fi
}

# ldflags=""
make_so ()
{
    if [ $platform = "linux" ] || [ $platform = "android" ]; then
        rm -rf tmpobj; mkdir tmpobj
        cd tmpobj
        for lib in $thelibs; do
            lib=../$lib
            if [ ! -e $lib ]; then
                echo "Can not find $lib!"
                continue
            fi
            ar x $lib
        done
        echo "Generate so lib."
        $CC -shared *.o -o lib$target.so $ldflags
        mv lib$target.so ../
        cd -
        rm -rf tmpobj
    else
        libtool -dynamic -arch_only i386 -o lib$target.so ${thelibs[@]:0}
    fi
}

set_platform ()
{
    platform="android"

    ROOT=/opt/zdisk/zerox/android/android-ndk-r9
    CCROOT=$ROOT/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86
    SYSROOT=$ROOT/platforms/android-14/arch-arm/
    HOST=arm-linux-androideabi

    #AR="$CCROOT/bin/$HOST-ar --sysroot=$SYSROOT"
    AR=ar
    CC="$CCROOT/bin/$HOST-gcc --sysroot=$SYSROOT"
    CXX="$CCROOT/bin/$HOST-g++ --sysroot=$SYSROOT"
    STRIP="$CCROOT/bin/$HOST-strip --sysroot=$SYSROOT"
    #RANLIB="$CCROOT/bin/$HOST-ranlib --sysroot=$SYSROOT"
}

make_glibapi ()
{
    target="glibapi"
    ldflags="-lc -lz -lm"
    thelibs="../lib/libandroid_support.a ../lib/libffi.a ../lib/libgio-2.0.a ../lib/libglib-2.0.a ../lib/libgmodule-2.0.a ../lib/libgobject-2.0.a ../lib/libgthread-2.0.a"

    make_archive
    make_so
    mv lib$target.{a,so} ../lib/
}


make_gstapi ()
{
    target="gstapi"
    ldflags="-lc -lz -lm -lEGL -L../../lib/ -lglibapi"
    thelibs=""

    gst_libs="
libgstparse.a
libgstprintf.a
libgstreamer-1.0.a
libgstbase-1.0.a
libcheckinternal.a
libgstcheck-1.0.a
libgstcontroller-1.0.a
libgstnet-1.0.a
libgstcoreelements.a
    "
    gst_base_libs="
libgstadder.a
libgstapp.a
libgstaudioconvert.a
libgstaudiorate.a
libgstaudiotestsrc.a
libgstencodebin.a
libgstvideoconvert.a
libgstgio.a
libgstplayback.a
libgstaudioresample.a
libgstsubparse.a
libgsttcp.a
libgsttypefindfunctions.a
libgstvideotestsrc.a
libgstvideorate.a
libgstvideoscale.a
libgstvolume.a
libgstallocators-1.0.a
libgstaudio-1.0.a
libgstapp-1.0.a
libgstfft-1.0.a
libgstriff-1.0.a
libgstrtp-1.0.a
libgstrtsp-1.0.a
libgstsdp-1.0.a
libgsttag-1.0.a
libgstpbutils-1.0.a
libgstvideo-1.0.a
    "

    tmp_libs="$gst_libs $gst_base_libs"
    for lib in $tmp_libs; do
        lib=`find ../lib -name $lib`
        if [ ! -z $lib ] && [ -e $lib ]; then
            thelibs="$thelibs $lib"
        fi
    done

    make_archive
    make_so
    mv lib$target.{a,so} ../lib/
}

set_platform
make_glibapi
make_gstapi

