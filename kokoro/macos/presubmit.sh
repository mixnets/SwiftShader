<<<<<<< HEAD   (7e24d3 Repoint marl branch from master to main)
=======
#!/bin/bash

set -e # Fail on any error.
set -x # Display commands being run.

BUILD_ROOT=$PWD

cd github/marl

git submodule update --init

if [ "$BUILD_SYSTEM" == "cmake" ]; then
    mkdir build
    cd build

    cmake .. -DMARL_BUILD_EXAMPLES=1 \
             -DMARL_BUILD_TESTS=1 \
             -DMARL_BUILD_BENCHMARKS=1 \
             -DMARL_WARNINGS_AS_ERRORS=1 \
             -DMARL_DEBUG_ENABLED=1

    make -j$(sysctl -n hw.logicalcpu)

    ./marl-unittests

    ./fractal
    ./primes > /dev/null
elif [ "$BUILD_SYSTEM" == "bazel" ]; then
    # Get bazel
    curl -L -k -O -s https://github.com/bazelbuild/bazel/releases/download/0.29.1/bazel-0.29.1-installer-darwin-x86_64.sh
    mkdir $BUILD_ROOT/bazel
    sh bazel-0.29.1-installer-darwin-x86_64.sh --prefix=$BUILD_ROOT/bazel
    rm bazel-0.29.1-installer-darwin-x86_64.sh
    # Build and run
    $BUILD_ROOT/bazel/bin/bazel test //:tests --test_output=all
    $BUILD_ROOT/bazel/bin/bazel run //examples:fractal
    $BUILD_ROOT/bazel/bin/bazel run //examples:primes > /dev/null
else
    echo "Unknown build system: $BUILD_SYSTEM"
    exit 1
fi
>>>>>>> BRANCH (1e6233 Squashed 'third_party/marl/' changes from 38c0c7a0f..c512711)
