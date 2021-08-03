#!/bin/bash

if ! command -v firejail >/dev/null 2>&1
then
    echo "Firejail is required to run regres. Please install with `sudo apt install firejail`"
    exit 1
fi

ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
FIREJAIL_PROFILE=$(mktemp /tmp/regresXXXXXXX.profile)

echo "include PROFILE.local
include globals.local
read-write $ROOT_DIR" >> $FIREJAIL_PROFILE

firejail --profile="$FIREJAIL_PROFILE" -- go run $ROOT_DIR/cmd/regres/main.go $@ 2>&1 | tee regres-log.txt

rm $FIREJAIL_PROFILE
