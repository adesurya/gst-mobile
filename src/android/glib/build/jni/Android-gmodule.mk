GLIB_TOP := $(call my-dir)/../..
LOCAL_PATH := $(GLIB_TOP)/gmodule
LIB_PATH := ../build/obj/local/armeabi-v7a


include $(CLEAR_VARS)
LOCAL_MODULE    := libglib-2.0
LOCAL_SRC_FILES := $(LIB_PATH)/libglib-2.0.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    gmodule.c    

LOCAL_LDLIBS := -ldl

LOCAL_SHARED_LIBRARIES := glib-2.0

LOCAL_MODULE:= gmodule-2.0

LOCAL_CFLAGS := 		\
	-I$(GLIB_TOP)		\
	-I$(GLIB_TOP)/android	\
	-I$(GLIB_TOP)/glib	\
	-I$(LOCAL_PATH)/android

LOCAL_CFLAGS += \
    -DG_LOG_DOMAIN=\"GModule\"      \
    -DG_DISABLE_DEPRECATED 

include $(BUILD_SHARED_LIBRARY)
