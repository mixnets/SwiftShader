cmake_minimum_required(VERSION 2.8)

project(SwiftShaders CXX)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The type of build: Debug Release RelWithDebInfo." )

option(BUILD_EGL "Build the EGL library" 1)
option(BUILD_GLESv2 "Build the OpenGL ES 2 library" 1)
option(BUILD_GLES_CM "Build the OpenGL ES 1.1 library" 1)
option(BUILD_RAD "Build the Radiance library" 1)

###########################################################
# Detect system
###########################################################

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(LINUX ON)
else()
    message(FATAL_ERROR "Platform is not supported")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH "x86_64")
else()
    set(ARCH "x86")
endif()

###########################################################
# Directories
###########################################################

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(OPENGL_DIR ${SOURCE_DIR}/OpenGL)
set(OPENGL_COMPILER_DIR ${OPENGL_DIR}/compiler)
set(RADIANCE_DIR ${SOURCE_DIR}/Radiance)
set(LLVM_DIR ${SOURCE_DIR}/LLVM)
set(TESTS_DIR ${CMAKE_SOURCE_DIR}/tests)

###########################################################
# Compile flags
###########################################################

macro( set_cpp_flag FLAG )
    if( ${ARGC} GREATER 1 )
        set( CMAKE_CXX_FLAGS_${ARGV1} "${CMAKE_CXX_FLAGS_${ARGV1}} ${FLAG}" )
    else()
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}" )
    endif()
endmacro()

if (LINUX OR APPLE)
    set_cpp_flag("--std=c++11")
    set_cpp_flag("-Wall")
    set_cpp_flag("-fexceptions")

    # Remove xor, and, or and friends from the list of keywords, they are used
    # by Reactor
    set_cpp_flag("-fno-operator-names")

    # LLVM headers requires these flags to be set
    set_cpp_flag("-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS")

    if(ARCH EQUAL "x86")
        set_cpp_flag("-m32")
        set_cpp_flag("-msse2")
        set_cpp_flag("-march=i686")
    endif()
    if(ARCH EQUAL "x86_64")
        set_cpp_flag("-m64")
        set_cpp_flag("-fPIC")
        set_cpp_flag("-march=core2")
    endif()

    # Use -g3 to have even more debug info
    set_cpp_flag("-g -g3" DEBUG)
    set_cpp_flag("-g -g3" RELWITHDEBINFO)
    set_cpp_flag("-s" RELEASE)

    # For distribution it is more important to be slim than super optimized
    # so even in Release we us only -O2
    set_cpp_flag("-O2" RELEASE)
    set_cpp_flag("-O2" RELWITHDEBINFO)

    set_cpp_flag("-DNDEBUG" RELEASE)
    set_cpp_flag("-DNDEBUG" RELWITHDEBINFO)
    set_cpp_flag("-DANGLE_DISABLE_TRACE" RELEASE)
    set_cpp_flag("-DANGLE_DISABLE_TRACE" RELWITHDEBINFO)

    # Put each variable and function in its own section so that when linking
    # with -gc-sections unused functions and variables are removed.
    set_cpp_flag("-ffunction-sections" RELEASE)
    set_cpp_flag("-fdata-sections" RELEASE)
    set_cpp_flag("-fomit-frame-pointer" RELEASE)
endif()

###########################################################
# LLVM
###########################################################

# We want the code for all of LLVM except the code for non-X86 targets
file(GLOB_RECURSE LLVM_LIST ${LLVM_DIR}/lib/*.cpp)
file(GLOB_RECURSE LLVM_ALL_TARGET_LIST ${LLVM_DIR}/lib/Target/*.cpp)
file(GLOB_RECURSE LLVM_X86_TARGET_LIST ${LLVM_DIR}/lib/Target/X86/*.cpp)

list(REMOVE_ITEM LLVM_LIST ${LLVM_ALL_TARGET_LIST})
list(APPEND LLVM_LIST ${LLVM_X86_TARGET_LIST})
list(REMOVE_ITEM LLVM_LIST "${LLVM_DIR}/lib/Target/X86/X86MCInstLower.cpp")
list(APPEND LLVM_LIST
    ${LLVM_DIR}/lib/Target/Mangler.cpp
    ${LLVM_DIR}/lib/Target/TargetData.cpp
    ${LLVM_DIR}/lib/Target/TargetELFWriterInfo.cpp
    ${LLVM_DIR}/lib/Target/TargetFrameLowering.cpp
    ${LLVM_DIR}/lib/Target/TargetInstrInfo.cpp
    ${LLVM_DIR}/lib/Target/TargetLibraryInfo.cpp
    ${LLVM_DIR}/lib/Target/TargetLoweringObjectFile.cpp
    ${LLVM_DIR}/lib/Target/TargetMachine.cpp
    ${LLVM_DIR}/lib/Target/TargetRegisterInfo.cpp
    ${LLVM_DIR}/lib/Target/TargetSubtargetInfo.cpp
)

if(LINUX)
    set(LLVM_INCLUDE_DIR ${LLVM_DIR}/include-linux)
endif()

list(APPEND LLVM_INCLUDE_DIR
    ${LLVM_DIR}/include
    ${LLVM_DIR}/lib/Target/X86
)
add_library(llvm STATIC ${LLVM_LIST})
set_target_properties(llvm PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(llvm PROPERTIES INCLUDE_DIRECTORIES "${LLVM_INCLUDE_DIR}")

###########################################################
# Include Directories
###########################################################

set(COMMON_INCLUDE_DIR
    ${SOURCE_DIR}
    ${SOURCE_DIR}/Common
    ${SOURCE_DIR}/Main
    ${SOURCE_DIR}/Reactor
    ${SOURCE_DIR}/Renderer
    ${SOURCE_DIR}/Shader
    ${LLVM_INCLUDE_DIR}
)
set(OPENGL_INCLUDE_DIR
    ${OPENGL_DIR}
    ${OPENGL_DIR}/include
    ${COMMON_INCLUDE_DIR}
)

set(RADIANCE_INCLUDE_DIR
    ${RADIANCE_DIR}
    ${RADIANCE_DIR}/include
    ${OPENGL_INCLUDE_DIR}
)
###########################################################
# File Lists
###########################################################

file(GLOB_RECURSE COMMON_LIST
    ${SOURCE_DIR}/Common/*.cpp
    ${SOURCE_DIR}/Renderer/*.cpp
    ${SOURCE_DIR}/Shader/*.cpp
)
list(APPEND COMMON_LIST
    ${SOURCE_DIR}/Main/Config.cpp
    ${SOURCE_DIR}/Main/crc.cpp
    ${SOURCE_DIR}/Main/FrameBuffer.cpp
    ${SOURCE_DIR}/Main/Logo.cpp
    ${SOURCE_DIR}/Main/Register.cpp
    ${SOURCE_DIR}/Main/serialvalid.cpp
    ${SOURCE_DIR}/Main/SwiftConfig.cpp

    ${SOURCE_DIR}/Reactor/Nucleus.cpp
    ${SOURCE_DIR}/Reactor/Routine.cpp
    ${SOURCE_DIR}/Reactor/RoutineManager.cpp
)

file(GLOB_RECURSE EGL_LIST
    ${OPENGL_DIR}/libEGL/*.cpp
    ${OPENGL_DIR}/common/*.cpp
)

file(GLOB_RECURSE GLES2_LIST
    ${OPENGL_DIR}/libGLESv2/*.cpp
    ${OPENGL_DIR}/common/*.cpp
)

file(GLOB_RECURSE GLES_CM_LIST
    ${OPENGL_DIR}/libGLES_CM/*.cpp
    ${OPENGL_DIR}/common/*.cpp
)

file(GLOB_RECURSE RAD_LIST
    ${RADIANCE_DIR}/*.cpp
)

# Build the compiler list separately because we need to remove some os specific files
file(GLOB_RECURSE OPENGL_COMPILER_LIST
    ${OPENGL_COMPILER_DIR}/*.cpp
)
file(GLOB_RECURSE OPENGL_COMPILER_OS_SPECIFIC_LIST
    ${OPENGL_COMPILER_DIR}/ossource*.cpp
)
list(REMOVE_ITEM OPENGL_COMPILER_LIST ${OPENGL_COMPILER_OS_SPECIFIC_LIST})

###########################################################
# Append OS specific files to lists
###########################################################

if(LINUX)
    list(APPEND COMMON_LIST
        ${SOURCE_DIR}/Main/FrameBufferX11.cpp
    )
endif()

if(LINUX)
    list(APPEND OPENGL_COMPILER_LIST
        ${OPENGL_COMPILER_DIR}/ossource_posix.cpp
    )
endif()

###########################################################
# SwiftShaders Targets
###########################################################

# Takes target library and a directory where the export map is
# and add the linker options so that only the API symbols are
# exported.
macro(set_target_export_map TARGET DIR)
    get_target_property(LINKFLAGS ${TARGET} LINK_FLAGS)
    if(LINKFLAGS MATCHES "NOTFOUND")
        set(LINKFLAGS "")
    endif()
    if(LINUX OR APPLE)
        # Both hash-style are needed, TODO @capn knows why
        # The version script only exports the API functions and
        # hides all the others. Gc sections is used in combination
        # with each functions being in its section, to reduce the
        # binary size.
        set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "${LINKFLAGS} -Wl,--hash-style=both,--version-script=${DIR}/exports.map,--gc-sections")
    endif()
endmacro()

if(BUILD_EGL)
    add_library(EGL SHARED ${EGL_LIST})
    set_target_properties(EGL PROPERTIES INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}")
    set_target_export_map(EGL ${SOURCE_DIR}/OpenGL/libEGL)
    target_link_libraries(EGL dl X11 pthread)
endif()

if(BUILD_GLESv2)
    add_library(GLESv2 SHARED ${COMMON_LIST} ${GLES2_LIST} ${OPENGL_COMPILER_LIST})
    set_target_properties(GLESv2 PROPERTIES INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}")
    set_target_export_map(GLESv2 ${SOURCE_DIR}/OpenGL/libGLESv2)
    target_link_libraries(GLESv2 llvm X11 Xext pthread)
endif()

if(BUILD_GLES_CM)
    add_library(GLES_CM SHARED ${COMMON_LIST} ${GLES_CM_LIST})
    set_target_properties(GLES_CM PROPERTIES INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}")
    set_target_export_map(GLES_CM ${SOURCE_DIR}/OpenGL/libGLES_CM)
    target_link_libraries(GLES_CM llvm X11 Xext pthread)
endif()

if(BUILD_RAD)
    add_library(RAD SHARED ${COMMON_LIST} ${RAD_LIST} ${OPENGL_COMPILER_LIST})
    set_target_properties(RAD PROPERTIES INCLUDE_DIRECTORIES "${RADIANCE_INCLUDE_DIR}")
    set_target_export_map(RAD ${RADIANCE_DIR}/libRAD)
    target_link_libraries(RAD llvm X11 Xext pthread)
endif()

###########################################################
# Extra programs
###########################################################

add_executable(OGLES2HelloAPI tests/third_party/PowerVR/Examples/Beginner/01_HelloAPI/OGLES2/OGLES2HelloAPI_LinuxX11.cpp)
target_link_libraries(OGLES2HelloAPI dl X11 EGL GLESv2)
