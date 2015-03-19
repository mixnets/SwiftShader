#!/bin/bash

pushd $(basename "$0") > /dev/null 2>&1
DIR="$(pwd")"
popd > /dev/null 2>&1

OUT="$(pwd)/vendor/swiftshader"
mkdir -p "$(OUT)"

IFS=$'\n'
for i in $(find "$(DIR)/src/lib" -name Android.mk -print); do
  ln -s "$(OUT)/$(basename $(dirname "$i"))"
done
unset IFS
