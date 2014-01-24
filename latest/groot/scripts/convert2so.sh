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
    ldflags="-lc -lz -lm -lEGL -L../../lib/ -lglibapi -lvorbisenc -lvorbis -logg"
    thelibs=""
    ldflags+=" /tmp/gst_static_plugins.c"
cat > /tmp/gst_static_plugins.c << EOF
#define GST_PLUGIN(n) {extern void gst_plugin_##n##_register(void); gst_plugin_##n##_register();}

void gst_static_plugins()
{
    /* For gstreamer */
    GST_PLUGIN(coreelements);

    /* For gst-plugin-base */
    GST_PLUGIN(adder);
    GST_PLUGIN(app);
    GST_PLUGIN(audioconvert);
    GST_PLUGIN(audiorate);
    GST_PLUGIN(audiotestsrc);
    GST_PLUGIN(encoding);
    GST_PLUGIN(videoconvert);
    GST_PLUGIN(gio);
    GST_PLUGIN(playback);
    GST_PLUGIN(audioresample);
    GST_PLUGIN(subparse);
    GST_PLUGIN(tcp);
    GST_PLUGIN(typefindfunctions);
    GST_PLUGIN(videotestsrc);
    GST_PLUGIN(videorate);
    GST_PLUGIN(videoscale);
    GST_PLUGIN(volume);
    GST_PLUGIN(ogg);
    GST_PLUGIN(vorbis);

    /* For gst-plugin-base */
    GST_PLUGIN(alpha);
    GST_PLUGIN(alphacolor);
    GST_PLUGIN(apetag);
    GST_PLUGIN(audiofx);
    GST_PLUGIN(audioparsers);
    GST_PLUGIN(auparse);
    GST_PLUGIN(autodetect);
    GST_PLUGIN(avi);
    GST_PLUGIN(cutter);
    GST_PLUGIN(deinterlace);
    GST_PLUGIN(dtmf);
    GST_PLUGIN(debug);
    GST_PLUGIN(navigationtest);
    GST_PLUGIN(effectv);
    GST_PLUGIN(equalizer);
    GST_PLUGIN(flv);
    GST_PLUGIN(goom);
    GST_PLUGIN(goom2k1);
    GST_PLUGIN(id3demux);
    GST_PLUGIN(icydemux);
    GST_PLUGIN(imagefreeze);
    GST_PLUGIN(interleave);
    GST_PLUGIN(isomp4);
    GST_PLUGIN(alaw);
    GST_PLUGIN(mulaw);
    GST_PLUGIN(level);
    GST_PLUGIN(matroska);
    GST_PLUGIN(multifile);
    GST_PLUGIN(multipart);
    GST_PLUGIN(replaygain);
    GST_PLUGIN(rtp);
    GST_PLUGIN(rtpmanager);
    GST_PLUGIN(rtsp);
    GST_PLUGIN(shapewipe);
    GST_PLUGIN(smpte);
    GST_PLUGIN(spectrum);
    GST_PLUGIN(udp);
    GST_PLUGIN(videobox);
    GST_PLUGIN(videocrop);
    GST_PLUGIN(videofilter);
    /* GST_PLUGIN(videomixer); */
    GST_PLUGIN(wavenc);
    GST_PLUGIN(wavparse);
    GST_PLUGIN(flxdec);
    GST_PLUGIN(y4menc);
    GST_PLUGIN(oss4);
}
EOF

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
libgstogg.a
libgstvorbis.a
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

    gst_good_libs="
libparser.a
libgstalpha.a
libgstalphacolor.a
libgstapetag.a
libgstaudiofx.a
libgstaudioparsers.a
libgstauparse.a
libgstautodetect.a
libgstavi.a
libgstcutter.a
libgstdeinterlace.a
libgstdtmf.a
libgstdebug.a
libgstnavigationtest.a
libgsteffectv.a
libgstequalizer.a
libgstflv.a
libgstgoom.a
libgstgoom2k1.a
libgstid3demux.a
libgsticydemux.a
libgstimagefreeze.a
libgstinterleave.a
libgstisomp4.a
libgstalaw.a
libgstmulaw.a
libgstlevel.a
libgstmatroska.a
libgstmultifile.a
libgstmultipart.a
libgstreplaygain.a
libgstrtp.a
libgstrtpmanager.a
libgstrtsp.a
libgstshapewipe.a
libgstsmpte.a
libgstspectrum.a
libgstudp.a
libgstvideobox.a
libgstvideocrop.a
libgstvideofilter.a
libgstwavenc.a
libgstwavparse.a
libgstflxdec.a
libgsty4menc.a
libgstoss4audio.a
"
#libgstvideomixer.a conflict with libgstvideoconvert.a

    tmp_libs="$gst_libs $gst_base_libs $gst_good_libs"
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

