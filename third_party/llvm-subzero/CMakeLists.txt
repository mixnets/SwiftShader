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

file(GLOB_RECURSE SRC_FILES
    "*.cpp"
    "*.c"
    "*.h"
)

if(WIN32)
    set(PLATFORM_INCLUDE_DIR "build/Windows/include")
elseif(LINUX)
    set(PLATFORM_INCLUDE_DIR "build/Linux/include")
elseif(APPLE)
    set(PLATFORM_INCLUDE_DIR "build/MacOS/include")
endif()

if(WIN32)
    list(APPEND COMPILE_OPTIONS
        "/wd4141"  # 'inline': used more than once
        "/wd4146" # unary minus operator applied to unsigned type, result still unsigned
        "/wd4244"  # '=': conversion from '__int64' to 'llvm_regoff_t', possible loss of data
        "/wd4267"  # '=': conversion from 'size_t' to 'sopno', possible loss of data
        "/wd4291"  # 'void *operator new(size_t,const `anonymous-namespace'::NamedBufferAlloc &)': no matching operator delete found; memory will not be freed if initialization throws an exception
        "/wd4334" # ''operator' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
        "/wd4996" # The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: new_name.
    )
endif()

add_library(llvm-subzero STATIC EXCLUDE_FROM_ALL
  ${SRC_FILES}
)

set_target_properties(llvm-subzero PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "Subzero"
)

target_include_directories(llvm-subzero
    PUBLIC
        "include"
        ${PLATFORM_INCLUDE_DIR}
)

target_compile_options(llvm-subzero
    PUBLIC
        ${COMPILE_OPTIONS}
)
