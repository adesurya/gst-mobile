#!/bin/sh

PWD=`pwd`/..

cd $PWD/libffi-3.0.11
sh run_cfg.sh

cd $PWD/glib2.0-2.32.1  
sh run_cfg.sh

cd $PWD/gstreamer-1.2.2  
sh run_cfg.sh

cd $PWD/gst-plugins-base-1.2.2  
sh run_cfg.sh

cd $PWD/gst-plugins-good-1.2.2  
sh run_cfg.sh

cd $PWD/gst-plugins-ugly-1.2.2  
sh run_cfg.sh

cd $PWD/gst-plugins-bad-1.2.2  
sh run_cfg.sh

exit 0
