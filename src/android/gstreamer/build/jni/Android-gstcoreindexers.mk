GSTREAMER_TOP := $(call my-dir)/../..
EXTERNAL := $(GSTREAMER_TOP)/..
LOCAL_PATH := $(GSTREAMER_TOP)/plugins/indexers

include $(CLEAR_VARS)

GST_MAJORMINOR:= 0.10

LOCAL_SRC_FILES:= \
    gstindexers.c   \
    gstmemindex.c

LOCAL_SHARED_LIBRARIES := \
    libgstbase-0.10       \
    libgstreamer-0.10       \
    libglib-2.0             \
    libgthread-2.0          \
    libgmodule-2.0          \
    libgobject-2.0

LOCAL_MODULE:= gstcoreindexers

LOCAL_C_INCLUDES := 			\
	$(EXTERNAL)/gstreamer       	\
	$(EXTERNAL)/gstreamer/android      \
	$(EXTERNAL)/gstreamer/libs 	\
	$(EXTERNAL)/gstreamer/gst		\
	$(EXTERNAL)/gstreamer/gst/android	\
	$(EXTERNAL)/glib   		\
	$(EXTERNAL)/glib/android   	\
	$(EXTERNAL)/glib/glib   		\
	$(EXTERNAL)/glib/gmodule   	\
	$(EXTERNAL)/glib/gobject  		\
	$(EXTERNAL)/glib/gthread

LOCAL_CFLAGS := \
	-DHAVE_CONFIG_H			

#include $(BUILD_PLUGIN_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
