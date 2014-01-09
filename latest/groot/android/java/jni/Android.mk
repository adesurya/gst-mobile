LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := k2player
LOCAL_SRC_FILES := \
	atomic.cpp \
	mutex.cpp \
	JNIHelp.cpp \
	JniConstants.cpp \
	MediaPlayer.cpp  \
	k2_media_MediaPlayer.cpp

LOCAL_CFLAGS += -DHAS_ATOMICS=1 -DHAVE_PTHREAD_H
LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)
