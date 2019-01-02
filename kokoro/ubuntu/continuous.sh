#!/bin/bash

# Fail on any error.
set -e
# Display commands being run.
set -x

cd git/SwiftShader

mkdir -p build && cd build

cmake ..
make --jobs=16
