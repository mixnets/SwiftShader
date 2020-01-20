cmake_minimum_required(VERSION 3.6.3)

set(CMAKE_CXX_STANDARD 14)

project(SwiftShader C CXX ASM)

# On Windows we use custom solution and project files, except for certain third-
# party projects which use CMake-generated ones. They are manually (re)generated
# and made path relative using build/cmake.sh, so they can be checked into the
# repository. Therefore they should not be auto-regenerated and left using
# absolute paths by CMake's ZERO_CHECK.
if(WIN32)
    # Disable automatically regenerating project files on CMakeLists.txt changes.
    set(CMAKE_SUPPRESS_REGENERATION true)
endif()

###########################################################
# Detect system
###########################################################

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(LINUX ON)
elseif(CMAKE_SYSTEM_NAME MATCHES "Android")
    set(ANDROID ON)
    set(CMAKE_CXX_FLAGS "-DANDROID_NDK_BUILD")
elseif(WIN32)
elseif(APPLE)
else()
    message(FATAL_ERROR "Platform is not supported")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "aarch64")
    else()
        set(ARCH "arm")
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "mips*")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "mips64el")
    else()
        set(ARCH "mipsel")
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc*")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "ppc64le")
    else()
        message(FATAL_ERROR "Architecture is not supported")
    endif()
else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "x86_64")
    else()
        set(ARCH "x86")
    endif()
endif()

set(CMAKE_MACOSX_RPATH ON)

if ((CMAKE_GENERATOR MATCHES "Visual Studio") AND (CMAKE_GENERATOR_TOOLSET STREQUAL ""))
  message(WARNING "Visual Studio generators use the x86 host compiler by "
                  "default, even for 64-bit targets. This can result in linker "
                  "instability and out of memory errors. To use the 64-bit "
                  "host compiler, pass -Thost=x64 on the CMake command line.")
endif()

# Use CCache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(STATUS "Using ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

###########################################################
# Host libraries
###########################################################

find_library(X11 X11)
find_library(XCB xcb)

###########################################################
# Options
###########################################################

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The type of build: Debug Release MinSizeRel RelWithDebInfo." FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)

function (option_if_not_defined name description default)
if(NOT DEFINED ${name})
    option(${name} ${description} ${default})
endif()
endfunction()

function (set_if_not_defined name value)
    if(NOT DEFINED ${name})
        set(${name} ${value} PARENT_SCOPE)
    endif()
endfunction()

option_if_not_defined(SWIFTSHADER_BUILD_EGL "Build the EGL library" 1)
option_if_not_defined(SWIFTSHADER_BUILD_GLESv2 "Build the OpenGL ES 2 library" 1)
option_if_not_defined(SWIFTSHADER_BUILD_GLES_CM "Build the OpenGL ES 1.1 library" 1)
option_if_not_defined(SWIFTSHADER_BUILD_VULKAN "Build the Vulkan library" 1)
option_if_not_defined(SWIFTSHADER_BUILD_PVR "Fetch and build the PowerVR examples" 0)

option_if_not_defined(SWIFTSHADER_USE_GROUP_SOURCES "Group the source files in a folder tree for Visual Studio" 1)

option_if_not_defined(SWIFTSHADER_BUILD_SAMPLES "Build sample programs" 1)
option_if_not_defined(SWIFTSHADER_BUILD_TESTS "Build test programs" 1)

option_if_not_defined(SWIFTSHADER_MSAN "Build with memory sanitizer" 0)
option_if_not_defined(SWIFTSHADER_ASAN "Build with address sanitizer" 0)
option_if_not_defined(SWIFTSHADER_TSAN "Build with thread sanitizer" 0)
option_if_not_defined(SWIFTSHADER_UBSAN "Build with undefined behavior sanitizer" 0)
option_if_not_defined(SWIFTSHADER_WARNINGS_AS_ERRORS "Treat all warnings as errors" 1)
option_if_not_defined(SWIFTSHADER_DCHECK_ALWAYS_ON "Check validation macros even in release builds" 0)
option_if_not_defined(REACTOR_EMIT_DEBUG_INFO "Emit debug info for JIT functions" 0)
option_if_not_defined(REACTOR_EMIT_PRINT_LOCATION "Emit printing of location info for JIT functions" 0)
option_if_not_defined(REACTOR_VERIFY_LLVM_IR "Check reactor-generated LLVM IR is valid even in release builds" 0)
option_if_not_defined(SWIFTSHADER_LESS_DEBUG_INFO "Generate less debug info to reduce file size" 0)
option_if_not_defined(SWIFTSHADER_ENABLE_VULKAN_DEBUGGER "Enable vulkan debugger support" 0)

set(BUILD_MARL ${SWIFTSHADER_BUILD_VULKAN})

if(${SWIFTSHADER_BUILD_VULKAN} AND ${SWIFTSHADER_ENABLE_VULKAN_DEBUGGER})
    set_if_not_defined(SWIFTSHADER_BUILD_CPPDAP 1)
else()
    set_if_not_defined(SWIFTSHADER_BUILD_CPPDAP 0)
endif()

set(DEFAULT_REACTOR_BACKEND "LLVM")

set(REACTOR_BACKEND ${DEFAULT_REACTOR_BACKEND} CACHE STRING "JIT compiler back-end used by Reactor")
set_property(CACHE REACTOR_BACKEND PROPERTY STRINGS LLVM Subzero)

# If defined, overrides the default optimization level of the current reactor backend.
# Set to one of the rr::Optimization::Level enum values.
set(REACTOR_DEFAULT_OPT_LEVEL "Default" CACHE STRING "Reactor default optimization level")
set_property(CACHE REACTOR_DEFAULT_OPT_LEVEL PROPERTY STRINGS "None" "Less" "Default" "Aggressive")

# LLVM disallows calling cmake . from the main LLVM dir, the reason is that
# it builds header files that could overwrite the orignal ones. Here we
# want to include LLVM as a subdirectory and even though it wouldn't cause
# the problem, if cmake . is called from the main dir, the condition that
# LLVM checkes, "CMAKE_CURRENT_SOURCE_DIR == CMAKE_CURRENT_BINARY_DIR" will be true. So we
# disallow it ourselves too to. In addition if there are remining CMakeFiles
# and CMakeCache in the directory, cmake .. from a subdirectory will still
# try to build from the main directory so we instruct users to delete these
# files when they get the error.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "In source builds are not allowed by LLVM, please create a build/ directory and build from there. You may have to delete the CMakeCache.txt file and CMakeFiles directory that are next to the CMakeLists.txt.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

###########################################################
# Initialize submodules
###########################################################

if (NOT TARGET gtest)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/.git)
        message(WARNING "
      third_party/googletest submodule missing.
      Running 'git submodule update --init' to download it:
        ")

        execute_process(COMMAND git submodule update --init ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest)
    endif()
endif()

if (NOT TARGET libbacktrace)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libbacktrace/src/.git)
        message(WARNING "
      third_party/libbacktrace/src submodule missing.
      Running 'git submodule update --init' to download it:
        ")

        execute_process(COMMAND git submodule update --init ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libbacktrace)
    endif()
endif()

# Boost is downloaded if necessary

# From https://www.boost.org/users/download/
set(BOOST_VER 1.70.0)
set(BOOST_HASH_TARGZ "882b48708d211a5f48e60b0124cf5863c1534cd544ecd0664bb534a4b5d506e9")
set(BOOST_HASH_ZIP   "48f379b2e90dd1084429aae87d6bdbde9670139fa7569ee856c8c86dd366039d")

string(REPLACE "." "_" BOOST_VER_FNAME ${BOOST_VER})
set(BOOST_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/boost_${BOOST_VER_FNAME})

function(DownloadBoost)
    if (LINUX)
        set(BOOST_EXT "tar.gz")
        set(BOOST_HASH ${BOOST_HASH_TARGZ})
    else()
        set(BOOST_EXT "zip")
        set(BOOST_HASH ${BOOST_HASH_ZIP})
    endif()

    # Note: bintray.com has rate limiting, so use the sourceforge mirror
    # set(BOOST_URL https://dl.bintray.com/boostorg/release/${BOOST_VER}/source/boost_${BOOST_VER_FNAME}.${BOOST_EXT})
    set(BOOST_URL https://iweb.dl.sourceforge.net/project/boost/boost/${BOOST_VER}/boost_${BOOST_VER_FNAME}.${BOOST_EXT})

    if (NOT TARGET Boost::boost)
        if(NOT EXISTS ${BOOST_THIRD_PARTY_DIR})
            message(WARNING "
          third_party/boost is missing.
          Downloading and extracting boost:
            ")

            set(BOOST_ARCHIVE ${CMAKE_BINARY_DIR}/temp/boost_archive)
            message(STATUS "Downloading ${BOOST_URL} to ${BOOST_ARCHIVE}...")
            file(DOWNLOAD ${BOOST_URL} ${BOOST_ARCHIVE} EXPECTED_HASH SHA256=${BOOST_HASH})
            message(STATUS "Extracting ${BOOST_ARCHIVE} to ${BOOST_THIRD_PARTY_DIR}...")
            execute_process(
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party
                COMMAND cmake -E tar xf ${BOOST_ARCHIVE}
            )
        endif()
    endif()
endfunction()

if (SWIFTSHADER_BUILD_PVR)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PowerVR_Examples/.git)
        message(WARNING "
        third_party/PowerVR_Examples submodule missing.
        Running 'git submodule update --init' to download it:
        ")

        execute_process(COMMAND git submodule update --init ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PowerVR_Examples)
    endif()
    set(PVR_BUILD_EXAMPLES ON CACHE BOOL "Build the PowerVR SDK Examples" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/PowerVR_Examples)
endif()

###########################################################
# Convenience macros
###########################################################

# Recursively calls source_group on the files of the directory
# so that Visual Studio has the files in a folder tree
macro(group_all_sources directory)
    file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${directory} ${CMAKE_CURRENT_SOURCE_DIR}/${directory}/*)
    foreach(file ${files})
        if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${directory}/${file})
            group_all_sources(${directory}/${file})
        else()
            string(REPLACE "/" "\\" groupname ${directory})
            source_group(${groupname} FILES ${CMAKE_CURRENT_SOURCE_DIR}/${directory}/${file})
        endif()
    endforeach()
endmacro()

# Takes target library and a directory where the export map is
# and add the linker options so that only the API symbols are
# exported.
macro(set_shared_library_export_map TARGET DIR)
    if(MSVC)
        set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " /DEF:\"${DIR}/${TARGET}.def\"")
    elseif(APPLE)
        # The exported symbols list only exports the API functions and
        # hides all the others.
        set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS "-exported_symbols_list ${DIR}/${TARGET}.exports")
        set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_DEPENDS "${DIR}/${TARGET}.exports;")
        # Don't allow undefined symbols, unless it's a Sanitizer build.
        if(NOT SWIFTSHADER_MSAN AND NOT SWIFTSHADER_ASAN AND NOT SWIFTSHADER_TSAN AND NOT SWIFTSHADER_UBSAN)
            set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-undefined,error")
        endif()
    elseif(LINUX)
        # The version script only exports the API functions and
        # hides all the others.
        set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--version-script=${DIR}/${TARGET}.lds")
        set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_DEPENDS "${DIR}/${TARGET}.lds;")

        # Don't allow symbols to be overridden by another module.
        set_property(TARGET ${TARGET} APPEND_STRING PROPERTY COMPILE_FLAGS " -fvisibility=protected")

        if(ARCH STREQUAL "mipsel" OR ARCH STREQUAL "mips64el")
          # MIPS supports sysv hash-style only.
          set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--hash-style=sysv")
        else()
          # Both hash-style are needed, because we want both gold and
          # GNU ld to be able to read our libraries.
          set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--hash-style=both")
        endif()

        # Gc sections is used in combination with each functions being
        # in its own section, to reduce the binary size.
        set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--gc-sections")

        # Don't allow undefined symbols, unless it's a Sanitizer build.
        if(NOT SWIFTSHADER_MSAN AND NOT SWIFTSHADER_ASAN AND NOT SWIFTSHADER_TSAN AND NOT SWIFTSHADER_UBSAN)
            set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--no-undefined")
        endif()
    endif()
endmacro()

if(SWIFTSHADER_USE_GROUP_SOURCES)
    group_all_sources(src)
endif()

###########################################################
# Directories
###########################################################

set(SWIFTSHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SWIFTSHADER_OPENGL_DIR ${SWIFTSHADER_SOURCE_DIR}/OpenGL)
set(SWIFTSHADER_OPENGL_COMPILER_DIR ${SWIFTSHADER_OPENGL_DIR}/compiler)
set(SWIFTSHADER_VULKAN_DIR ${SWIFTSHADER_SOURCE_DIR}/Vulkan)
set(SWIFTSHADER_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(SWIFTSHADER_LLVM_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/llvm-7.0/llvm)
set(SWIFTSHADER_LLVM_CONFIG_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/llvm-7.0/configs)
set(SWIFTSHADER_LIBBACKTRACE_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/libbacktrace/src)
set(SWIFTSHADER_LIBBACKTRACE_CONFIG_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/libbacktrace/config)
set(SWIFTSHADER_LIBBACKTRACE_INCLUDE_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/libbacktrace/src)
set(SWIFTSHADER_MARL_INCLUDE_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/marl/include)
set(SWIFTSHADER_CPPDAP_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/cppdap)
set(SWIFTSHADER_CPPDAP_INCLUDE_DIR ${SWIFTSHADER_CPPDAP_DIR}/include)
set(SWIFTSHADER_JSON_INCLUDE_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/json/include)
set(SWIFTSHADER_SUBZERO_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/subzero)
set(SWIFTSHADER_SUBZERO_LLVM_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/llvm-subzero)
set(SWIFTSHADER_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(SWIFTSHADER_HELLO2_DIR ${SWIFTSHADER_THIRD_PARTY_DIR}/PowerVR_SDK/Examples/Beginner/01_HelloAPI/OGLES2)

###########################################################
# Compile flags
###########################################################

# Flags for project code (non 3rd party)
set(SWIFTSHADER_COMPILE_OPTIONS "")

macro(set_cpp_flag FLAG)
    if(${ARGC} GREATER 1)
        set(CMAKE_CXX_FLAGS_${ARGV1} "${CMAKE_CXX_FLAGS_${ARGV1}} ${FLAG}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
    endif()
endmacro()

macro(set_linker_flag FLAG)
    if(${ARGC} GREATER 1)
        set(CMAKE_EXE_LINKER_FLAGS ${ARGV1} "${CMAKE_EXE_LINKER_FLAGS ${ARGV1}} ${FLAG}")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAG}")
    endif()
endmacro()

if(MSVC)
    set_cpp_flag("/MP")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_SBCS)  # Single Byte Character Set (ASCII)
    add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)  # Disable MSVC warnings about std::aligned_storage being broken before VS 2017 15.8

    set_cpp_flag("/DEBUG:FASTLINK" DEBUG)
    set_cpp_flag("/DEBUG:FASTLINK" RELWITHDEBINFO)

    if(SWIFTSHADER_WARNINGS_AS_ERRORS)
        set_cpp_flag("/WX") # Treat all warnings as errors
    endif()

    # Disable specific warnings
    # TODO: Not all of these should be disabled, but for now, we want a warning-free msvc build. Remove these one by one
    #       and fix the actual warnings in code.
    list(APPEND SWIFTSHADER_COMPILE_OPTIONS
        "/wd4005" # 'identifier' : macro redefinition
        "/wd4018" # 'expression' : signed/unsigned mismatch
        "/wd4065" # switch statement contains 'default' but no 'case' labels
        "/wd4141" # 'modifier' : used more than once
        "/wd4244" # 'conversion' conversion from 'type1' to 'type2', possible loss of data
        "/wd4267" # 'var' : conversion from 'size_t' to 'type', possible loss of data
        "/wd4291" # 'void X new(size_t,unsigned int,unsigned int)': no matching operator delete found; memory will not be freed if initialization throws an exception
        "/wd4309" # 'conversion' : truncation of constant value
        "/wd4624" # 'derived class' : destructor was implicitly defined as deleted because a base class destructor is inaccessible or deleted
        "/wd4800" # 'type' : forcing value to bool 'true' or 'false' (performance warning)
        "/wd4838" # conversion from 'type_1' to 'type_2' requires a narrowing conversion
        "/wd5030" # attribute 'attribute' is not recognized
        "/wd5038" # data member 'member1' will be initialized after data member 'member2' data member 'member' will be initialized after base class 'base_class'
    )

    # Treat specific warnings as errors
    list(APPEND SWIFTSHADER_COMPILE_OPTIONS
        "/we4018" # 'expression' : signed/unsigned mismatch
        "/we4471" # 'enumeration': a forward declaration of an unscoped enumeration must have an underlying type (int assumed)
        "/we4838" # conversion from 'type_1' to 'type_2' requires a narrowing conversion
        "/we5038" # data member 'member1' will be initialized after data member 'member2' data member 'member' will be initialized after base class 'base_class'
    )
else()
    set_cpp_flag("-fno-exceptions")

    list(APPEND SWIFTSHADER_COMPILE_OPTIONS
        "-Wall"
        "-Wreorder"
        "-Wsign-compare"
        "-Wmissing-braces"
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND SWIFTSHADER_COMPILE_OPTIONS
            "-Wunused-lambda-capture"
            "-Wstring-conversion"
            "-Wextra-semi"
            "-Wignored-qualifiers"
            # Silence errors caused by unknown warnings when building with older
            # versions of Clang. This demands checking that warnings added above
            # are spelled correctly and work as intended!
            "-Wno-unknown-warning-option"
        )
    endif()

    # Remove xor, and, or and friends from the list of keywords, they are used
    # by Reactor
    list(APPEND SWIFTSHADER_COMPILE_OPTIONS
        "-fno-operator-names"
    )

    # LLVM headers requires these flags to be set
    set_cpp_flag("-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS")

    if(ARCH STREQUAL "x86")
        set_cpp_flag("-m32")
        set_cpp_flag("-msse2")
        set_cpp_flag("-mfpmath=sse")
        set_cpp_flag("-march=pentium4")
        set_cpp_flag("-mtune=generic")
    endif()
    if(ARCH STREQUAL "x86_64")
        set_cpp_flag("-m64")
        set_cpp_flag("-fPIC")
        set_cpp_flag("-march=x86-64")
        set_cpp_flag("-mtune=generic")
    endif()
    if(ARCH STREQUAL "mipsel")
        set_cpp_flag("-march=mips32r2")
        set_cpp_flag("-fPIC")
        set_cpp_flag("-mhard-float")
        set_cpp_flag("-mfp32")
    endif()
    if(ARCH STREQUAL "mips64el")
        set_cpp_flag("-march=mips64r2")
        set_cpp_flag("-mabi=64")
        set_cpp_flag("-fPIC")
    endif()

    if(SWIFTSHADER_LESS_DEBUG_INFO)
        # Use -g1 to be able to get stack traces
        set_cpp_flag("-g -g1" DEBUG)
        set_cpp_flag("-g -g1" RELWITHDEBINFO)
    else()
        # Use -g3 to have even more debug info
        set_cpp_flag("-g -g3" DEBUG)
        set_cpp_flag("-g -g3" RELWITHDEBINFO)
    endif()

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Treated as an unused argument with clang
        set_cpp_flag("-s" RELEASE)
    endif()

    if(SWIFTSHADER_WARNINGS_AS_ERRORS)
        set_cpp_flag("-Werror") # Treat all warnings as errors
    endif()

    if(SWIFTSHADER_DCHECK_ALWAYS_ON)
        list(APPEND SWIFTSHADER_COMPILE_OPTIONS "-DDCHECK_ALWAYS_ON")
    endif()

    # Disable pedanitc warnings
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND SWIFTSHADER_COMPILE_OPTIONS
            "-Wno-ignored-attributes"   # ignoring attributes on template argument 'X'
            "-Wno-attributes"           # 'X' attribute ignored
            "-Wno-strict-aliasing"      # dereferencing type-punned pointer will break strict-aliasing rules
            "-Wno-comment"              # multi-line comment
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND SWIFTSHADER_COMPILE_OPTIONS
            "-Wno-unneeded-internal-declaration"  # function 'X' is not needed and will not be emitted
            "-Wno-unused-private-field"           # private field 'offset' is not used - TODO: Consider enabling this once Vulkan is further implemented.
            "-Wno-comment"                        # multi-line comment
            "-Wno-undefined-var-template"         # instantiation of variable 'X' required here, but no definition is available
        )
    endif()

    # For distribution it is more important to be slim than super optimized
    set_cpp_flag("-Os" RELEASE)
    set_cpp_flag("-Os" RELWITHDEBINFO)

    set_cpp_flag("-DNDEBUG" RELEASE)
    set_cpp_flag("-DNDEBUG" RELWITHDEBINFO)
    set_cpp_flag("-DANGLE_DISABLE_TRACE" RELEASE)
    set_cpp_flag("-DANGLE_DISABLE_TRACE" RELWITHDEBINFO)

    # Put each variable and function in its own section so that when linking
    # with -gc-sections unused functions and variables are removed.
    set_cpp_flag("-ffunction-sections" RELEASE)
    set_cpp_flag("-fdata-sections" RELEASE)
    set_cpp_flag("-fomit-frame-pointer" RELEASE)

    if(SWIFTSHADER_MSAN)
        set_cpp_flag("-fsanitize=memory")
        set_linker_flag("-fsanitize=memory")
    elseif(SWIFTSHADER_ASAN)
        set_cpp_flag("-fsanitize=address")
        set_linker_flag("-fsanitize=address")
    elseif(SWIFTSHADER_TSAN)
        set_cpp_flag("-fsanitize=thread")
        set_linker_flag("-fsanitize=thread")
    elseif(SWIFTSHADER_UBSAN)
        set_cpp_flag("-fsanitize=undefined")
        set_linker_flag("-fsanitize=undefined")
    endif()
endif()

if(REACTOR_EMIT_PRINT_LOCATION)
    # This feature depends on REACTOR_EMIT_DEBUG_INFO, so enable it
    set(REACTOR_EMIT_DEBUG_INFO "On")
    list(APPEND SWIFTSHADER_COMPILE_OPTIONS "-DENABLE_RR_EMIT_PRINT_LOCATION")
endif()

if(REACTOR_EMIT_DEBUG_INFO)
    message(WARNING "REACTOR_EMIT_DEBUG_INFO is enabled. This will likely affect performance.")
    list(APPEND SWIFTSHADER_COMPILE_OPTIONS "-DENABLE_RR_DEBUG_INFO")
endif()

if(REACTOR_VERIFY_LLVM_IR)
    list(APPEND SWIFTSHADER_COMPILE_OPTIONS "-DENABLE_RR_LLVM_IR_VERIFICATION")
endif()

if(REACTOR_DEFAULT_OPT_LEVEL)
    list(APPEND SWIFTSHADER_COMPILE_OPTIONS "-DREACTOR_DEFAULT_OPT_LEVEL=${REACTOR_DEFAULT_OPT_LEVEL}")
endif()

if(WIN32)
    add_definitions(-DWINVER=0x501 -DNOMINMAX -DSTRICT)
    set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} "" "lib")
endif()

###########################################################
# LLVM
###########################################################

set(LLVM_LIST
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/AliasAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/AliasAnalysisSummary.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/AliasSetTracker.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/AssumptionCache.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/BasicAliasAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/BlockFrequencyInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/BlockFrequencyInfoImpl.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/BranchProbabilityInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/CallGraph.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/CallGraphSCCPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/CaptureTracking.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/CFG.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/CFLAndersAliasAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/CFLSteensAliasAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/CmpInstAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/CodeMetrics.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ConstantFolding.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/DemandedBits.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/DependenceAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/DivergenceAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/DominanceFrontier.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/EHPersonalities.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/GlobalsModRef.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/InlineCost.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/InstructionSimplify.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/IteratedDominanceFrontier.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/IVUsers.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/LazyBlockFrequencyInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/LazyBranchProbabilityInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/LazyValueInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/Loads.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/LoopAnalysisManager.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/LoopInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/LoopPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/MemoryBuiltins.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/MemoryDependenceAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/MemoryLocation.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/MemorySSA.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/MemorySSAUpdater.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/MustExecute.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ObjCARCAliasAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ObjCARCAnalysisUtils.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ObjCARCInstKind.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/OptimizationRemarkEmitter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/OrderedBasicBlock.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/PHITransAddr.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/PhiValues.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/PostDominators.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ProfileSummaryInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/PtrUseVisitor.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ScalarEvolution.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ScalarEvolutionAliasAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ScalarEvolutionExpander.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ScalarEvolutionNormalization.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ScopedNoAliasAA.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/TargetLibraryInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/TargetTransformInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/TypeBasedAliasAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ValueLattice.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ValueLatticeUtils.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/ValueTracking.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Analysis/VectorUtils.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/AsmParser/LLLexer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/AsmParser/LLParser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/AsmParser/Parser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/BinaryFormat/Dwarf.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/BinaryFormat/Magic.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/BinaryFormat/Wasm.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Bitcode/Reader/BitcodeReader.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Bitcode/Reader/BitstreamReader.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Bitcode/Reader/MetadataLoader.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Bitcode/Reader/ValueList.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AggressiveAntiDepBreaker.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AllocationOrder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/Analysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/AccelTable.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/AddressPool.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/ARMException.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/AsmPrinter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/AsmPrinterDwarf.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/AsmPrinterInlineAsm.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/CodeViewDebug.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DbgValueHistoryCalculator.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DebugHandlerBase.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DebugLocStream.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DIE.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DIEHash.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DwarfCFIException.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DwarfCompileUnit.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DwarfDebug.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DwarfExpression.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DwarfFile.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DwarfStringPool.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/DwarfUnit.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/EHStreamer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/WinCFGuard.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AsmPrinter/WinException.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/AtomicExpandPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/BasicTargetTransformInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/BranchFolding.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/BranchRelaxation.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/BreakFalseDeps.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/CalcSpillWeights.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/CallingConvLower.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/CFIInstrInserter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/CodeGen.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/CodeGenPrepare.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/CriticalAntiDepBreaker.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/DeadMachineInstructionElim.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/DetectDeadLanes.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/DFAPacketizer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/DwarfEHPrepare.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/EarlyIfConversion.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/EdgeBundles.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ExecutionDomainFix.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ExpandISelPseudos.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ExpandMemCmp.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ExpandPostRAPseudos.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ExpandReductions.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/FaultMaps.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/FEntryInserter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/FuncletLayout.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GCMetadata.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GCMetadataPrinter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GCRootLowering.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GCStrategy.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/CallLowering.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/GlobalISel.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/InstructionSelect.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/InstructionSelector.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/IRTranslator.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/LegalityPredicates.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/LegalizeMutations.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/Legalizer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/LegalizerHelper.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/LegalizerInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/Localizer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/MachineIRBuilder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/RegBankSelect.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/RegisterBank.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/RegisterBankInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalISel/Utils.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/IfConversion.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ImplicitNullChecks.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/IndirectBrExpandPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/InlineSpiller.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/InterferenceCache.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/InterleavedAccessPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/IntrinsicLowering.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LatencyPriorityQueue.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LazyMachineBlockFrequencyInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LexicalScopes.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveDebugValues.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveDebugVariables.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveInterval.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveIntervals.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveIntervalUnion.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LivePhysRegs.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveRangeCalc.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveRangeEdit.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveRangeShrink.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveRegMatrix.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveRegUnits.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveStacks.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LiveVariables.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LLVMTargetMachine.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LocalStackSlotAllocation.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LoopTraversal.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LowerEmuTLS.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/LowLevelType.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineBasicBlock.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineBlockFrequencyInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineBlockPlacement.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineBranchProbabilityInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineCombiner.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineCopyPropagation.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineCSE.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineDominanceFrontier.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineDominators.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineFrameInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineFunction.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineFunctionPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineFunctionPrinterPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineInstr.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineInstrBundle.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineLICM.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineLoopInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineModuleInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineModuleInfoImpls.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineOperand.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineOptimizationRemarkEmitter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineOutliner.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachinePassRegistry.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachinePipeliner.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachinePostDominators.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineRegionInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineRegisterInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineScheduler.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineSink.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineSSAUpdater.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineTraceMetrics.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MachineVerifier.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MacroFusion.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MIRCanonicalizerPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MIRPrinter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/MIRPrintingPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/OptimizePHIs.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/PatchableFunction.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/PeepholeOptimizer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/PHIElimination.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/PHIEliminationUtils.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/PostRAHazardRecognizer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/PostRASchedulerList.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/PreISelIntrinsicLowering.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ProcessImplicitDefs.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/PrologEpilogInserter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/PseudoSourceValue.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ReachingDefAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegAllocBase.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegAllocBasic.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegAllocFast.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegAllocGreedy.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegisterClassInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegisterCoalescer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegisterPressure.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegisterScavenging.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegisterUsageInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegUsageInfoCollector.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegUsageInfoPropagate.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RenameIndependentSubregs.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ResetMachineFunctionPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SafeStack.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SafeStackColoring.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SafeStackLayout.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ScalarizeMaskedMemIntrin.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ScheduleDAG.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ScheduleDAGInstrs.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ScheduleDAGPrinter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ScoreboardHazardRecognizer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/DAGCombiner.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/FastISel.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/FunctionLoweringInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/InstrEmitter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/LegalizeDAG.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/LegalizeFloatTypes.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/LegalizeIntegerTypes.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/LegalizeTypes.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/LegalizeTypesGeneric.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/LegalizeVectorOps.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/LegalizeVectorTypes.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/ResourcePriorityQueue.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/ScheduleDAGRRList.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/ScheduleDAGSDNodes.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/ScheduleDAGVLIW.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/SelectionDAG.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/SelectionDAGAddressAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/SelectionDAGBuilder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/SelectionDAGDumper.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/SelectionDAGISel.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/SelectionDAGPrinter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/SelectionDAGTargetInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/StatepointLowering.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SelectionDAG/TargetLowering.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ShadowStackGCLowering.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ShrinkWrap.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SjLjEHPrepare.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SlotIndexes.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SpillPlacement.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/SplitKit.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/StackColoring.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/StackMapLivenessAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/StackMaps.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/StackProtector.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/StackSlotColoring.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TailDuplication.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TailDuplicator.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TargetFrameLoweringImpl.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TargetInstrInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TargetLoweringBase.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TargetLoweringObjectFileImpl.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TargetOptionsImpl.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TargetPassConfig.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TargetRegisterInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TargetSchedule.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TargetSubtargetInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/TwoAddressInstructionPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/UnreachableBlockElim.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/ValueTypes.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/VirtRegMap.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/WasmEHPrepare.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/WinEHPrepare.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/XRayInstrumentation.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/CodeViewError.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/CodeViewRecordIO.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/ContinuationRecordBuilder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/CVTypeVisitor.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/Formatters.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/GlobalTypeTableBuilder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/Line.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/RecordName.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/RecordSerialization.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/SimpleTypeSerializer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/SymbolRecordMapping.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/TypeDumpVisitor.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/TypeHashing.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/TypeIndex.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/TypeIndexDiscovery.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/TypeRecordMapping.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/DebugInfo/CodeView/TypeTableCollection.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/ExecutionEngine.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/GDBRegistrationListener.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/Orc/Core.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/Orc/Legacy.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/Orc/OrcError.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/RuntimeDyld/JITSymbol.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/RuntimeDyld/RTDyldMemoryManager.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/RuntimeDyld/RuntimeDyld.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/RuntimeDyld/RuntimeDyldChecker.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/RuntimeDyld/RuntimeDyldCOFF.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/RuntimeDyld/RuntimeDyldELF.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/RuntimeDyld/RuntimeDyldMachO.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/RuntimeDyld/Targets/RuntimeDyldELFMips.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/SectionMemoryManager.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ExecutionEngine/TargetSelect.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/AsmWriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Attributes.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/AutoUpgrade.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/BasicBlock.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Comdat.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/ConstantFold.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/ConstantRange.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Constants.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/DataLayout.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/DebugInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/DebugInfoMetadata.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/DebugLoc.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/DiagnosticHandler.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/DiagnosticInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/DiagnosticPrinter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/DIBuilder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Dominators.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Function.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Globals.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/GVMaterializer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/InlineAsm.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Instruction.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Instructions.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/IntrinsicInst.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/IRBuilder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/IRPrintingPasses.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/LegacyPassManager.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/LLVMContext.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/LLVMContextImpl.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Mangler.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/MDBuilder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Metadata.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Module.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Operator.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/OptBisect.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Pass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/PassManager.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/PassRegistry.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/ProfileSummary.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Statepoint.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Type.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/TypeFinder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Use.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/User.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Value.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/ValueSymbolTable.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/IR/Verifier.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/ELFObjectWriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MachObjectWriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCAsmBackend.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCAsmInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCAsmInfoCOFF.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCAsmInfoDarwin.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCAsmInfoELF.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCAsmMacro.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCAsmStreamer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCAssembler.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCCodeEmitter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCCodePadder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCCodeView.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCContext.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCDisassembler/MCRelocationInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCDwarf.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCELFObjectTargetWriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCELFStreamer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCExpr.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCFragment.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCInst.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCInstPrinter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCInstrAnalysis.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCInstrDesc.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCLinkerOptimizationHint.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCMachObjectTargetWriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCMachOStreamer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCNullStreamer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCObjectFileInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCObjectStreamer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCObjectWriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCParser/AsmLexer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCParser/AsmParser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCParser/COFFAsmParser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCParser/DarwinAsmParser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCParser/ELFAsmParser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCParser/MCAsmLexer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCParser/MCAsmParser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCParser/MCAsmParserExtension.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCParser/MCTargetAsmParser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCRegisterInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCSchedule.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCSection.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCSectionCOFF.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCSectionELF.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCSectionMachO.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCSectionWasm.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCStreamer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCSubtargetInfo.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCSymbol.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCSymbolELF.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCTargetOptions.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCValue.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCWasmStreamer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCWin64EH.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCWinCOFFStreamer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/MCWinEH.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/StringTableBuilder.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/SubtargetFeature.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/WasmObjectWriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/MC/WinCOFFObjectWriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/Archive.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/Binary.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/COFFObjectFile.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/ELF.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/ELFObjectFile.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/Error.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/IRObjectFile.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/IRSymtab.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/MachOObjectFile.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/MachOUniversal.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/ModuleSymbolTable.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/ObjectFile.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/RecordStreamer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/SymbolicFile.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/WasmObjectFile.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Object/WindowsResource.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/ProfileData/InstrProf.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/APFloat.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/APInt.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/APSInt.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/ARMAttributeParser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/ARMBuildAttrs.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Atomic.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/BinaryStreamError.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/BinaryStreamReader.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/BinaryStreamRef.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/BinaryStreamWriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/BlockFrequency.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/BranchProbability.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Chrono.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/circular_raw_ostream.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/CodeGenCoverage.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/CommandLine.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Compression.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/ConvertUTF.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/ConvertUTFWrapper.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/DataExtractor.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Debug.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/DebugCounter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/DJB.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/DynamicLibrary.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Errno.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Error.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/ErrorHandling.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/FoldingSet.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/FormattedStream.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/FormatVariadic.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/GraphWriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Hashing.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Host.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/IntEqClasses.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/IntervalMap.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/JamCRC.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/KnownBits.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/LEB128.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/LineIterator.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Locale.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/LowLevelType.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/ManagedStatic.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/MathExtras.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/MD5.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Memory.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/MemoryBuffer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Mutex.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/NativeFormatting.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Path.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/PrettyStackTrace.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Process.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Program.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/RandomNumberGenerator.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/raw_ostream.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/regcomp.c
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/regerror.c
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Regex.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/regexec.c
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/regfree.c
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/regstrlcpy.c
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/RWMutex.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/ScaledNumber.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/ScopedPrinter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/SHA1.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Signals.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/SmallPtrSet.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/SmallVector.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/SourceMgr.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Statistic.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/StringExtras.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/StringMap.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/StringRef.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/StringSaver.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/TargetParser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/TargetRegistry.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Threading.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Timer.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/ToolOutputFile.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Triple.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Twine.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Unicode.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/UnicodeCaseFold.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/Valgrind.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/YAMLParser.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Support/YAMLTraits.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Target/TargetLoweringObjectFile.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Target/TargetMachine.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Coroutines/CoroCleanup.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Coroutines/CoroEarly.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Coroutines/CoroElide.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Coroutines/CoroFrame.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Coroutines/CoroSplit.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Coroutines/Coroutines.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineAddSub.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineAndOrXor.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineCalls.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineCasts.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineCompares.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineLoadStoreAlloca.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineMulDivRem.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombinePHI.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineSelect.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineShifts.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineSimplifyDemanded.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstCombineVectorOps.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/InstCombine/InstructionCombining.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/IPO/BarrierNoopPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/ADCE.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/ConstantHoisting.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/DeadStoreElimination.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/EarlyCSE.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/GVN.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/LICM.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/LoopDataPrefetch.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/LoopStrengthReduce.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/MergeICmps.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/PartiallyInlineLibCalls.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/Reassociate.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/SCCP.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/SeparateConstOffsetFromGEP.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/SimplifyCFGPass.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/SROA.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/BasicBlockUtils.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/BreakCriticalEdges.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/BuildLibCalls.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/BypassSlowDivision.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/CloneFunction.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/DemoteRegToStack.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/EntryExitInstrumenter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/EscapeEnumerator.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/InlineFunction.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/LCSSA.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/Local.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/LoopSimplify.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/LoopUtils.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/LowerInvoke.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/OrderedInstructions.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/PromoteMemoryToRegister.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/SimplifyCFG.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/SimplifyLibCalls.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/SSAUpdater.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/SymbolRewriter.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/ValueMapper.cpp
    ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Utils/VNCoercion.cpp
    ${SWIFTSHADER_LLVM_DIR}/../stubs/Stubs.cpp
)

if(ARCH STREQUAL "x86" OR ARCH STREQUAL "x86_64")
    list(APPEND LLVM_LIST
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/AsmParser/X86AsmInstrumentation.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/AsmParser/X86AsmParser.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/InstPrinter/X86ATTInstPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/InstPrinter/X86InstComments.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/InstPrinter/X86InstPrinterCommon.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/InstPrinter/X86IntelInstPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/MCTargetDesc/X86AsmBackend.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/MCTargetDesc/X86ELFObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/MCTargetDesc/X86MachObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/MCTargetDesc/X86MCAsmInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/MCTargetDesc/X86MCCodeEmitter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/MCTargetDesc/X86MCTargetDesc.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/MCTargetDesc/X86WinCOFFObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/MCTargetDesc/X86WinCOFFStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/MCTargetDesc/X86WinCOFFTargetStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/ShadowCallStack.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/TargetInfo/X86TargetInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/Utils/X86ShuffleDecode.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86AsmPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86AvoidStoreForwardingBlocks.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86CallFrameOptimization.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86CallingConv.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86CallLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86CmovConversion.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86DomainReassignment.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86EvexToVex.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86ExpandPseudo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86FastISel.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86FixupBWInsts.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86FixupLEAs.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86FixupSetCC.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86FlagsCopyLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86FloatingPoint.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86FrameLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86IndirectBranchTracking.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86InstrFMA3Info.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86InstrFoldTables.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86InstrInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86InstructionSelector.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86InterleavedAccess.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86ISelDAGToDAG.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86ISelLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86LegalizerInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86MachineFunctionInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86MacroFusion.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86MCInstLower.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86OptimizeLEAs.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86PadShortFunction.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86RegisterBankInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86RegisterInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86RetpolineThunks.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86SelectionDAGInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86ShuffleDecodeConstantPool.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86SpeculativeLoadHardening.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86Subtarget.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86TargetMachine.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86TargetObjectFile.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86TargetTransformInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86VZeroUpper.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86WinAllocaExpander.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86/X86WinEHState.cpp
    )
elseif(ARCH STREQUAL "mipsel" OR ARCH STREQUAL "mips64el")
    list(APPEND LLVM_LIST
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/AsmParser/MipsAsmParser.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/InstPrinter/MipsInstPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsABIFlagsSection.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsABIInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsAsmBackend.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsELFObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsELFStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsMCAsmInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsMCCodeEmitter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsMCExpr.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsMCTargetDesc.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsNaClELFStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsOptionRecord.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MCTargetDesc/MipsTargetStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MicroMipsSizeReduction.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/Mips16FrameLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/Mips16HardFloat.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/Mips16HardFloatInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/Mips16InstrInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/Mips16ISelDAGToDAG.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/Mips16ISelLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/Mips16RegisterInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsAnalyzeImmediate.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsAsmPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsBranchExpansion.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsCallLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsCCState.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsConstantIslandPass.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsDelaySlotFiller.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsExpandPseudo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsFastISel.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsFrameLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsInstrInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsInstructionSelector.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsISelDAGToDAG.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsISelLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsLegalizerInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsMachineFunction.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsMCInstLower.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsModuleISelDAGToDAG.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsOptimizePICCall.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsOs16.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsRegisterBankInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsRegisterInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsSEFrameLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsSEInstrInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsSEISelDAGToDAG.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsSEISelLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsSERegisterInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsSubtarget.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsTargetMachine.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/MipsTargetObjectFile.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips/TargetInfo/MipsTargetInfo.cpp
    )
elseif(ARCH STREQUAL "aarch64")
    list(APPEND LLVM_LIST
        ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/GlobalMerge.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/CodeGen/RegAllocPBQP.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/MC/ConstantPools.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64A53Fix835769.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64A57FPLoadBalancing.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64AdvSIMDScalarPass.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64AsmPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64CallLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64CleanupLocalDynamicTLSPass.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64CollectLOH.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64CondBrTuning.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64ConditionalCompares.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64ConditionOptimizer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64DeadRegisterDefinitionsPass.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64ExpandPseudoInsts.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64FalkorHWPFFix.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64FastISel.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64FrameLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64InstrInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64InstructionSelector.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64ISelDAGToDAG.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64ISelLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64LegalizerInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64LoadStoreOptimizer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64MacroFusion.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64MCInstLower.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64PBQPRegAlloc.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64PromoteConstant.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64RedundantCopyElimination.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64RegisterBankInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64RegisterInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64SelectionDAGInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64SIMDInstrOpt.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64StorePairSuppress.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64Subtarget.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64TargetMachine.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64TargetObjectFile.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AArch64TargetTransformInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/AsmParser/AArch64AsmParser.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/Disassembler/AArch64Disassembler.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/Disassembler/AArch64ExternalSymbolizer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/InstPrinter/AArch64InstPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64AsmBackend.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64ELFObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64ELFStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64MachObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64MCAsmInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64MCCodeEmitter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64MCExpr.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64MCTargetDesc.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64TargetStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64WinCOFFObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/MCTargetDesc/AArch64WinCOFFStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/TargetInfo/AArch64TargetInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64/Utils/AArch64BaseInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/LoopDataPrefetch.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Transforms/Scalar/SeparateConstOffsetFromGEP.cpp
    )
elseif(ARCH STREQUAL "arm")
    list(APPEND LLVM_LIST
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/A15SDOptimizer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMAsmPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMBaseInstrInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMBaseRegisterInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMCallLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMCodeGenPrepare.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMComputeBlockSize.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMConstantIslandPass.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMConstantPoolValue.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMExpandPseudoInsts.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMFastISel.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMFrameLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMHazardRecognizer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMInstrInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMInstructionSelector.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMISelDAGToDAG.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMISelLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMLegalizerInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMLoadStoreOptimizer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMMachineFunctionInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMMacroFusion.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMMCInstLower.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMOptimizeBarriersPass.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMParallelDSP.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMRegisterBankInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMRegisterInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMSelectionDAGInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMSubtarget.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMTargetMachine.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMTargetObjectFile.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ARMTargetTransformInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/AsmParser/ARMAsmParser.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/Disassembler/ARMDisassembler.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/InstPrinter/ARMInstPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMAsmBackend.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMELFObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMELFStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMMachObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMMachORelocationInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMMCAsmInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMMCCodeEmitter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMMCExpr.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMMCTargetDesc.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMTargetStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMUnwindOpAsm.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMWinCOFFObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MCTargetDesc/ARMWinCOFFStreamer.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/MLxExpansionPass.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/TargetInfo/ARMTargetInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/Thumb1FrameLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/Thumb1InstrInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/Thumb2InstrInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/Thumb2ITBlockPass.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/Thumb2SizeReduction.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/ThumbRegisterInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM/Utils/ARMBaseInfo.cpp
    )
elseif(ARCH STREQUAL "ppc64le")
    list(APPEND LLVM_LIST
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/AsmParser/PPCAsmParser.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/Disassembler/PPCDisassembler.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/InstPrinter/PPCInstPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/MCTargetDesc/PPCAsmBackend.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/MCTargetDesc/PPCELFObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/MCTargetDesc/PPCMachObjectWriter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/MCTargetDesc/PPCMCAsmInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/MCTargetDesc/PPCMCCodeEmitter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/MCTargetDesc/PPCMCExpr.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/MCTargetDesc/PPCMCTargetDesc.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/MCTargetDesc/PPCPredicates.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCAsmPrinter.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCBoolRetToInt.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCBranchCoalescing.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCBranchSelector.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCCCState.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCCTRLoops.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCEarlyReturn.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCExpandISEL.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCFastISel.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCFrameLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCHazardRecognizers.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCInstrInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCISelDAGToDAG.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCISelLowering.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCLoopPreIncPrep.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCMachineFunctionInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCMCInstLower.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCMIPeephole.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCPreEmitPeephole.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCQPXLoadSplat.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCReduceCRLogicals.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCRegisterInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCSubtarget.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCTargetMachine.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCTargetObjectFile.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCTargetTransformInfo.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCTLSDynamicCall.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCTOCRegDeps.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCVSXCopy.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCVSXFMAMutate.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/PPCVSXSwapRemoval.cpp
        ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC/TargetInfo/PowerPCTargetInfo.cpp
    )
endif()

if(REACTOR_EMIT_DEBUG_INFO)
    list(APPEND LLVM_LIST
        ${SWIFTSHADER_LLVM_DIR}/lib/Demangle/ItaniumDemangle.cpp
    )
endif(REACTOR_EMIT_DEBUG_INFO)

set(LLVM_INCLUDE_DIR "")

if(WIN32)
    list(APPEND LLVM_INCLUDE_DIR ${SWIFTSHADER_LLVM_CONFIG_DIR}/windows/include)
elseif(LINUX)
    list(APPEND LLVM_INCLUDE_DIR ${SWIFTSHADER_LLVM_CONFIG_DIR}/linux/include)
    list(APPEND SWIFTSHADER_LIBBACKTRACE_INCLUDE_DIR ${SWIFTSHADER_LIBBACKTRACE_CONFIG_DIR}/linux/include)
elseif(APPLE)
    list(APPEND LLVM_INCLUDE_DIR ${SWIFTSHADER_LLVM_CONFIG_DIR}/darwin/include)
    list(APPEND SWIFTSHADER_LIBBACKTRACE_INCLUDE_DIR ${SWIFTSHADER_LIBBACKTRACE_CONFIG_DIR}/darwin/include)
elseif(ANDROID)
    list(APPEND LLVM_INCLUDE_DIR ${SWIFTSHADER_LLVM_CONFIG_DIR}/android/include)
    list(APPEND SWIFTSHADER_LIBBACKTRACE_INCLUDE_DIR ${SWIFTSHADER_LIBBACKTRACE_CONFIG_DIR}/android/include)
endif()

list(APPEND LLVM_INCLUDE_DIR
    ${SWIFTSHADER_LLVM_DIR}/include
    ${SWIFTSHADER_LLVM_DIR}/lib/Target/AArch64
    ${SWIFTSHADER_LLVM_DIR}/lib/Target/ARM
    ${SWIFTSHADER_LLVM_DIR}/lib/Target/Mips
    ${SWIFTSHADER_LLVM_DIR}/lib/Target/PowerPC
    ${SWIFTSHADER_LLVM_DIR}/lib/Target/X86
    ${SWIFTSHADER_LLVM_CONFIG_DIR}/common/include
    ${SWIFTSHADER_LLVM_CONFIG_DIR}/common/lib/IR
    ${SWIFTSHADER_LLVM_CONFIG_DIR}/common/lib/Target/AArch64
    ${SWIFTSHADER_LLVM_CONFIG_DIR}/common/lib/Target/ARM
    ${SWIFTSHADER_LLVM_CONFIG_DIR}/common/lib/Target/Mips
    ${SWIFTSHADER_LLVM_CONFIG_DIR}/common/lib/Target/PowerPC
    ${SWIFTSHADER_LLVM_CONFIG_DIR}/common/lib/Target/X86
    ${SWIFTSHADER_LLVM_CONFIG_DIR}/common/lib/Transforms/InstCombine
)

# Suppress known LLVM warnings
set(LLVM_COMPILE_OPTIONS)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    list(APPEND LLVM_COMPILE_OPTIONS
        "-Wno-noexcept-type"           # mangled name for ‘X’ will change in C++17 because the exception specification is part of a function type
        "-Wno-maybe-uninitialized"     # ‘X’ may be used uninitialized in this function
        "-Wno-unused-but-set-variable" # variable ‘X’ set but not used
    )
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8)
        list(APPEND LLVM_COMPILE_OPTIONS
            "-Wno-class-memaccess"  # memset/memcmp used on non-trivial class object
            "-Wno-attributes"	# declared inline but not inline
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    list(APPEND LLVM_COMPILE_OPTIONS
        "/wd4141" # 'inline': used more than once
        "/wd4146" # unary minus operator applied to unsigned type, result still unsigned
        "/wd4244" # 'X': conversion from 'uint64_t' to 'const unsigned int', possible loss of data
        "/wd4267" # 'X': conversion from 'size_t' to 'unsigned int', possible loss of data
        "/wd4291" # 'void X new(size_t,unsigned int,unsigned int)': no matching operator delete found; memory will not be freed if initialization throws an exception
        "/wd4319" # 'X': zero extending 'unsigned int' to 'uint64_t' of greater size
        "/wd4624" # 'X': destructor was implicitly defined as deleted
        "/wd4805" # 'X': unsafe mix of type 'unsigned int' and type 'bool' in operation
        "/wd4996" # 'X': Call to 'Y' with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct.
    )
endif()

add_library(llvm STATIC ${LLVM_LIST})
set_target_properties(llvm PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    INCLUDE_DIRECTORIES "${LLVM_INCLUDE_DIR}"
    COMPILE_OPTIONS "${LLVM_COMPILE_OPTIONS}"
    COMPILE_DEFINITIONS "__STDC_CONSTANT_MACROS; __STDC_LIMIT_MACROS;"
    FOLDER "LLVM"
)

# Add required libraries for LLVM
if(LINUX)
    target_link_libraries(llvm dl)
endif(LINUX)

###########################################################
# Subzero
###########################################################

if(${REACTOR_BACKEND} STREQUAL "Subzero")
    set(SUBZERO_LIST
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceAssembler.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceCfg.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceCfgNode.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceClFlags.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceELFObjectWriter.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceELFSection.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceFixups.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceGlobalContext.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceGlobalInits.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceInst.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceInstrumentation.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceIntrinsics.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceLiveness.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceLoopAnalyzer.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceMangling.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceMemory.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceOperand.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceRangeSpec.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceRegAlloc.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceRevision.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceRNG.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceSwitchLowering.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceTargetLowering.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceThreading.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceTimerTree.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceTypes.cpp
        ${SWIFTSHADER_SUBZERO_DIR}/src/IceVariableSplitting.cpp
    )

    # FIXME: Shouldn't depend on external source files directly.
    list(APPEND SUBZERO_LIST
        ${SWIFTSHADER_SOURCE_DIR}/Common/Memory.cpp
    )

    if(ARCH STREQUAL "x86_64")
        list(APPEND SUBZERO_LIST
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceTargetLoweringX86.cpp
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceInstX8664.cpp
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceTargetLoweringX8664.cpp
        )
        set(SUBZERO_TARGET X8664)
    elseif(ARCH STREQUAL "x86")
        list(APPEND SUBZERO_LIST
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceTargetLoweringX86.cpp
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceInstX8632.cpp
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceTargetLoweringX8632.cpp
        )
        set(SUBZERO_TARGET X8632)
    elseif(ARCH STREQUAL "arm")
        list(APPEND SUBZERO_LIST
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceAssemblerARM32.cpp
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceInstARM32.cpp
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceTargetLoweringARM32.cpp
        )
        set(SUBZERO_TARGET ARM32)
    elseif(ARCH STREQUAL "mipsel")
        list(APPEND SUBZERO_LIST
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceAssemblerMIPS32.cpp
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceInstMIPS32.cpp
            ${SWIFTSHADER_SUBZERO_DIR}/src/IceTargetLoweringMIPS32.cpp
        )
        set(SUBZERO_TARGET MIPS32)
    else()
        message(FATAL_ERROR "Architecture '${ARCH}' not supported by Subzero")
    endif()

    file(GLOB_RECURSE SUBZERO_DEPENDENCIES_LIST
        ${SWIFTSHADER_SUBZERO_LLVM_DIR}/*.cpp
        ${SWIFTSHADER_SUBZERO_LLVM_DIR}/*.c
        ${SWIFTSHADER_SUBZERO_LLVM_DIR}/*.h
    )

    set(SUBZERO_REACTOR_LIST
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/Debug.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/Debug.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/EmulatedReactor.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/ExecutableMemory.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/ExecutableMemory.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/Nucleus.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/Optimizer.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/Print.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/Reactor.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/Reactor.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/Routine.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/SubzeroReactor.cpp
    )

    set(SUBZERO_INCLUDE_DIR
        ${SWIFTSHADER_SUBZERO_DIR}/
        ${SWIFTSHADER_SUBZERO_LLVM_DIR}/include/
        ${SWIFTSHADER_SUBZERO_DIR}/pnacl-llvm/include/
    )

    if(WIN32)
        list(APPEND SUBZERO_INCLUDE_DIR ${SWIFTSHADER_SUBZERO_LLVM_DIR}/build/Windows/include/)
    elseif(LINUX)
        list(APPEND SUBZERO_INCLUDE_DIR ${SWIFTSHADER_SUBZERO_LLVM_DIR}/build/Linux/include/)
    elseif(APPLE)
        list(APPEND SUBZERO_INCLUDE_DIR ${SWIFTSHADER_SUBZERO_LLVM_DIR}/build/MacOS/include/)
    endif()

    if(WIN32)
        list(APPEND SUBZERO_COMPILE_OPTIONS
            "/wd4146" # unary minus operator applied to unsigned type, result still unsigned
            "/wd4334" # ''operator' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
            "/wd4996" # The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: new_name.
        )
    endif()

    add_library(ReactorSubzero STATIC
        ${SUBZERO_LIST}
        ${SUBZERO_DEPENDENCIES_LIST}
        ${SUBZERO_REACTOR_LIST}
    )
    set_target_properties(ReactorSubzero PROPERTIES
        POSITION_INDEPENDENT_CODE 1
        INCLUDE_DIRECTORIES "${SUBZERO_INCLUDE_DIR}"
        COMPILE_OPTIONS "${SUBZERO_COMPILE_OPTIONS};${SWIFTSHADER_COMPILE_OPTIONS}"
        COMPILE_DEFINITIONS "SZTARGET=${SUBZERO_TARGET}; ALLOW_DUMP=0; ALLOW_TIMERS=0; ALLOW_LLVM_CL=0; ALLOW_LLVM_IR=0; ALLOW_LLVM_IR_AS_INPUT=0; ALLOW_MINIMAL_BUILD=0; ALLOW_WASM=0; ICE_THREAD_LOCAL_HACK=0;"
        FOLDER "Subzero"
    )

    if(WIN32)
        target_compile_definitions(ReactorSubzero PRIVATE SUBZERO_USE_MICROSOFT_ABI)
    endif()
endif()

###########################################################
# Include Directories
###########################################################

set(COMMON_INCLUDE_DIR
    ${SWIFTSHADER_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIR}
    ${SWIFTSHADER_LIBBACKTRACE_INCLUDE_DIR}
)
set(OPENGL_INCLUDE_DIR
    ${SWIFTSHADER_OPENGL_DIR}
    ${COMMON_INCLUDE_DIR}
)

set(VULKAN_INCLUDE_DIR
    ${COMMON_INCLUDE_DIR}
    ${SWIFTSHADER_MARL_INCLUDE_DIR}
    ${SWIFTSHADER_CPPDAP_INCLUDE_DIR}
)

###########################################################
# File Lists
###########################################################

file(GLOB_RECURSE SWIFTSHADER_LIST
    ${SWIFTSHADER_SOURCE_DIR}/Common/*.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Common/*.h
    ${SWIFTSHADER_SOURCE_DIR}/Common/*.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Renderer/*.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Renderer/*.h
    ${SWIFTSHADER_SOURCE_DIR}/Renderer/*.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Shader/*.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Shader/*.h
    ${SWIFTSHADER_SOURCE_DIR}/Shader/*.hpp
)
list(APPEND SWIFTSHADER_LIST
    ${SWIFTSHADER_SOURCE_DIR}/Main/Config.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Main/Config.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBuffer.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBuffer.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Main/SwiftConfig.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Main/SwiftConfig.hpp
)
list(REMOVE_ITEM SWIFTSHADER_LIST
    ${SWIFTSHADER_SOURCE_DIR}/Common/DebugAndroid.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Common/DebugAndroid.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Common/GrallocAndroid.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Common/GrallocAndroid.hpp
)

if(REACTOR_EMIT_DEBUG_INFO)
    set(LIBBACKTRACE_LIST
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/atomic.c
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/backtrace.c
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/backtrace.h
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/dwarf.c
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/fileline.c
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/filenames.h
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/internal.h
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/mmap.c
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/mmapio.c
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/posix.c
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/print.c
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/simple.c
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/sort.c
        ${SWIFTSHADER_LIBBACKTRACE_DIR}/state.c
        ${SWIFTSHADER_LIBBACKTRACE_INCLUDE_DIR}/backtrace-supported.h
        ${SWIFTSHADER_LIBBACKTRACE_INCLUDE_DIR}/config.h
    )

    if(WIN32)
        list(APPEND LIBBACKTRACE_LIST ${SWIFTSHADER_LIBBACKTRACE_DIR}/pecoff.c)
    elseif(LINUX)
        list(APPEND LIBBACKTRACE_LIST ${SWIFTSHADER_LIBBACKTRACE_DIR}/elf.c)
    elseif(APPLE)
        message(FATAL_ERROR "libbacktrace does not support mach-o yet")
    endif()
endif(REACTOR_EMIT_DEBUG_INFO)

set(REACTOR_LLVM_LIST
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/CPUID.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/CPUID.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/Debug.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/Debug.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/EmulatedReactor.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/ExecutableMemory.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/ExecutableMemory.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/LLVMReactor.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/LLVMReactor.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/LLVMReactorDebugInfo.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/LLVMReactorDebugInfo.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/Nucleus.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/Print.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/Reactor.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/Reactor.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Reactor/Routine.hpp
)

file(GLOB_RECURSE EGL_LIST
    ${SWIFTSHADER_OPENGL_DIR}/libEGL/*.cpp
    ${SWIFTSHADER_OPENGL_DIR}/libEGL/*.h
    ${SWIFTSHADER_OPENGL_DIR}/libEGL/*.hpp
    ${SWIFTSHADER_OPENGL_DIR}/common/Object.cpp
    ${SWIFTSHADER_OPENGL_DIR}/common/Object.hpp
    ${SWIFTSHADER_OPENGL_DIR}/common/debug.cpp
    ${SWIFTSHADER_OPENGL_DIR}/common/debug.h
    ${SWIFTSHADER_SOURCE_DIR}/Common/SharedLibrary.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Common/SharedLibrary.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

file(GLOB_RECURSE GLES2_LIST
    ${SWIFTSHADER_OPENGL_DIR}/libGLESv2/*.cpp
    ${SWIFTSHADER_OPENGL_DIR}/libGLESv2/*.h
    ${SWIFTSHADER_OPENGL_DIR}/libGLESv2/*.hpp
    ${SWIFTSHADER_OPENGL_DIR}/common/*.cpp
    ${SWIFTSHADER_OPENGL_DIR}/common/*.h
    ${SWIFTSHADER_OPENGL_DIR}/common/*.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Common/SharedLibrary.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Common/SharedLibrary.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GLES2/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GLES3/*.h
)

file(GLOB_RECURSE GLES_CM_LIST
    ${SWIFTSHADER_OPENGL_DIR}/libGLES_CM/*.cpp
    ${SWIFTSHADER_OPENGL_DIR}/libGLES_CM/*.h
    ${SWIFTSHADER_OPENGL_DIR}/libGLES_CM/*.hpp
    ${SWIFTSHADER_OPENGL_DIR}/common/*.cpp
    ${SWIFTSHADER_OPENGL_DIR}/common/*.h
    ${SWIFTSHADER_OPENGL_DIR}/common/*.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Common/SharedLibrary.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Common/SharedLibrary.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GLES/*.h
)

file(GLOB_RECURSE OPENGL_COMPILER_LIST
    ${SWIFTSHADER_OPENGL_COMPILER_DIR}/*.cpp
    ${SWIFTSHADER_OPENGL_COMPILER_DIR}/*.h
    ${SWIFTSHADER_OPENGL_COMPILER_DIR}/*.hpp
)
file(GLOB_RECURSE OPENGL_COMPILER_OS_SPECIFIC_LIST
    ${SWIFTSHADER_OPENGL_COMPILER_DIR}/ossource*.cpp
    ${SWIFTSHADER_OPENGL_COMPILER_DIR}/ossource*.h
    ${SWIFTSHADER_OPENGL_COMPILER_DIR}/ossource*.hpp
)
list(REMOVE_ITEM OPENGL_COMPILER_LIST ${OPENGL_COMPILER_OS_SPECIFIC_LIST})

file(GLOB VULKAN_LIST
    ${SWIFTSHADER_VULKAN_DIR}/*.cpp
    ${SWIFTSHADER_VULKAN_DIR}/*.h
    ${SWIFTSHADER_VULKAN_DIR}/*.hpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Build.cpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Build.hpp
    ${SWIFTSHADER_SOURCE_DIR}/System/CPUID.cpp
    ${SWIFTSHADER_SOURCE_DIR}/System/CPUID.hpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Configurator.cpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Configurator.hpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Debug.cpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Debug.hpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Half.cpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Half.hpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Math.cpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Math.hpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Memory.cpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Memory.hpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Socket.cpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Socket.hpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Synchronization.hpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Timer.cpp
    ${SWIFTSHADER_SOURCE_DIR}/System/Timer.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Device/*.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Device/*.hpp
    ${SWIFTSHADER_SOURCE_DIR}/Pipeline/*.cpp
    ${SWIFTSHADER_SOURCE_DIR}/Pipeline/*.hpp
    ${SWIFTSHADER_SOURCE_DIR}/WSI/VkSurfaceKHR.cpp
    ${SWIFTSHADER_SOURCE_DIR}/WSI/VkSurfaceKHR.hpp
    ${SWIFTSHADER_SOURCE_DIR}/WSI/VkSwapchainKHR.cpp
    ${SWIFTSHADER_SOURCE_DIR}/WSI/VkSwapchainKHR.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/*.h}
)

if(SWIFTSHADER_ENABLE_VULKAN_DEBUGGER)
    file(GLOB_RECURSE VULKAN_DEBUG_LIST
        ${SWIFTSHADER_VULKAN_DIR}/Debug/*.cpp
        ${SWIFTSHADER_VULKAN_DIR}/Debug/*.h
        ${SWIFTSHADER_VULKAN_DIR}/Debug/*.hpp
    )
    list(APPEND VULKAN_LIST ${VULKAN_DEBUG_LIST})
    list(APPEND SWIFTSHADER_COMPILE_OPTIONS "-DENABLE_VK_DEBUGGER")
endif(SWIFTSHADER_ENABLE_VULKAN_DEBUGGER)

if(LINUX OR ANDROID)
    list(APPEND VULKAN_LIST
        ${SWIFTSHADER_SOURCE_DIR}/System/Linux/MemFd.cpp
	${SWIFTSHADER_SOURCE_DIR}/System/Linux/MemFd.hpp)
endif(LINUX OR ANDROID)

###########################################################
# Append OS specific files to lists
###########################################################

if(WIN32)
    list(APPEND SWIFTSHADER_LIST
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferDD.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferDD.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferGDI.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferGDI.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferWin.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferWin.hpp
    )
    list(APPEND OPENGL_COMPILER_LIST ${SWIFTSHADER_OPENGL_COMPILER_DIR}/ossource_win.cpp)
    list(APPEND EGL_LIST ${SWIFTSHADER_OPENGL_DIR}/libEGL/libEGL.rc)
    list(APPEND GLES2_LIST ${SWIFTSHADER_OPENGL_DIR}/libGLESv2/libGLESv2.rc)
    list(APPEND GLES_CM_LIST ${SWIFTSHADER_OPENGL_DIR}/libGLES_CM/libGLES_CM.rc)
    list(APPEND VULKAN_LIST
        ${SWIFTSHADER_VULKAN_DIR}/Vulkan.rc
        ${SWIFTSHADER_SOURCE_DIR}/WSI/Win32SurfaceKHR.cpp
        ${SWIFTSHADER_SOURCE_DIR}/WSI/Win32SurfaceKHR.hpp
    )
elseif(LINUX)
    list(APPEND SWIFTSHADER_LIST
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferX11.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferX11.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Common/SharedLibrary.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/libX11.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/libX11.hpp
    )
    list(APPEND EGL_LIST
        ${SWIFTSHADER_SOURCE_DIR}/Common/SharedLibrary.hpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/libX11.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/libX11.hpp
    )
    list(APPEND OPENGL_COMPILER_LIST
        ${SWIFTSHADER_OPENGL_COMPILER_DIR}/ossource_posix.cpp
    )

    if(X11)
        list(APPEND VULKAN_LIST
            ${SWIFTSHADER_SOURCE_DIR}/WSI/XlibSurfaceKHR.cpp
            ${SWIFTSHADER_SOURCE_DIR}/WSI/XlibSurfaceKHR.hpp
            ${SWIFTSHADER_SOURCE_DIR}/WSI/libX11.cpp
            ${SWIFTSHADER_SOURCE_DIR}/WSI/libX11.hpp
        )
    endif(X11)

    if(XCB)
        list(APPEND VULKAN_LIST
            ${SWIFTSHADER_SOURCE_DIR}/WSI/XcbSurfaceKHR.cpp
            ${SWIFTSHADER_SOURCE_DIR}/WSI/XcbSurfaceKHR.hpp
        )
    endif(XCB)

elseif(APPLE)
    list(APPEND SWIFTSHADER_LIST
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferOSX.mm
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferOSX.hpp
    )
    list(APPEND EGL_LIST
        ${SWIFTSHADER_OPENGL_DIR}/libEGL/OSXUtils.mm
        ${SWIFTSHADER_OPENGL_DIR}/libEGL/OSXUtils.hpp
    )
    list(APPEND OPENGL_COMPILER_LIST
        ${SWIFTSHADER_OPENGL_COMPILER_DIR}/ossource_posix.cpp
    )

    list(APPEND VULKAN_LIST
        ${SWIFTSHADER_SOURCE_DIR}/WSI/MetalSurface.mm
        ${SWIFTSHADER_SOURCE_DIR}/WSI/MetalSurface.h
    )
elseif(ANDROID)
    list(APPEND SWIFTSHADER_LIST
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferAndroid.cpp
        ${SWIFTSHADER_SOURCE_DIR}/Main/FrameBufferAndroid.hpp
    )
    list(APPEND OPENGL_COMPILER_LIST
        ${SWIFTSHADER_OPENGL_COMPILER_DIR}/ossource_posix.cpp
    )
endif()

if(WIN32)
    set(OS_LIBS odbc32 odbccp32 WS2_32 dxguid)
elseif(LINUX)
    set(OS_LIBS dl pthread)
elseif(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(QUARTZ_FRAMEWORK Quartz)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(IOSURFACE_FRAMEWORK IOSurface)
    set(OS_LIBS "${COCOA_FRAMEWORK}" "${QUARTZ_FRAMEWORK}" "${CORE_FOUNDATION_FRAMEWORK}" "${IOSURFACE_FRAMEWORK}")
endif()

###########################################################
# SwiftShader Targets
###########################################################

add_library(SwiftShader STATIC ${SWIFTSHADER_LIST})
set_target_properties(SwiftShader PROPERTIES
    INCLUDE_DIRECTORIES "${COMMON_INCLUDE_DIR}"
    POSITION_INDEPENDENT_CODE 1
    FOLDER "Core"
    COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
    COMPILE_DEFINITIONS "NO_SANITIZE_FUNCTION=;"
)
target_link_libraries(SwiftShader ${OS_LIBS})

if(${REACTOR_BACKEND} STREQUAL "LLVM")
    add_library(ReactorLLVM STATIC ${REACTOR_LLVM_LIST})
    set_target_properties(ReactorLLVM PROPERTIES
        INCLUDE_DIRECTORIES "${COMMON_INCLUDE_DIR}"
        POSITION_INDEPENDENT_CODE 1
        FOLDER "Core"
        COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
    )
    target_link_libraries(ReactorLLVM llvm ${OS_LIBS})

    if(REACTOR_EMIT_DEBUG_INFO)
        if(WIN32)
            # Boost stacktrace uses COM on Windows.
            # On Windows, cache COM instances in TLS for performance.
            target_compile_definitions(ReactorLLVM PRIVATE BOOST_STACKTRACE_USE_WINDBG_CACHED)
        else()
            # Boost stacktrace uses libbacktrace
            target_compile_definitions(ReactorLLVM PRIVATE BOOST_STACKTRACE_USE_BACKTRACE)

            # Boost stacktrace uses exceptions, so force enable it
            set_cpp_flag("-fexceptions")

            add_library(Libbacktrace STATIC ${LIBBACKTRACE_LIST})
            set_target_properties(Libbacktrace PROPERTIES
                INCLUDE_DIRECTORIES "${SWIFTSHADER_LIBBACKTRACE_INCLUDE_DIR}"
                POSITION_INDEPENDENT_CODE 1
                FOLDER "Core"
            )
            target_link_libraries(Libbacktrace ${OS_LIBS})
            target_link_libraries(ReactorLLVM Libbacktrace)
        endif()

        # Download boost and use it
        DownloadBoost()
        set(BOOST_INCLUDEDIR ${BOOST_THIRD_PARTY_DIR})
        find_package(Boost REQUIRED)
        target_link_libraries(ReactorLLVM Boost::boost)

    endif(REACTOR_EMIT_DEBUG_INFO)

    set(Reactor ReactorLLVM)
elseif(${REACTOR_BACKEND} STREQUAL "Subzero")
    set(Reactor ReactorSubzero)
else()
    message(FATAL_ERROR "REACTOR_BACKEND must be 'LLVM' or 'Subzero'")
endif()

add_library(GLCompiler STATIC ${OPENGL_COMPILER_LIST})
set_target_properties(GLCompiler PROPERTIES
    INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}"
    POSITION_INDEPENDENT_CODE 1
    FOLDER "OpenGL"
    COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
)
target_link_libraries(GLCompiler ${OS_LIBS})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(LIB_PREFIX "lib64")
else()
    set(LIB_PREFIX "lib")
endif()

if(SWIFTSHADER_BUILD_EGL)
    add_library(libEGL SHARED ${EGL_LIST})
    set_target_properties(libEGL PROPERTIES
        INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}"
        FOLDER "OpenGL"
        COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
        COMPILE_DEFINITIONS "EGL_EGLEXT_PROTOTYPES; EGLAPI=; NO_SANITIZE_FUNCTION=;$<$<CONFIG:Debug>:DEBUGGER_WAIT_DIALOG>"
        PREFIX ""
    )
    if(LINUX)
        if(X11)
            set_property(TARGET libEGL APPEND PROPERTY COMPILE_DEFINITIONS "USE_X11=1")
        endif(X11)
    endif()
    if (ANDROID)
        set_target_properties(libEGL PROPERTIES SUFFIX "_swiftshader.so")
    endif ()
    set_shared_library_export_map(libEGL ${SWIFTSHADER_SOURCE_DIR}/OpenGL/libEGL)
    target_link_libraries(libEGL ${OS_LIBS})
    add_custom_command(
        TARGET libEGL
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:libEGL>/translator
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libEGL> $<TARGET_FILE_DIR:libEGL>/translator/${LIB_PREFIX}EGL_translator${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libEGL> ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/
    )
endif()

if(SWIFTSHADER_BUILD_GLESv2)
    add_library(libGLESv2 SHARED ${GLES2_LIST})
    set_target_properties(libGLESv2 PROPERTIES
        INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}"
        FOLDER "OpenGL"
        COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
        COMPILE_DEFINITIONS "GL_GLEXT_PROTOTYPES; GL_API=; GL_APICALL=; GLAPI=; NO_SANITIZE_FUNCTION=;"
        PREFIX ""
    )
    if (ANDROID)
        set_target_properties(libGLESv2 PROPERTIES SUFFIX "_swiftshader.so")
    endif ()
    set_shared_library_export_map(libGLESv2 ${SWIFTSHADER_SOURCE_DIR}/OpenGL/libGLESv2)
    target_link_libraries(libGLESv2 SwiftShader ${Reactor} GLCompiler ${OS_LIBS})
    add_custom_command(
        TARGET libGLESv2
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:libGLESv2>/translator
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libGLESv2> $<TARGET_FILE_DIR:libGLESv2>/translator/${LIB_PREFIX}GLES_V2_translator${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libGLESv2> ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/
    )
endif()

if(SWIFTSHADER_BUILD_GLES_CM)
    add_library(libGLES_CM SHARED ${GLES_CM_LIST})
    set_target_properties(libGLES_CM PROPERTIES
        INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}"
        FOLDER "OpenGL"
        COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
        COMPILE_DEFINITIONS "GL_GLEXT_PROTOTYPES; EGLAPI=; GL_API=; GL_APICALL=; GLAPI=;"
        PREFIX ""
    )
    if (ANDROID)
        set_target_properties(libGLES_CM PROPERTIES SUFFIX "_swiftshader.so")
    endif ()
    set_shared_library_export_map(libGLES_CM ${SWIFTSHADER_SOURCE_DIR}/OpenGL/libGLES_CM)
    target_link_libraries(libGLES_CM SwiftShader ${Reactor} GLCompiler ${OS_LIBS})
    add_custom_command(
        TARGET libGLES_CM
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:libGLES_CM>/translator
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libGLES_CM> $<TARGET_FILE_DIR:libGLES_CM>/translator/${LIB_PREFIX}GLES_CM_translator${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
endif()

if(BUILD_MARL)
    set(MARL_THIRD_PARTY_DIR ${SWIFTSHADER_THIRD_PARTY_DIR})
    add_subdirectory(third_party/marl)
endif(BUILD_MARL)

if(SWIFTSHADER_BUILD_CPPDAP)
    set(CPPDAP_THIRD_PARTY_DIR ${SWIFTSHADER_THIRD_PARTY_DIR})
    add_subdirectory(${SWIFTSHADER_CPPDAP_DIR})
endif(SWIFTSHADER_BUILD_CPPDAP)

if(SWIFTSHADER_BUILD_VULKAN)
    add_library(vk_swiftshader SHARED ${VULKAN_LIST})

    if (NOT TARGET SPIRV-Tools)
        # This variable is also used by SPIRV-Tools to locate SPIRV-Headers
        set(SPIRV-Headers_SOURCE_DIR "${SWIFTSHADER_THIRD_PARTY_DIR}/SPIRV-Headers")
        list(APPEND VULKAN_INCLUDE_DIR "${SPIRV-Headers_SOURCE_DIR}/include")

        add_subdirectory(third_party/SPIRV-Tools)
    endif(NOT TARGET SPIRV-Tools)

    # Copy the OpenCLDebugInfo100.h header that's generated by SPIRV-Tools
    # out to a separate directory that can be added to the include path.
    # Ideally, this header would just be pre-built and part of SPIRV-Headers.
    # See: https://github.com/KhronosGroup/SPIRV-Headers/issues/137
    set(SPIRV_TOOLS_EXT_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/spirv-tools-ext/include)
    add_custom_target(spirv_tools_ext_includes
        DEPENDS spirv-tools-header-OpenCLDebugInfo100
        COMMAND ${CMAKE_COMMAND} -E copy
            ${spirv-tools_BINARY_DIR}/OpenCLDebugInfo100.h
            ${SPIRV_TOOLS_EXT_INC_DIR}/spirv-tools/ext/OpenCLDebugInfo100.h
    )
    list(APPEND VULKAN_INCLUDE_DIR "${SPIRV_TOOLS_EXT_INC_DIR}")
    add_dependencies(vk_swiftshader spirv_tools_ext_includes)

    set_target_properties(vk_swiftshader PROPERTIES
        INCLUDE_DIRECTORIES "${VULKAN_INCLUDE_DIR}"
        FOLDER "Vulkan"
        COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
        COMPILE_DEFINITIONS "VK_EXPORT=;NO_SANITIZE_FUNCTION=;$<$<CONFIG:Debug>:DEBUGGER_WAIT_DIALOG>"
    )

    if(WIN32)
        set_property(TARGET vk_swiftshader APPEND
                     PROPERTY COMPILE_DEFINITIONS "VK_USE_PLATFORM_WIN32_KHR")
    elseif(LINUX)
        if(X11)
            set_property(TARGET vk_swiftshader APPEND
                        PROPERTY COMPILE_DEFINITIONS "VK_USE_PLATFORM_XLIB_KHR")
        endif(X11)
        if(XCB)
            set_property(TARGET vk_swiftshader APPEND
                        PROPERTY COMPILE_DEFINITIONS "VK_USE_PLATFORM_XCB_KHR")
        endif(XCB)
    elseif(APPLE)
        set_property(TARGET vk_swiftshader APPEND
                     PROPERTY COMPILE_DEFINITIONS "VK_USE_PLATFORM_MACOS_MVK")
    endif()

    set_shared_library_export_map(vk_swiftshader ${SWIFTSHADER_SOURCE_DIR}/Vulkan)

    set(VK_SWIFTSHADER_LIBS ${Reactor} marl ${OS_LIBS} SPIRV-Tools SPIRV-Tools-opt)
    if(SWIFTSHADER_ENABLE_VULKAN_DEBUGGER)
        list(APPEND VK_SWIFTSHADER_LIBS cppdap)
    endif(SWIFTSHADER_ENABLE_VULKAN_DEBUGGER)
    target_link_libraries(vk_swiftshader ${VK_SWIFTSHADER_LIBS})

    add_custom_command(
        TARGET vk_swiftshader
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:vk_swiftshader>/translator
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vk_swiftshader> $<TARGET_FILE_DIR:vk_swiftshader>/translator/${LIB_PREFIX}Vulkan_translator${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vk_swiftshader> ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/
    )

    # The vk_swiftshader_icd.json manifest file will point to ICD_LIBRARY_PATH.
    # Set ICD_LIBRARY_PATH to be a relative path similar to "./libvk_swiftshader.so", so both files can be moved.
    # A relative path is relative to the manifest file.
    set(ICD_LIBRARY_PATH "${CMAKE_SHARED_LIBRARY_PREFIX}vk_swiftshader${CMAKE_SHARED_LIBRARY_SUFFIX}")
    if(WIN32)
        # The path is output to a JSON file, which requires backslashes to be escaped.
        set(ICD_LIBRARY_PATH ".\\\\${ICD_LIBRARY_PATH}")
    else()
        set(ICD_LIBRARY_PATH "./${ICD_LIBRARY_PATH}")
    endif()
    configure_file(
        "${SWIFTSHADER_VULKAN_DIR}/vk_swiftshader_icd.json.tmpl"
        "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/vk_swiftshader_icd.json")
endif()

###########################################################
# Sample programs
###########################################################

if(SWIFTSHADER_BUILD_SAMPLES)
    if(WIN32)
        add_executable(OGLES2HelloAPI ${SWIFTSHADER_HELLO2_DIR}/OGLES2HelloAPI_Windows.cpp)
        set_target_properties(OGLES2HelloAPI PROPERTIES
            INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
            COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
            COMPILE_DEFINITIONS "GL_GLEXT_PROTOTYPES"
            FOLDER "Samples"
        )
        target_link_libraries(OGLES2HelloAPI libEGL libGLESv2)
        set_property(TARGET OGLES2HelloAPI APPEND_STRING PROPERTY LINK_FLAGS "/SUBSYSTEM:WINDOWS")
        set(CMAKE_DEFAULT_STARTUP_PROJECT OGLES2HelloAPI)
    elseif(LINUX)
        add_executable(OGLES2HelloAPI ${SWIFTSHADER_HELLO2_DIR}/OGLES2HelloAPI_LinuxX11.cpp)
        set_target_properties(OGLES2HelloAPI PROPERTIES
            INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
            COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
            COMPILE_DEFINITIONS "GL_GLEXT_PROTOTYPES"
        )
        target_link_libraries(OGLES2HelloAPI dl ${X11} libEGL libGLESv2)   # Explicitly link our "lib*" targets, not the platform provided "EGL" and "GLESv2"
    elseif(APPLE)
        add_executable(OGLES2HelloAPI MACOSX_BUNDLE
            ${SWIFTSHADER_HELLO2_DIR}/OGLES2HelloAPI_OSX.mm
            ${SWIFTSHADER_HELLO2_DIR}/Build/OSX/en.lproj/MainMenu.xib
        )
        set_target_properties(OGLES2HelloAPI PROPERTIES
            INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
            COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS};-Wno-deprecated-declarations" # 'NSTitledWindowMask', 'NSClosableWindowMask' is deprecated
            COMPILE_DEFINITIONS "GL_GLEXT_PROTOTYPES"
            MACOSX_BUNDLE_INFO_PLIST "${SWIFTSHADER_HELLO2_DIR}/Build/OSX/Info.plist"
        )
        target_link_libraries(OGLES2HelloAPI libEGL libGLESv2 ${OS_LIBS})
        set_source_files_properties(${SWIFTSHADER_HELLO2_DIR}/Build/OSX/en.lproj/MainMenu.xib PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
    endif()
endif()


if(SWIFTSHADER_BUILD_TESTS)
    set(REACTOR_UNIT_TESTS_LIST
        ${SWIFTSHADER_SOURCE_DIR}/Reactor/ReactorUnitTests.cpp
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/src/gtest-all.cc
    )

    set(REACTOR_UNIT_TESTS_INCLUDE_DIR
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/include
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/
    )

    add_executable(ReactorUnitTests ${REACTOR_UNIT_TESTS_LIST})
    set_target_properties(ReactorUnitTests PROPERTIES
        INCLUDE_DIRECTORIES "${REACTOR_UNIT_TESTS_INCLUDE_DIR}"
        COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
        FOLDER "Tests"
    )

    if(NOT WIN32 AND ${REACTOR_BACKEND} STREQUAL "Subzero")
        target_link_libraries(ReactorUnitTests ${Reactor} pthread dl)
    else()
        target_link_libraries(ReactorUnitTests ${Reactor})
    endif()
endif()

if(SWIFTSHADER_BUILD_TESTS)
    set(GLES_UNITTESTS_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/GLESUnitTests/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/GLESUnitTests/unittests.cpp
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/src/gtest-all.cc
    )

    set(GLES_UNITTESTS_INCLUDE_DIR
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/include/
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googlemock/include/
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
    )

    add_executable(gles-unittests ${GLES_UNITTESTS_LIST})
    set_target_properties(gles-unittests PROPERTIES
        INCLUDE_DIRECTORIES "${GLES_UNITTESTS_INCLUDE_DIR}"
        FOLDER "Tests"
        COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
        COMPILE_DEFINITIONS "STANDALONE"
    )

    target_link_libraries(gles-unittests libEGL libGLESv2 ${OS_LIBS})
    if(ANDROID)
        target_link_libraries(gles-unittests -landroid)
    endif()
endif(SWIFTSHADER_BUILD_TESTS)

if(SWIFTSHADER_BUILD_TESTS)
    # Math unit tests
    set(MATH_UNITTESTS_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/MathUnitTests/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/MathUnitTests/unittests.cpp
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/src/gtest-all.cc
    )

    set(MATH_UNITTESTS_INCLUDE_DIR
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/include/
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googlemock/include/
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
    )

    add_executable(math-unittests ${MATH_UNITTESTS_LIST})
    set_target_properties(math-unittests PROPERTIES
        INCLUDE_DIRECTORIES "${MATH_UNITTESTS_INCLUDE_DIR}"
        FOLDER "Tests"
        COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
    )

    if(NOT WIN32)
        target_link_libraries(math-unittests pthread)
    endif()
endif()

if(SWIFTSHADER_BUILD_TESTS AND SWIFTSHADER_BUILD_VULKAN)
    set(VK_UNITTESTS_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/VulkanUnitTests/Device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/VulkanUnitTests/Driver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/VulkanUnitTests/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/VulkanUnitTests/unittests.cpp
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/src/gtest-all.cc
    )

    set(VK_UNITTESTS_INCLUDE_DIR
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/include/
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googlemock/include/
        ${SWIFTSHADER_THIRD_PARTY_DIR}/googletest/googletest/
        ${SWIFTSHADER_THIRD_PARTY_DIR}/SPIRV-Tools/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
    )

    add_executable(vk-unittests ${VK_UNITTESTS_LIST})
    add_dependencies(vk-unittests vk_swiftshader)

    set_target_properties(vk-unittests PROPERTIES
        INCLUDE_DIRECTORIES "${VK_UNITTESTS_INCLUDE_DIR}"
        FOLDER "Tests"
        COMPILE_OPTIONS "${SWIFTSHADER_COMPILE_OPTIONS}"
        COMPILE_DEFINITIONS "STANDALONE"
    )

    target_link_libraries(vk-unittests ${OS_LIBS} SPIRV-Tools)
endif(SWIFTSHADER_BUILD_TESTS AND SWIFTSHADER_BUILD_VULKAN)
