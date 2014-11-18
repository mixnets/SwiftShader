LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/egl
LOCAL_MODULE := libGLES_CM_swiftshader

LOCAL_SRC_FILES := \
        ../common/debug.cpp

LOCAL_SRC_FILES += \
        Buffer.cpp \
        Context.cpp \
        Device.cpp \
        Framebuffer.cpp \
        HandleAllocator.cpp \
        Image.cpp \
        IndexDataManager.cpp \
        libGLES_CM.cpp \
        main.cpp \
        MatrixStack.cpp \
        RefCountObject.cpp \
        Renderbuffer.cpp \
        ResourceManager.cpp \
        Texture.cpp \
        utilities.cpp \
        VertexDataManager.cpp

LOCAL_CFLAGS += -DLOG_TAG=\"libGLES_CM_swiftshader\"
LOCAL_CFLAGS += -fomit-frame-pointer -ffunction-sections -fdata-sections -DNDEBUG -DANGLE_DISABLE_TRACE
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -std=c++11

LOCAL_SHARED_LIBRARIES += libdl libcutils liblog
LOCAL_LDLIBS += -lpthread -ldl
#LOCAL_LDFLAGS += -Wl,-v

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../../ \
	$(LOCAL_PATH)/../../Main/ \
	$(LOCAL_PATH)/../../Common/ \
	$(LOCAL_PATH)/../libGLESv2 \

include external/stlport/libstlport.mk

include $(BUILD_SHARED_LIBRARY)
