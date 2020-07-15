#!/bin/bash -e

KYTHE_VERSION=v0.0.45

# Obtain the Kythe package
wget -q -O kythe.tar.gz \
    https://github.com/kythe/kythe/releases/download/$KYTHE_VERSION/kythe-$KYTHE_VERSION.tar.gz
tar -xzf kythe.tar.gz
KYTHE="${PWD}/kythe-${KYTHE_VERSION}"

# Must be run from the SwiftShader checkout directory.
export KYTHE_ROOT_DIRECTORY="${PWD}"
export KYTHE_CORPUS="swiftshader.googlesource.com/SwiftShader"

export KYTHE_OUTPUT_DIRECTORY="${PWD}/kythe-output"
mkdir -p "${KYTHE_OUTPUT_DIRECTORY}"

# $CMAKE_ROOT_DIRECTORY is passed into the -sourcedir flag. This value should be
# the directory that contains the top-level CMakeLists.txt file. In many
# repositories this path is the same as $KYTHE_ROOT_DIRECTORY.
export CMAKE_ROOT_DIRECTORY="${KYTHE_ROOT_DIRECTORY}"

# Run Kythe's extractor tool which will invoke CMake to create an instrumented
# build from wich cross-referencing data is produced.
${KYTHE}/tools/runextractor cmake \
    -extractor=${KYTHE}/extractors/cxx_extractor \
    -sourcedir=${CMAKE_ROOT_DIRECTORY}