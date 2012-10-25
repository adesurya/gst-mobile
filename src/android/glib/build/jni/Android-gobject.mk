GLIB_TOP := $(call my-dir)/../..
LOCAL_PATH:= $(GLIB_TOP)/gobject
LIB_PATH := ../build/obj/local/armeabi-v7a


include $(CLEAR_VARS)
LOCAL_MODULE    := libglib-2.0
LOCAL_SRC_FILES := $(LIB_PATH)/libglib-2.0.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libgmodule-2.0
LOCAL_SRC_FILES := $(LIB_PATH)/libgmodule-2.0.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libgthread-2.0
LOCAL_SRC_FILES := $(LIB_PATH)/libgthread-2.0.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    gboxed.c                \
    genums.c                \
    gparam.c                \
    gsignal.c               \
    gtypemodule.c           \
    gtypeplugin.c           \
    gvalue.c                \
    gvaluetypes.c           \
    gclosure.c              \
    gobject.c               \
    gparamspecs.c           \
    gtype.c                 \
    gvaluearray.c           \
    gvaluetransform.c       \
    gsourceclosure.c

LOCAL_SHARED_LIBRARIES := glib-2.0 gmodule-2.0 gthread-2.0

LOCAL_MODULE:= gobject-2.0

LOCAL_CFLAGS := 		\
	-I$(LOCAL_PATH)		\
	-I$(GLIB_TOP)		\
	-I$(GLIB_TOP)/android	\
	-I$(GLIB_TOP)/glib

LOCAL_CFLAGS += \
    -DG_LOG_DOMAIN=\"GLib-GObject\" \
    -DGOBJECT_COMPILATION           \
    -DG_DISABLE_CONST_RETURNS       \
    -DG_DISABLE_DEPRECATED 

include $(BUILD_SHARED_LIBRARY)
