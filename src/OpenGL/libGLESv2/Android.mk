LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/egl
LOCAL_MODULE := libGLESv2_swiftshader

LOCAL_SRC_FILES := \
	../../Common/Configurator.cpp \
	../../Common/CPUID.cpp \
	../../Common/Debug.cpp \
	../../Common/Half.cpp \
	../../Common/Math.cpp \
	../../Common/Memory.cpp \
	../../Common/Resource.cpp \
	../../Common/Socket.cpp \
	../../Common/Thread.cpp \
	../../Common/Timer.cpp

LOCAL_SRC_FILES += \
	../common/debug.cpp

LOCAL_SRC_FILES += \
        ../compiler/AnalyzeCallDepth.cpp \
        ../compiler/Compiler.cpp \
        ../compiler/Diagnostics.cpp \
        ../compiler/DirectiveHandler.cpp \
        ../compiler/InfoSink.cpp \
        ../compiler/Initialize.cpp \
        ../compiler/InitializeDll.cpp \
        ../compiler/InitializeParseContext.cpp \
        ../compiler/IntermTraverse.cpp \
        ../compiler/Intermediate.cpp \
        ../compiler/OutputASM.cpp \
        ../compiler/ParseHelper.cpp \
        ../compiler/PoolAlloc.cpp \
        ../compiler/RemoveTree.cpp \
        ../compiler/ShaderLang.cpp \
        ../compiler/SymbolTable.cpp \
        ../compiler/TranslatorASM.cpp \
        ../compiler/ValidateLimitations.cpp \
        ../compiler/debug.cpp \
        ../compiler/glslang_lex.cpp \
        ../compiler/glslang_tab.cpp \
        ../compiler/intermOut.cpp \
        ../compiler/ossource_posix.cpp \
        ../compiler/parseConst.cpp \
        ../compiler/preprocessor/Diagnostics.cpp \
        ../compiler/preprocessor/DirectiveHandler.cpp \
        ../compiler/preprocessor/DirectiveParser.cpp \
        ../compiler/preprocessor/ExpressionParser.cpp \
        ../compiler/preprocessor/Input.cpp \
        ../compiler/preprocessor/Lexer.cpp \
        ../compiler/preprocessor/Macro.cpp \
        ../compiler/preprocessor/MacroExpander.cpp \
        ../compiler/preprocessor/Preprocessor.cpp \
        ../compiler/preprocessor/Token.cpp \
        ../compiler/preprocessor/Tokenizer.cpp \
	../compiler/util.cpp

LOCAL_SRC_FILES += \
        ../../Main/Config.cpp \
        ../../Main/FrameBuffer.cpp \
        ../../Main/FrameBufferAndroid.cpp \
        ../../Main/Logo.cpp \
        ../../Main/Register.cpp \
        ../../Main/SwiftConfig.cpp \
        ../../Main/crc.c \

LOCAL_SRC_FILES += \
	Buffer.cpp \
	Context.cpp \
	Device.cpp \
	Fence.cpp \
	Framebuffer.cpp \
	HandleAllocator.cpp \
	Image.cpp \
	IndexDataManager.cpp \
	libGLESv2.cpp \
	main.cpp \
	Program.cpp \
	Query.cpp \
	RefCountObject.cpp \
	Renderbuffer.cpp \
	ResourceManager.cpp \
	Shader.cpp \
	Texture.cpp \
	utilities.cpp \
	VertexDataManager.cpp

LOCAL_CFLAGS += -DLOG_TAG=\"libGLESv2_swiftshader\"
LOCAL_CFLAGS += -fomit-frame-pointer -ffunction-sections -fdata-sections -DNDEBUG -DANGLE_DISABLE_TRACE
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -std=c++11

LOCAL_SHARED_LIBRARIES += libdl libcutils liblog libhardware libpixelflinger libui
LOCAL_LDLIBS += -lpthread -ldl
#LOCAL_LDFLAGS += -Wl,-v

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../../ \
	$(LOCAL_PATH)/../../Main/ \
	$(LOCAL_PATH)/../../Common/

include external/stlport/libstlport.mk

include $(BUILD_SHARED_LIBRARY)
