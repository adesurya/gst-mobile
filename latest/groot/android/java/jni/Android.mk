LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := k2player
LOCAL_SRC_FILES := \
	atomic.cpp \
	mutex.cpp \
	JNIHelp.cpp \
	JniConstants.cpp \
	gstapi.cpp \
	MediaPlayer.cpp  \
	k2_media_MediaPlayer.cpp

LOCAL_CFLAGS += -DHAS_ATOMICS=1 -DHAVE_PTHREAD_H 
LOCAL_CFLAGS += -I../../../include/gstreamer-1.0 -I../../../include/glib-2.0 -I../../../lib/glib-2.0/include
LOCAL_LDLIBS += -llog
LOCAL_LDFLAGS += -L../../../lib -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0

include $(BUILD_SHARED_LIBRARY)
