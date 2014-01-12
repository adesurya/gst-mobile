#!/bin/sh

PWD=`pwd`/..

####################
(
cd $PWD/libffi-3.0.11
sh run_cfg.sh > /tmp/bld_ffi.log
)

(
cd $PWD/glib2.0-2.32.1  
sh run_cfg.sh > /tmp/bld_glib2.log
)


####################
gst_ver=1.2.2

(
cd $PWD/gstreamer-$gst_ver
sh run_cfg.sh > /tmp/bld_gst.log
)

(
cd $PWD/gst-plugins-base-$gst_ver
sh run_cfg.sh > /tmp/bld_gstbase.log
)

(
cd $PWD/gst-plugins-good-$gst_ver
sh run_cfg.sh > /tmp/bld_gstgood.log
)

(
cd $PWD/gst-plugins-ugly-$gst_ver
sh run_cfg.sh > /tmp/bld_gstugly.log
)

(
cd $PWD/gst-plugins-bad-$gst_ver
sh run_cfg.sh > /tmp/bld_gstbad.log
)

exit 0
