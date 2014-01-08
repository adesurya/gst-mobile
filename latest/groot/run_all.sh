#!/bin/sh

PWD=`pwd`/..

####################
cd $PWD/libffi-3.0.11
sh run_cfg.sh

cd $PWD/glib2.0-2.32.1  
sh run_cfg.sh


####################
gst_ver=1.2.2

cd $PWD/gstreamer-$gst_ver
sh run_cfg.sh

cd $PWD/gst-plugins-base-$gst_ver
sh run_cfg.sh

cd $PWD/gst-plugins-good-$gst_ver
sh run_cfg.sh

cd $PWD/gst-plugins-ugly-$gst_ver
sh run_cfg.sh

cd $PWD/gst-plugins-bad-$gst_ver
sh run_cfg.sh

exit 0
