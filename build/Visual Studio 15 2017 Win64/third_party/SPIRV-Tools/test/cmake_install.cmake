# Install script for directory: C:/Users/Nicolas/Development/SwiftShader/third_party/SPIRV-Tools/test

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/SwiftShader")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/Nicolas/Development/SwiftShader/build/Visual Studio 15 2017 Win64/third_party/SPIRV-Tools/test/comp/cmake_install.cmake")
  include("C:/Users/Nicolas/Development/SwiftShader/build/Visual Studio 15 2017 Win64/third_party/SPIRV-Tools/test/link/cmake_install.cmake")
  include("C:/Users/Nicolas/Development/SwiftShader/build/Visual Studio 15 2017 Win64/third_party/SPIRV-Tools/test/opt/cmake_install.cmake")
  include("C:/Users/Nicolas/Development/SwiftShader/build/Visual Studio 15 2017 Win64/third_party/SPIRV-Tools/test/reduce/cmake_install.cmake")
  include("C:/Users/Nicolas/Development/SwiftShader/build/Visual Studio 15 2017 Win64/third_party/SPIRV-Tools/test/stats/cmake_install.cmake")
  include("C:/Users/Nicolas/Development/SwiftShader/build/Visual Studio 15 2017 Win64/third_party/SPIRV-Tools/test/tools/cmake_install.cmake")
  include("C:/Users/Nicolas/Development/SwiftShader/build/Visual Studio 15 2017 Win64/third_party/SPIRV-Tools/test/util/cmake_install.cmake")
  include("C:/Users/Nicolas/Development/SwiftShader/build/Visual Studio 15 2017 Win64/third_party/SPIRV-Tools/test/val/cmake_install.cmake")

endif()

