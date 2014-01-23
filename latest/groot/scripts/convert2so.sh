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
        $CC -shared *.o -o lib$target.so
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
    thelibs="../lib/libgio-2.0.a ../lib/libglib-2.0.a ../lib/libgmodule-2.0.a ../lib/libgobject-2.0.a ../lib/libgthread-2.0.a"

    make_archive
    make_so
    mv lib$target.{a,so} ../lib/
}


make_gstapi ()
{
    target="gstapi"
    thelibs="
    ../lib/libgstallocators-1.0.a        ../lib/libgstcodecparsers-1.0.a  ../lib/libgstnet-1.0.a          ../lib/libgstrtsp-1.0.a        
    ../lib/libgstapp-1.0.a               ../lib/libgstcontroller-1.0.a    ../lib/libgstpbutils-1.0.a      ../lib/libgstsdp-1.0.a          
    ../lib/libgstaudio-1.0.a             ../lib/libgstegl-1.0.a           ../lib/libgstphotography-1.0.a  ../lib/libgsttag-1.0.a           
    ../lib/libgstbase-1.0.a              ../lib/libgstfft-1.0.a           ../lib/libgstreamer-1.0.a       ../lib/libgsturidownloader-1.0.a  
    ../lib/libgstbasecamerabinsrc-1.0.a  ../lib/libgstinsertbin-1.0.a     ../lib/libgstriff-1.0.a         ../lib/libgstvideo-1.0.a
    ../lib/libgstcheck-1.0.a             ../lib/libgstmpegts-1.0.a        ../lib/libgstrtp-1.0.a          
    "

    make_archive
    make_so
    mv lib$target.{a,so} ../lib/
}

set_platform
make_glibapi
make_gstapi

