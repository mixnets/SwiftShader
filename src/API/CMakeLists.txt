# Copyright 2022 The SwiftShader Authors. All Rights Reserved.
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

set(ROOT_PROJECT_COMPILE_OPTIONS
    ${SWIFTSHADER_COMPILE_OPTIONS}
    ${WARNINGS_AS_ERRORS}
)

set(API_SRC_FILES
    libVulkan.cpp
    main.cpp
    resource.h
    Version.hpp
    VulkanPlatform.hpp
    VkGetProcAddress.cpp
    VkGetProcAddress.hpp
    VkPromotedExtensions.cpp
    VkStringify.cpp
    VkStringify.hpp
    VkStructConversion.hpp
)

if(WIN32)
    list(APPEND API_SRC_FILES
        Vulkan.rc
    )
endif()

set(API_COMPILE_OPTIONS "")
if(FUCHSIA)
    # At the moment, the Fuchsia build uses unofficial VK_STRUCTURE_TYPE_XX
    # constants that are defined as macros in <vulkan/fuchsia_extras.h>. When
    # these appear in switch() cases, the compiler complains that the values
    # are not part of the VkStructureType enum. Silence this warning, until
    # the constants are upstreamed to the official Vulkan headers.
    list(APPEND API_COMPILE_OPTIONS "-Wno-switch")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND API_COMPILE_OPTIONS
        # TODO(b/208256248): Avoid exit-time destructor.
        #"-Wexit-time-destructors"  # declaration requires an exit-time destructor
    )
endif()

set(API_LINKER_FLAGS "")
if(FUCHSIA)
    # On Fuchsia, the Vulkan ICD is loaded into a process sandbox that doesn't
    # have system libraries available, so ensure it does not depend on libc++.so.
    list(APPEND API_LINKER_FLAGS "-static-libstdc++")
endif()

add_library(vk_swiftshader SHARED
    ${API_SRC_FILES}
)

set_target_properties(vk_swiftshader PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "SwiftShader VK"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(vk_swiftshader
    PRIVATE
        ".."
)

target_compile_options(vk_swiftshader
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
        ${API_COMPILE_OPTIONS}
)

target_compile_definitions(vk_swiftshader
    PRIVATE
        "VK_EXPORT="
        $<$<CONFIG:Debug>:"DEBUGGER_WAIT_DIALOG">
)

target_link_options(vk_swiftshader
    PRIVATE
        ${SWIFTSHADER_LINK_FLAGS}
        ${API_LINKER_FLAGS}
)

target_link_libraries(vk_swiftshader
    PRIVATE
        vk_vulkan
        vk_system
        vk_pipeline
        vk_device
        vk_wsi
        Reactor
        marl
        ${OS_LIBS}
        ${SWIFTSHADER_LIBS}
        $<$<BOOL:${SWIFTSHADER_ENABLE_VULKAN_DEBUGGER}>:cppdap>
)

set_shared_library_export_map(vk_swiftshader ${CMAKE_CURRENT_SOURCE_DIR})

# The name of the application-facing library. Usually this would be the Vulkan Loader,
# which then loads the Installable Client Driver (ICD), see:
# https://www.lunarg.com/tutorial-overview-of-vulkan-loader-layers/
# But SwiftShader can also be loaded directlty by the application. This enables using
# SwiftShader without requiring a Loader to be installed, and simplifies testing and
# debugging. ${VULKAN_API_LIBRARY_NAME} is the name of this drop-in API library.
if(WIN32)
    set(VULKAN_API_LIBRARY_NAME "vulkan-1.dll")
elseif(LINUX)
    set(VULKAN_API_LIBRARY_NAME "libvulkan.so.1")
elseif(APPLE)
    set(VULKAN_API_LIBRARY_NAME "libvulkan.dylib")
elseif(FUCHSIA)
    set(VULKAN_API_LIBRARY_NAME "libvulkan.so")
else()
    message(FATAL_ERROR "Platform does not support Vulkan yet")
endif()

add_custom_command(
    TARGET vk_swiftshader
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vk_swiftshader> ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vk_swiftshader> ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/${VULKAN_API_LIBRARY_NAME}

    # Copy to ${CMAKE_BINARY_DIR}/bin so the library sits next to the PVR samples.
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vk_swiftshader> ${CMAKE_BINARY_DIR}/bin/${VULKAN_API_LIBRARY_NAME}
)

# The SWIFTSHADER_VULKAN_API_LIBRARY_INSTALL_PATH environment variable can be set to a path
# where we'd like the drop-in API library to be installed. For example for testing with dEQP
# this could be something like C:/src/deqp/build/external/vulkancts/modules/vulkan/Debug/
if(DEFINED ENV{SWIFTSHADER_VULKAN_API_LIBRARY_INSTALL_PATH})
    add_custom_command(
        TARGET vk_swiftshader
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vk_swiftshader> $ENV{SWIFTSHADER_VULKAN_API_LIBRARY_INSTALL_PATH}${VULKAN_API_LIBRARY_NAME}
    )
endif()

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
    "vk_swiftshader_icd.json.tmpl"
    "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/vk_swiftshader_icd.json"
)
