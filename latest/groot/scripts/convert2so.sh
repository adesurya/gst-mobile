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

    [ -f lib$target.a ] && mv lib$target.a ../lib/
}

# ldflags=""
make_so ()
{
    if [ $platform = "linux" ] || [ $platform = "android" ]; then
        echo "Generate lib$target.so ..."
        $CC -shared -o lib$target.so -Wl,-whole-archive $thelibs -Wl,-no-whole-archive $ldflags
        $CC -DTEST_PRIV_API -o /tmp/test_$target -L. -l$target $ldflags 
    else
        libtool -dynamic -arch_only i386 -o lib$target.so ${thelibs[@]:0}
    fi

    [ -f lib$target.so ] && mv lib$target.so ../lib/
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
    ldflags=" /tmp/test_glibapi.c"
    ldflags+=" -lc -lz -lm"
cat > /tmp/test_glibapi.c << EOF
#ifdef TEST_PRIV_API
int main(int argc, char *argv[])
{
    return 0;
}
#endif
EOF
    thelibs="../lib/libandroid_support.a ../lib/libffi.a ../lib/libgio-2.0.a ../lib/libglib-2.0.a ../lib/libgmodule-2.0.a ../lib/libgobject-2.0.a ../lib/libgthread-2.0.a"

    make_archive
    make_so
}

make_gstapi ()
{
    target="gstapi"
    ldflags=" /tmp/gst_static_plugins.c"
    ldflags+=" -lc -lz -lm -lEGL -lGLESv2 -lOpenSLES"
    ldflags+=" -L../lib -lglibapi -lvorbisenc -lvorbis -ltheora -logg"
cat > /tmp/gst_static_plugins.c << EOF

static int s_plugins_num = 0;
#define GST_PLUGIN(n) {extern void gst_plugin_##n##_register(void); gst_plugin_##n##_register();s_plugins_num++;}

#ifdef TEST_PRIV_API
int main(int argc, char *argv[])
#else
int gst_static_plugins()
#endif
{
    /* For gstreamer */
    GST_PLUGIN(coreelements);

    /* For gst-plugin-base */
    GST_PLUGIN(adder);
    GST_PLUGIN(app);
    GST_PLUGIN(audioconvert);
    GST_PLUGIN(audiorate);
    /* GST_PLUGIN(audiotestsrc); */
    GST_PLUGIN(encoding);
    GST_PLUGIN(videoconvert);
    GST_PLUGIN(gio);
    GST_PLUGIN(playback);
    GST_PLUGIN(audioresample);
    GST_PLUGIN(subparse);
    GST_PLUGIN(tcp);
    GST_PLUGIN(typefindfunctions);
    /* GST_PLUGIN(videotestsrc); */
    GST_PLUGIN(videorate);
    GST_PLUGIN(videoscale);
    GST_PLUGIN(volume);
    GST_PLUGIN(ogg);
    GST_PLUGIN(theora);
    GST_PLUGIN(vorbis);

    /* For gst-plugin-good */
    GST_PLUGIN(alpha);
    GST_PLUGIN(alphacolor);
    /* GST_PLUGIN(apetag); */
    GST_PLUGIN(audiofx);
    GST_PLUGIN(audioparsers);
    GST_PLUGIN(auparse);
    GST_PLUGIN(autodetect);
    GST_PLUGIN(avi);
    GST_PLUGIN(cutter);
    //GST_PLUGIN(deinterlace);
    GST_PLUGIN(dtmf);
    GST_PLUGIN(debug);
    /* GST_PLUGIN(navigationtest); */
    GST_PLUGIN(effectv);
    GST_PLUGIN(equalizer);
    GST_PLUGIN(flv);
    GST_PLUGIN(goom);
    GST_PLUGIN(goom2k1);
    GST_PLUGIN(id3demux);
    //GST_PLUGIN(icydemux);
    GST_PLUGIN(imagefreeze);
    //GST_PLUGIN(interleave);
    GST_PLUGIN(isomp4);
    GST_PLUGIN(alaw);
    GST_PLUGIN(mulaw);
    GST_PLUGIN(level);
    GST_PLUGIN(matroska);
    //GST_PLUGIN(multifile);
    //GST_PLUGIN(multipart);
    GST_PLUGIN(replaygain);
    GST_PLUGIN(rtp);
    GST_PLUGIN(rtpmanager);
    GST_PLUGIN(rtsp);
    //GST_PLUGIN(shapewipe);
    //GST_PLUGIN(smpte);
    //GST_PLUGIN(spectrum);
    GST_PLUGIN(udp);
    GST_PLUGIN(videobox);
    GST_PLUGIN(videocrop);
    GST_PLUGIN(videofilter);
    /* GST_PLUGIN(videomixer); */
    GST_PLUGIN(wavenc);
    GST_PLUGIN(wavparse);
    GST_PLUGIN(flxdec);
    /* GST_PLUGIN(y4menc); */
    /* GST_PLUGIN(oss4); */

    /* For gst-plugin-ugly */
    GST_PLUGIN(asf);
    GST_PLUGIN(dvdlpcmdec);
    GST_PLUGIN(dvdsub);
    GST_PLUGIN(realmedia);
    /* GST_PLUGIN(xingmux); */

    /* For gst-plugin-bad */
    /* GST_PLUGIN(accurip); */
    GST_PLUGIN(adpcmdec);
    GST_PLUGIN(adpcmenc);
    GST_PLUGIN(aiff);
    GST_PLUGIN(asfmux);
    GST_PLUGIN(audiofxbad);
    GST_PLUGIN(audiovisualizers);
    GST_PLUGIN(autoconvert);
    /* GST_PLUGIN(bayer); */
    //GST_PLUGIN(camerabin);
    GST_PLUGIN(coloreffects);
    GST_PLUGIN(dataurisrc);
    /* GST_PLUGIN(debugutilsbad); */
    GST_PLUGIN(dvbsuboverlay);
    GST_PLUGIN(dvdspu);
    /* GST_PLUGIN(festival); */
    /* GST_PLUGIN(fieldanalysis); */
    /* GST_PLUGIN(freeverb); */
    /* GST_PLUGIN(frei0r); */
    /* GST_PLUGIN(gaudieffects); */
    GST_PLUGIN(geometrictransform);
    GST_PLUGIN(gdp);
    GST_PLUGIN(id3tag);
    //GST_PLUGIN(inter);
    //GST_PLUGIN(interlace);
    //GST_PLUGIN(ivtc);
    GST_PLUGIN(jpegformat);
    GST_PLUGIN(rfbsrc);
    GST_PLUGIN(liveadder);
    GST_PLUGIN(midi);
    GST_PLUGIN(mpegpsdemux);
    GST_PLUGIN(mpegtsdemux);
    GST_PLUGIN(mpegtsmux);
    GST_PLUGIN(mpegpsmux);
    /* GST_PLUGIN(mxf); */
    /* GST_PLUGIN(pcapparse); */
    /* GST_PLUGIN(pnm); */
    /* GST_PLUGIN(rawparse); */
    //GST_PLUGIN(removesilence);
    GST_PLUGIN(sdp);
    //GST_PLUGIN(segmentclip);
    /* GST_PLUGIN(gstsiren); */
    GST_PLUGIN(smooth);
    GST_PLUGIN(speed);
    GST_PLUGIN(subenc);
    GST_PLUGIN(videofiltersbad);
    GST_PLUGIN(videoparsersbad);
    /* GST_PLUGIN(y4mdec); */
    /* GST_PLUGIN(yadif); */
    /* GST_PLUGIN(androidmedia); */
    GST_PLUGIN(fbdevsink);
    GST_PLUGIN(opensles);
    GST_PLUGIN(eglglessink);

    return s_plugins_num;
}
EOF

    gst_libs="
libgstreamer-1.0.a
libgstbase-1.0.a
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
#libgstaudiotestsrc.a
libgstencodebin.a
libgstvideoconvert.a
libgstgio.a
libgstplayback.a
libgstaudioresample.a
libgstsubparse.a
libgsttcp.a
libgsttypefindfunctions.a
#libgstvideotestsrc.a
libgstvideorate.a
libgstvideoscale.a
libgstvolume.a
libgstogg.a
libgsttheora.a
libgstvorbis.a
##libgstallocators-1.0.a
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
libgstalpha.a
libgstalphacolor.a
#libgstapetag.a
libgstaudiofx.a
libgstaudioparsers.a
libgstauparse.a
libgstautodetect.a
libgstavi.a
libgstcutter.a
#libgstdeinterlace.a
libgstdtmf.a
libgstdebug.a
#libgstnavigationtest.a
libgsteffectv.a
libgstequalizer.a
libgstflv.a
libgstgoom.a
libgstgoom2k1.a
libgstid3demux.a
#libgsticydemux.a
libgstimagefreeze.a
#libgstinterleave.a
libgstisomp4.a
libgstalaw.a
libgstmulaw.a
libgstlevel.a
libgstmatroska.a
#libgstmultifile.a
#libgstmultipart.a
libgstreplaygain.a
libgstrtp.a
libgstrtpmanager.a
libgstrtsp.a
#libgstshapewipe.a
#libgstsmpte.a
#libgstspectrum.a
libgstudp.a
libgstvideobox.a
libgstvideocrop.a
libgstvideofilter.a
#libgstvideomixer.a
libgstwavenc.a
libgstwavparse.a
libgstflxdec.a
#libgsty4menc.a
#libgstoss4audio.a
"

gst_ugly_libs="
libgstasf.a
libgstdvdlpcmdec.a
libgstdvdsub.a
libgstrmdemux.a
#libgstxingmux.a
"

gst_bad_libs="
#libgstaccurip.a
libgstadpcmdec.a
libgstadpcmenc.a
libgstaiff.a
libgstasfmux.a
libgstaudiofxbad.a
libgstaudiovisualizers.a
libgstautoconvert.a
#libgstbayer.a
#libgstcamerabin2.a
libgstcoloreffects.a
libgstdataurisrc.a
#libgstdebugutilsbad.a
libgstdvbsuboverlay.a
libgstdvdspu.a
#libgstfestival.a
#libgstfieldanalysis.a
#libgstfreeverb.a
#libgstfrei0r.a
#libgstgaudieffects.a
libgstgeometrictransform.a
libgstgdp.a
libgstid3tag.a
#libgstinter.a
#libgstinterlace.a
#libgstivtc.a
libgstjpegformat.a
libgstrfbsrc.a
libgstliveadder.a
libgstmidi.a
libgstmpegpsdemux.a
libgstmpegtsdemux.a
libgstmpegtsmux.a
libgstmpegpsmux.a
#libgstmxf.a
#libgstpcapparse.a
#libgstpnm.a
#libgstrawparse.a
#libgstremovesilence.a
libgstsdpelem.a
#libgstsegmentclip.a
#libgstsiren.a
libgstsmooth.a
libgstspeed.a
libgstsubenc.a
libgstvideofiltersbad.a
libgstvideoparsersbad.a
#libgsty4mdec.a
#libgstyadif.a
#libgstbasecamerabinsrc-1.0.a
libgstegl-1.0.a
libgstinsertbin-1.0.a
#libgstphotography-1.0.a
libgstcodecparsers-1.0.a
libgstmpegts-1.0.a
libgsturidownloader-1.0.a
#libgstandroidmedia.a
libgstfbdevsink.a
libgstopensles.a
libgsteglglessink.a
"

    thelibs=""
    tmp_libs="$gst_libs $gst_base_libs $gst_good_libs $gst_ugly_libs $gst_bad_libs"
    for f in $tmp_libs; do
        lib=`find ../lib -name $f`
        if [ ! -z $lib ] && [ -e $lib ]; then
            thelibs="$thelibs $lib"
        else
            echo "Error: $f is nexist!"
        fi
    done

    make_archive
    make_so
}


set_platform
make_glibapi
make_gstapi
exit 0
