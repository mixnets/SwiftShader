# Copyright 2020 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# set(ROOT_PROJECT_COMPILE_OPTIONS
#     ${SWIFTSHADER_COMPILE_OPTIONS}
#     ${WARNINGS_AS_ERRORS}
# )

if(MSVC)
    add_compile_options("/MP")
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

set(SUBZERO_SRC_FILES
    src/IceAssembler.cpp
    src/IceCfg.cpp
    src/IceCfgNode.cpp
    src/IceClFlags.cpp
    src/IceELFObjectWriter.cpp
    src/IceELFSection.cpp
    src/IceFixups.cpp
    src/IceGlobalContext.cpp
    src/IceGlobalInits.cpp
    src/IceInst.cpp
    src/IceInstrumentation.cpp
    src/IceIntrinsics.cpp
    src/IceLiveness.cpp
    src/IceLoopAnalyzer.cpp
    src/IceMangling.cpp
    src/IceMemory.cpp
    src/IceOperand.cpp
    src/IceRangeSpec.cpp
    src/IceRegAlloc.cpp
    src/IceRevision.cpp
    src/IceRNG.cpp
    src/IceSwitchLowering.cpp
    src/IceTargetLowering.cpp
    src/IceThreading.cpp
    src/IceTimerTree.cpp
    src/IceTypes.cpp
    src/IceVariableSplitting.cpp
)

if(ARCH STREQUAL "x86_64")
    list(APPEND SUBZERO_SRC_FILES
        src/IceTargetLoweringX86.cpp
        src/IceInstX8664.cpp
        src/IceTargetLoweringX8664.cpp
    )
    set(SUBZERO_TARGET_CPU X8664)
elseif(ARCH STREQUAL "x86")
    list(APPEND SUBZERO_SRC_FILES
        src/IceTargetLoweringX86.cpp
        src/IceInstX8632.cpp
        src/IceTargetLoweringX8632.cpp
    )
    set(SUBZERO_TARGET_CPU X8632)
elseif(ARCH STREQUAL "arm")
    list(APPEND SUBZERO_SRC_FILES
        src/IceAssemblerARM32.cpp
        src/IceInstARM32.cpp
        src/IceTargetLoweringARM32.cpp
    )
    set(SUBZERO_TARGET_CPU ARM32)
elseif(ARCH STREQUAL "mipsel")
    list(APPEND SUBZERO_SRC_FILES
        src/IceAssemblerMIPS32.cpp
        src/IceInstMIPS32.cpp
        src/IceTargetLoweringMIPS32.cpp
    )
    set(SUBZERO_TARGET_CPU MIPS32)
else()
    message(WARNING "Architecture '${ARCH}' not supported by Subzero")
endif()

if(WIN32)
    list(APPEND SUBZERO_COMPILE_OPTIONS
        "/wd4146" # unary minus operator applied to unsigned type, result still unsigned
        "/wd4334" # ''operator' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
        "/wd4996" # The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: new_name.
    )
endif()

add_library(subzero STATIC EXCLUDE_FROM_ALL
    ${SUBZERO_SRC_FILES}
)

set_target_properties(subzero PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "Subzero"
)

target_include_directories(subzero
    PUBLIC
        # Add lib root as include dir, so client code can do #include "src/..."
        # TODO: Split out headers into separate 'include' directory.
        "."
        "pnacl-llvm/include"
)

target_compile_options(subzero
    PRIVATE
        # ${ROOT_PROJECT_COMPILE_OPTIONS}
        ${SUBZERO_COMPILE_OPTIONS}
)

target_compile_definitions(subzero
    PUBLIC
        "SZTARGET=${SUBZERO_TARGET_CPU}"
        "ALLOW_DUMP=0"
        "ALLOW_TIMERS=0"
        "ALLOW_LLVM_CL=0"
        "ALLOW_LLVM_IR=0"
        "ALLOW_LLVM_IR_AS_INPUT=0"
        "ALLOW_MINIMAL_BUILD=0"
        "ALLOW_WASM=0"
        "ICE_THREAD_LOCAL_HACK=0"
    PRIVATE
        $<$<BOOL:${WIN32}>:"SUBZERO_USE_MICROSOFT_ABI">
)

target_link_libraries(subzero
    PUBLIC
        llvm-subzero
)
