#!/bin/bash

cd git/SwiftShader

# Validate commit message
git log -1 --pretty=%B | grep -E '^Bug:|^Issue:|^Fixes:|^Regres:'

if [ $? -ne 0 ]
then
  echo "error: Git commit message must have a Bug: line."
  exit 1
fi

# Get clang-format.
sudo rm /etc/apt/sources.list.d/cuda.list*
sudo add-apt-repository "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-8 main"
curl -L -k -s https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get update
sudo apt-get install -y clang-format-8

# Run presubmit tests
./tests/presubmit.sh