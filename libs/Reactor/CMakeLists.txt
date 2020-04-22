# Copyright 2020 The REACTOR Authors. All Rights Reserved.
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

cmake_minimum_required(VERSION 3.6.3)

set(CMAKE_CXX_STANDARD 14)

project(Reactor C CXX ASM)

# Options

function (option_if_not_defined name description default)
    if(NOT DEFINED ${name})
        option(${name} ${description} ${default})
    endif()
endfunction()

option_if_not_defined(REACTOR_EMIT_DEBUG_INFO "Emit debug info for JIT functions" FALSE)

# TODO:...
# set(DEFAULT_REACTOR_BACKEND "LLVM")
# set(REACTOR_BACKEND ${DEFAULT_REACTOR_BACKEND} CACHE STRING "JIT compiler back-end used by Reactor")
# set_property(CACHE REACTOR_BACKEND PROPERTY STRINGS LLVM Subzero)

set(DEFAULT_REACTOR_LLVM_VERSION "7.0")
set(REACTOR_LLVM_VERSION ${DEFAULT_REACTOR_LLVM_VERSION} CACHE STRING "LLVM version to use")
set_property(CACHE REACTOR_LLVM_VERSION PROPERTY STRINGS "7.0" "10.0")

if(MSVC)
    add_compile_options("/MP")
endif()

# Dependencies

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party)

if (NOT TARGET llvm-subzero)
    add_subdirectory(${THIRD_PARTY_DIR}/llvm-subzero third_party/llvm-subzero EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET subzero)
    add_subdirectory(${THIRD_PARTY_DIR}/subzero third_party/subzero EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET marl)
    set(MARL_THIRD_PARTY_DIR ${THIRD_PARTY_DIR})
    add_subdirectory(${THIRD_PARTY_DIR}/marl third_party/marl)
endif()

if (NOT TARGET llvm)
    add_subdirectory(${THIRD_PARTY_DIR}/llvm-${DEFAULT_REACTOR_LLVM_VERSION} third_party/llvm-${DEFAULT_REACTOR_LLVM_VERSION} EXCLUDE_FROM_ALL)
endif()

# Reactor

set(REACTOR_SRC_FILES
    include/Debug.hpp
    include/ExecutableMemory.hpp
    include/Nucleus.hpp
    include/Print.hpp
    include/Reactor.hpp
    include/ReactorDebugInfo.hpp
    include/Routine.hpp
    src/Debug.cpp
    src/EmulatedReactor.cpp
    src/ExecutableMemory.cpp
    src/Reactor.cpp
    src/ReactorDebugInfo.cpp
)

set(SUBZERO_SRC_FILES
    src/Optimizer.cpp
    src/SubzeroReactor.cpp
)

set(LLVM_SRC_FILES
    include/CPUID.hpp
    include/LLVMReactor.hpp
    include/LLVMReactorDebugInfo.hpp
    src/CPUID.cpp
    src/LLVMJIT.cpp
    src/LLVMReactor.cpp
    src/LLVMReactorDebugInfo.cpp
)

set(REACTOR_PRIVATE_COMPILE_DEFINITIONS "")
set(REACTOR_PRIVATE_LINK_LIBRARIES "")
if(REACTOR_EMIT_DEBUG_INFO)
    if(WIN32)
        # Boost stacktrace uses COM on Windows.
        # On Windows, cache COM instances in TLS for performance.
        list(APPEND REACTOR_PRIVATE_COMPILE_DEFINITIONS "BOOST_STACKTRACE_USE_WINDBG_CACHED")
    else()
        # Boost stacktrace uses libbacktrace
        list(APPEND REACTOR_PRIVATE_COMPILE_DEFINITIONS "BOOST_STACKTRACE_USE_BACKTRACE")
        list(APPEND REACTOR_PRIVATE_LINK_LIBRARIES libbacktrace)
    endif()

    list(APPEND REACTOR_PRIVATE_LINK_LIBRARIES Boost::boost)
endif(REACTOR_EMIT_DEBUG_INFO)


# SubzeroReactor library

add_library(ReactorSubzero STATIC
    ${REACTOR_SRC_FILES}
    ${SUBZERO_SRC_FILES}
)

set_target_properties(ReactorSubzero PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "Reactor"
)

target_include_directories(ReactorSubzero
    PUBLIC
        include
)

target_compile_definitions(ReactorSubzero
    PRIVATE
        ${REACTOR_PRIVATE_COMPILE_DEFINITIONS}
)

target_compile_options(ReactorSubzero
    PRIVATE
        # ${ROOT_PROJECT_COMPILE_OPTIONS}
)

target_link_libraries(ReactorSubzero
    PRIVATE
        subzero
        marl
        ${REACTOR_PRIVATE_LINK_LIBRARIES}
)


# ReactorLLVM library

add_library(ReactorLLVM STATIC
    ${REACTOR_SRC_FILES}
    ${LLVM_SRC_FILES}
)

set_target_properties(ReactorLLVM PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "Reactor"
)

target_include_directories(ReactorLLVM
    PUBLIC
        include
)

target_compile_definitions(ReactorLLVM
    PRIVATE
        ${REACTOR_PRIVATE_COMPILE_DEFINITIONS}
)

target_compile_options(ReactorLLVM
    PRIVATE
        # ${ROOT_PROJECT_COMPILE_OPTIONS}
)

target_link_libraries(ReactorLLVM
    PRIVATE
        llvm
        ${REACTOR_PRIVATE_LINK_LIBRARIES}
)
