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


set(VULKAN_UNIT_TESTS_COMMON_SRC_FILES
    Device.hpp
    Driver.hpp
    VkGlobalFuncs.hpp
    VkInstanceFuncs.hpp
    Device.cpp
    Driver.cpp
)

add_library(vk-unittests-common
    ${VULKAN_UNIT_TESTS_COMMON_SRC_FILES}
)

# vk-unittests-common loads vk_swiftshader, so make sure it gets built first
add_dependencies(vk-unittests-common
    vk_swiftshader
)

set_target_properties(vk-unittests-common PROPERTIES
    FOLDER "Tests"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(vk-unittests-common
    PRIVATE
        "${SWIFTSHADER_DIR}/include"
)

target_compile_definitions(vk-unittests-common
    PRIVATE
        "STANDALONE"
)

target_compile_options(vk-unittests-common
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
)

target_link_options(vk-unittests-common
    PRIVATE
        ${SWIFTSHADER_LINK_FLAGS}
)

target_link_libraries(vk-unittests-common
    PRIVATE
        gtest
        gmock
        SPIRV-Tools
        ${ROOT_PROJECT_LINK_LIBRARIES}
)
