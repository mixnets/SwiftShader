#!/bin/bash

# Invoke this from the root of an internal Android tree. It will create
# links to build swiftshader as vendor code in vendor/swiftshader

set -o errexit

pushd $(dirname "$0") > /dev/null 2>&1
SOURCE_DIR="$(pwd)"
popd > /dev/null 2>&1

SOURCE_DIR_LINKED="$(pwd)/vendor/transgaming/swiftshader-src"
OBJECT_DIR="$(pwd)/vendor/transgaming/swiftshader"

JOBS=$(grep '^processor' /proc/cpuinfo | wc -l)

# Doing this first verifies that we're at the top of the tree

. build/envsetup.sh
rm -rf "${SOURCE_DIR_LINKED}"
mkdir -p "${SOURCE_DIR_LINKED}"
ln -s "${SOURCE_DIR}/.dir-locals.el" "${SOURCE_DIR_LINKED}"

IFS=$'\n'
for i in $(find "${SOURCE_DIR}/src" -name Android.mk -print); do
  ln -s "$(dirname "${i}" )" "${SOURCE_DIR_LINKED}"
done
unset IFS

for config in gce_x86-userdebug; do
  lunch ${config}
  . device/google/gce_x86/configure_java.sh

  TARGET_ARCH=$(get_build_var TARGET_ARCH)

  rm -rf ${OBJECT_DIR}/${TARGET_ARCH}
  make -j ${JOBS} \
     libEGL_swiftshader_vendor_debug \
     libEGL_swiftshader_vendor_release \
     libGLESv1_CM_swiftshader_vendor_debug \
     libGLESv1_CM_swiftshader_vendor_release \
     libGLESv2_swiftshader_vendor_debug \
     libGLESv2_swiftshader_vendor_release

  rm -rf ${OBJECT_DIR}/${TARGET_ARCH}/*/obj

  # JBMR2 (and earlier?) doesn't allow the library name to differ from the
  # module name. Rename the generated libraries.
  IFS=$'\n'
  for i in $(find ${OBJECT_DIR}/${TARGET_ARCH} -name '*_vendor_*'); do
    j="${i/_vendor_debug/}"
    j="${j/_vendor_release/}"
    if [[ "$i" != "$j" ]]; then
      mv "$i" "$j"
    fi
  done
  unset IFS
  pushd "${OBJECT_DIR}"
  git add ${TARGET_ARCH}
  popd
done

# Decide if we need any warning in the commit message

WARN=""
if [[ "$(cd ${SOURCE_DIR}; git diff HEAD)" != "" ]]; then
  WARN="TREE WAS NOT CLEAN"
elif [[ "$(cd ${SOURCE_DIR}; git diff origin/master)" != "" ]]; then
  WARN="TREE WAS NOT MERGED"
fi

# Do the commit

pushd "${OBJECT_DIR}"
git commit \
  -m "${WARN} SwiftShader build as of:" \
  -m "    git hash: $( cd ${SOURCE_DIR}; git log -n1 | grep commit | head -1 | awk '{ print $2; }')" \
  -m "$(cd ${SOURCE_DIR}; git log -n1 | grep '^ *Change-Id:' | tail -1 | sed 's,-, ,')" \
  -m "    At $(cd ${SOURCE_DIR}; git remote -v | grep ^origin | head -1 | awk '{ print $2; }')"
popd
