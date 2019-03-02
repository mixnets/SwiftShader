#!/bin/bash

# Fail on any error.
set -e

# Display commands being run.
set -x

cd git/SwiftShader

git submodule update --init

# Check that all Visual Studio project files are up to date
./check_vcxproj_files.sh

# Build the source
mkdir -p build && cd build
cmake ..
make --jobs=$(nproc) VERBOSE=1

# Run the GLES unit tests.
./unittests