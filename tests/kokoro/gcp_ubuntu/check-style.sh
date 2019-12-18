#!/bin/bash

set -x # Display commands being run.

# Download clang tar, verify.
CLANG_TAR=/tmp/clang-8.tar.xz
curl http://releases.llvm.org/8.0.0/clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz > ${CLANG_TAR}
echo "2be69be355b012ae206dbc0ea7d84b831d77dc27" ${CLANG_TAR} | sha1sum -c -
if [ $? -ne 0 ]
then
  echo "clang download's sha was not as expected"
  return 1
fi

set -e # Fail on any error.

CLANG_DIR=/tmp/clang-8
mkdir ${CLANG_DIR}
tar -xf ${CLANG_TAR} -C ${CLANG_DIR}

export CLANG_FORMAT=${CLANG_DIR}/clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-16.04/bin/clang-format

# Run presubmit tests
./tests/presubmit.sh
