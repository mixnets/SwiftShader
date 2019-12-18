#!/bin/bash

set -e # Fail on any error.
set -x # Display commands being run.

# Get clang-format.
CLANG_TAR = /tmp/clang-8.tar.xz
curl http://releases.llvm.org/8.0.0/clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz -o ${CLANG_TAR}
echo "2be69be355b012ae206dbc0ea7d84b831d77dc27" ${CLANG_TAR} | sha1sum -c -

curl http://releases.llvm.org/8.0.0/clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz
sudo rm /etc/apt/sources.list.d/cuda.list*
sudo add-apt-repository "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-8 main"
curl -L -k -s https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get update
sudo apt-get install -y clang-format-8

# Run presubmit tests
./tests/presubmit.sh
