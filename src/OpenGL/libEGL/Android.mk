LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CLANG := true

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/egl
LOCAL_MODULE := libEGL_swiftshader

LOCAL_SRC_FILES += \
	../common/Object.cpp \
	../common/debug.cpp \
	Config.cpp \
	Display.cpp \
	Surface.cpp \
	libEGL.cpp \
	main.cpp

LOCAL_CFLAGS += -DLOG_TAG=\"libEGL_swiftshader\"
LOCAL_CFLAGS += -DNDEBUG -DANGLE_DISABLE_TRACE
LOCAL_CFLAGS += -std=c++11

LOCAL_SHARED_LIBRARIES += libdl liblog libandroid
LOCAL_LDFLAGS += -Wl,--version-script=$(LOCAL_PATH)/exports.map -Wl,--hash-style=sysv

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../../

include external/stlport/libstlport.mk

include $(BUILD_SHARED_LIBRARY)
