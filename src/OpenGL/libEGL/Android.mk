LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/egl
LOCAL_MODULE := libEGL_swiftshader

LOCAL_SRC_FILES += \
	../common/debug.cpp \
	Config.cpp \
	Display.cpp \
	libEGL.cpp \
	main.cpp \
	Surface.cpp

LOCAL_CFLAGS += -DLOG_TAG=\"libEGL_swiftshader\"
LOCAL_CFLAGS += -fomit-frame-pointer -ffunction-sections -fdata-sections -DNDEBUG -DANGLE_DISABLE_TRACE
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -std=c++11

LOCAL_SHARED_LIBRARIES += libdl libcutils liblog
LOCAL_LDLIBS += -lpthread -ldl

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../../

include external/stlport/libstlport.mk

include $(BUILD_SHARED_LIBRARY)
