GLIB_TOP := $(call my-dir)/../..
LOCAL_PATH:= $(GLIB_TOP)/gthread
LIB_PATH := ../build/obj/local/armeabi-v7a


include $(CLEAR_VARS)
LOCAL_MODULE    := libglib-2.0
LOCAL_SRC_FILES := $(LIB_PATH)/libglib-2.0.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    gthread-impl.c         

LOCAL_SHARED_LIBRARIES := glib-2.0

LOCAL_MODULE:= gthread-2.0

LOCAL_CFLAGS := 		\
	-I$(LOCAL_PATH)		\
	-I$(GLIB_TOP)		\
	-I$(GLIB_TOP)/android	\
	-I$(GLIB_TOP)/glib

LOCAL_CFLAGS += \
    -DG_LOG_DOMAIN=\"GThread\"      \
    -D_POSIX4_DRAFT_SOURCE          \
    -D_POSIX4A_DRAFT10_SOURCE       \
    -U_OSF_SOURCE                   \
    -DG_DISABLE_DEPRECATED 

include $(BUILD_SHARED_LIBRARY)
