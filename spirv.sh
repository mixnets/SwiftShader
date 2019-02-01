#!/bin/bash

mkdir build
cd build
mkdir "Visual Studio 15 2017 Win64"
cd "Visual Studio 15 2017 Win64"

cmake -B"build/Visual Studio 15 2017 Win64/" -G"Visual Studio 15 2017 Win64" -Thost=x64 -DSPIRV-Headers_SOURCE_DIR="${CMAKE_HOME_DIRECTORY}/third_party/SPIRV-Headers" -DCMAKE_CONFIGURATION_TYPES="Debug;Release" -DSKIP_SPIRV_TOOLS_INSTALL=true -DSPIRV_SKIP_EXECUTABLES=true -DSPIRV_SKIP_TESTS=true ../..

cd ../..

echo Making project files path relative. This might take a second.

CD=$(pwd -W)

find . -type f \( -name \*.vcxproj -o -name \*.vcxproj.filters -o -name \*.sln \) -execdir sed -i -b -e "s?$CD?\$(SolutionDir)?g" {} \;

CD2=$(echo $(pwd -W) | sed 's?/?\\\\?g')

find . -type f \( -name \*.vcxproj -o -name \*.vcxproj.filters -o -name \*.sln \) -execdir sed -i -b -e "s?$CD2?\$(SolutionDir)?g" {} \;

# cd ../..