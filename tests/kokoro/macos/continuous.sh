#!/bin/bash

# Fail on any error.
set -e
# Display commands being run.
set -x

cd git/SwiftShader

git submodule update --init

mkdir -p build && cd build

if [[ -z "${REACTOR_BACKEND}" ]]; then
  REACTOR_BACKEND="LLVM"
fi

# LESS_DEBUG_INFO is used here to work around Kokoro timeouts with debug, llvm
# builds. See b/147355576 for more information.
cmake .. "-DASAN=ON" "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" "-DREACTOR_BACKEND=${REACTOR_BACKEND}" "-DREACTOR_VERIFY_LLVM_IR=1" "-DLESS_DEBUG_INFO=1"
make -j$(sysctl -n hw.logicalcpu)

# Run unit tests

cd .. # Some tests must be run from project root

build/ReactorUnitTests
build/gles-unittests
build/vk-unittests