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

list(APPEND ROOT_PROJECT_COMPILE_OPTIONS
    ${SWIFTSHADER_COMPILE_OPTIONS}
    ${WARNINGS_AS_ERRORS}
)

# list(APPEND ROOT_PROJECT_LINK_LIBRARIES
#     ${OS_LIBS}
# )

set(REACTOR_SRC_FILES
    Debug.cpp
    Debug.hpp
    EmulatedReactor.cpp
    ExecutableMemory.cpp
    ExecutableMemory.hpp
    Nucleus.hpp
    Print.hpp
    Reactor.cpp
    Reactor.hpp
    ReactorDebugInfo.cpp
    ReactorDebugInfo.hpp
    Routine.hpp
)

set(SUBZERO_SRC_FILES
    Optimizer.cpp
    SubzeroReactor.cpp
)

set(LLVM_SRC_FILES
    CPUID.cpp
    CPUID.hpp
    LLVMJIT.cpp
    LLVMReactor.cpp
    LLVMReactor.hpp
    LLVMReactorDebugInfo.cpp
    LLVMReactorDebugInfo.hpp
)

# SubzeroReactor library

add_library(ReactorSubzero STATIC EXCLUDE_FROM_ALL
    ${REACTOR_SRC_FILES}
    ${SUBZERO_SRC_FILES}
)

set_target_properties(ReactorSubzero PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "Reactor"
)

target_include_directories(ReactorSubzero
    PUBLIC
        .
)

target_compile_options(ReactorSubzero
    PUBLIC
        ${ROOT_PROJECT_COMPILE_OPTIONS}
)

target_link_libraries(ReactorSubzero
    PRIVATE
        subzero
        marl
)


# ReactorLLVM library

add_library(ReactorLLVM STATIC EXCLUDE_FROM_ALL
    ${REACTOR_SRC_FILES}
    ${LLVM_SRC_FILES}
)

set_target_properties(ReactorLLVM PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "Reactor"
)

target_include_directories(ReactorLLVM
    PUBLIC
        .
)

target_compile_options(ReactorLLVM
    PUBLIC
        ${ROOT_PROJECT_COMPILE_OPTIONS}
)

target_link_libraries(ReactorLLVM
    PRIVATE
        llvm
        ${ROOT_PROJECT_LINK_LIBRARIES}
)

if(REACTOR_EMIT_DEBUG_INFO)
    # Use boost::stacktrace to get stack trace info
    if(NOT WIN32)
        # Boost stacktrace uses libbacktrace
        add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/libbacktrace third_party/libbacktrace)
    endif()
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/boost third_party/boost)

    function(reactor_add_stacktrace_deps target)
        if(WIN32)
            # Boost stacktrace uses COM on Windows.
            # On Windows, cache COM instances in TLS for performance.
            target_compile_definitions(${target} PRIVATE BOOST_STACKTRACE_USE_WINDBG_CACHED)
        else()
            target_compile_definitions(${target} PRIVATE BOOST_STACKTRACE_USE_BACKTRACE)
            target_link_libraries(${target} libbacktrace)
        endif()

        target_link_libraries(${target} PRIVATE Boost::boost)
    endfunction()

    reactor_add_stacktrace_deps(ReactorSubzero)
    reactor_add_stacktrace_deps(ReactorLLVM)

endif(REACTOR_EMIT_DEBUG_INFO)
