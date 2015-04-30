LOCAL_PATH := $(call my-dir)

COMMON_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libcutils

COMMON_STATIC_LIBRARIES := \
    libgceframebufferconfig \
    libgcemetadata \
    libyuv_static

COMMON_SRC_FILES := 	\
	gralloc.cpp 	\
	framebuffer.cpp \
	mapper.cpp

COMMON_CFLAGS:= -DLOG_TAG=\"gralloc_gce_x86\" -Wno-missing-field-initializers

COMMON_C_INCLUDES := \
    device/google/gce/include \
    external/libyuv/files/include

include $(CLEAR_VARS)

LOCAL_MODULE_PATH := vendor/transgaming/swiftshader/$(TARGET_ARCH)/debug
LOCAL_MODULE := gralloc_swiftshader.gce_x86_vendor_debug
LOCAL_INSTALLED_MODULE_STEM := gralloc.gce_x86.so
LOCAL_CFLAGS += $(COMMON_CFLAGS) -UNDEBUG -g -O0

LOCAL_SHARED_LIBRARIES += $(COMMON_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES += $(COMMON_STATIC_LIBRARIES)
LOCAL_SRC_FILES += $(COMMON_SRC_FILES)
LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE_PATH := vendor/transgaming/swiftshader/$(TARGET_ARCH)/release
LOCAL_MODULE := gralloc_swiftshader.gce_x86_vendor_release
LOCAL_INSTALLED_MODULE_STEM := gralloc.gce_x86.so
LOCAL_CFLAGS += $(COMMON_CFLAGS)

LOCAL_SHARED_LIBRARIES += $(COMMON_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES += $(COMMON_STATIC_LIBRARIES)
LOCAL_SRC_FILES += $(COMMON_SRC_FILES)
LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)
