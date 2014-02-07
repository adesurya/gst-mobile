LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := glibapi
LOCAL_SRC_FILES := ../../../lib/libglibapi.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := gstapi
LOCAL_SRC_FILES := ../../../lib/libgstapi.so
include $(PREBUILT_SHARED_LIBRARY)


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

LOCAL_CFLAGS += -DHAS_ATOMICS=1 -DHAVE_PTHREAD_H #-DNDEBUG
LOCAL_CFLAGS += -I../../../include/gstreamer-1.0 -I../../../include/glib-2.0 -I../../../lib/glib-2.0/include
LOCAL_LDLIBS += -llog -landroid
#LOCAL_LDFLAGS += -L../../../lib -lglibapi -lgstapi
#LOCAL_LDFLAGS += -lffi -landroid_support

LOCAL_SHARED_LIBRARIES := glibapi gstapi

include $(BUILD_SHARED_LIBRARY)
