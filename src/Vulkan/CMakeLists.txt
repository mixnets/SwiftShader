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

set(ROOT_PROJECT_COMPILE_OPTIONS
    ${SWIFTSHADER_COMPILE_OPTIONS}
    ${WARNINGS_AS_ERRORS}
)

set(VULKAN_SRC_FILES
    VkBuffer.cpp
    VkBuffer.hpp
    VkBufferView.cpp
    VkBufferView.hpp
    VkCommandBuffer.cpp
    VkCommandBuffer.hpp
    VkCommandPool.cpp
    VkCommandPool.hpp
    VkConfig.hpp
    VkDebugUtilsMessenger.cpp
    VkDebugUtilsMessenger.hpp
    VkDescriptorPool.cpp
    VkDescriptorPool.hpp
    VkDescriptorSet.cpp
    VkDescriptorSet.hpp
    VkDescriptorSetLayout.cpp
    VkDescriptorSetLayout.hpp
    VkDescriptorUpdateTemplate.cpp
    VkDescriptorUpdateTemplate.hpp
    VkDestroy.hpp
    VkDevice.cpp
    VkDevice.hpp
    VkDeviceMemory.cpp
    VkDeviceMemory.hpp
    VkDeviceMemoryExternalHost.cpp
    VkDeviceMemoryExternalHost.hpp
    VkDeviceMemoryExternalLinux.hpp
    VkEvent.hpp
    VkFence.hpp
    VkFormat.cpp
    VkFormat.hpp
    VkFramebuffer.cpp
    VkFramebuffer.hpp
    VkImage.cpp
    VkImage.hpp
    VkImageView.cpp
    VkImageView.hpp
    VkInstance.cpp
    VkInstance.hpp
    VkMemory.cpp
    VkMemory.hpp
    VkObject.hpp
    VkPhysicalDevice.cpp
    VkPhysicalDevice.hpp
    VkPipeline.cpp
    VkPipeline.hpp
    VkPipelineCache.cpp
    VkPipelineCache.hpp
    VkPrivateData.hpp
    VkSpecializationInfo.cpp
    VkSpecializationInfo.hpp
    VkPipelineLayout.cpp
    VkPipelineLayout.hpp
    VkQueryPool.cpp
    VkQueryPool.hpp
    VkQueue.cpp
    VkQueue.hpp
    VkRenderPass.cpp
    VkRenderPass.hpp
    VkSampler.cpp
    VkSampler.hpp
    VkSemaphore.cpp
    VkSemaphore.hpp
    VkSemaphoreExternalFuchsia.hpp
    VkSemaphoreExternalLinux.hpp
    VkShaderModule.cpp
    VkShaderModule.hpp
    VkTimelineSemaphore.cpp
    VkTimelineSemaphore.hpp
)

if(SWIFTSHADER_ENABLE_VULKAN_DEBUGGER)
    list(APPEND VULKAN_SRC_FILES
        Debug/Context.cpp
        Debug/Context.hpp
        Debug/Debug.cpp
        Debug/EventListener.cpp
        Debug/EventListener.hpp
        Debug/File.cpp
        Debug/File.hpp
        Debug/ID.hpp
        Debug/Location.hpp
        Debug/Server.cpp
        Debug/Server.hpp
        Debug/Thread.cpp
        Debug/Thread.hpp
        Debug/TypeOf.cpp
        Debug/TypeOf.hpp
        Debug/Value.cpp
        Debug/Value.hpp
        Debug/Variable.cpp
        Debug/Variable.hpp
        Debug/WeakMap.hpp
    )
endif()

set(VULKAN_COMPILE_OPTIONS "")
if(FUCHSIA)
    # At the moment, the Fuchsia build uses unofficial VK_STRUCTURE_TYPE_XX
    # constants that are defined as macros in <vulkan/fuchsia_extras.h>. When
    # these appear in switch() cases, the compiler complains that the values
    # are not part of the VkStructureType enum. Silence this warning, until
    # the constants are upstreamed to the official Vulkan headers.
    list(APPEND VULKAN_COMPILE_OPTIONS "-Wno-switch")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND VULKAN_COMPILE_OPTIONS
        # TODO(b/208256248): Avoid exit-time destructor.
        #"-Wexit-time-destructors"  # declaration requires an exit-time destructor
    )
endif()

set(VULKAN_LINKER_FLAGS "")
if(FUCHSIA)
    # On Fuchsia, the Vulkan ICD is loaded into a process sandbox that doesn't
    # have system libraries available, so ensure it does not depend on libc++.so.
    list(APPEND VULKAN_LINKER_FLAGS "-static-libstdc++")
endif()

add_library(vk_vulkan EXCLUDE_FROM_ALL
    ${VULKAN_SRC_FILES}
)

set_target_properties(vk_vulkan PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "SwiftShader VK"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(vk_vulkan
    PRIVATE
        ".."
)

target_compile_options(vk_vulkan
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
        ${VULKAN_COMPILE_OPTIONS}
)

target_compile_definitions(vk_vulkan
    PRIVATE
        $<$<CONFIG:Debug>:"DEBUGGER_WAIT_DIALOG">
)

target_link_options(vk_vulkan
    PRIVATE
        ${SWIFTSHADER_LINK_FLAGS}
        ${VULKAN_LINKER_FLAGS}
)

target_link_libraries(vk_vulkan
    PRIVATE
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
