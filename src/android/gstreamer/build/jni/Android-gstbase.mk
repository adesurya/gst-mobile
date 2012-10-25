GSTREAMER_TOP := $(call my-dir)/../..
LOCAL_PATH := $(GSTREAMER_TOP)/libs/gst/base

include $(CLEAR_VARS)

GST_MAJORMINOR:= 0.10

LOCAL_SRC_FILES:= \
    gstadapter.c		\
    gstbasesink.c		\
    gstbasesrc.c		\
    gstbasetransform.c	\
    gstcollectpads.c	\
    gstpushsrc.c		\
    gsttypefindhelper.c	\
    gstdataqueue.c

LOCAL_STATIC_LIBRARIES := \
    gstreamer-0.10       \
    glib-2.0             \
    gthread-2.0          \
    gmodule-2.0          \
    gobject-2.0

LOCAL_MODULE:= gstbase-$(GST_MAJORMINOR)

#LOCAL_TOP_PATH := $(LOCAL_PATH)/../../../..

LOCAL_CFLAGS := 			\
    -I$(LOCAL_PATH)   			\
    -I$(GSTREAMER_TOP)       		\
    -I$(GSTREAMER_TOP)/android   	\
    -I$(GSTREAMER_TOP)/gst		\
    -I$(GSTREAMER_TOP)/gst/android	\
    -I$(GSTREAMER_TOP)/libs 		\
    -I$(GSTREAMER_TOP)/../glib  	\
    -I$(GSTREAMER_TOP)/../glib/android 	\
    -I$(GSTREAMER_TOP)/../glib/glib  	\
    -I$(GSTREAMER_TOP)/../glib/gmodule 	\
    -I$(GSTREAMER_TOP)/../glib/gobject 	\
    -I$(GSTREAMER_TOP)/../glib/gthread

LOCAL_CFLAGS += \
    -DHAVE_CONFIG_H			

include $(BUILD_STATIC_LIBRARY)
