#!/bin/bash

# llvm now lives in a mono-repo along with clang, libc, lldb and a whole bunch
# of other projects (~90k files at time of writing).
# SwiftShader only requires the llvm project from this repo, and does not wish
# to pull in everything else.
# This script performs a sparse checkout of the llvm project, and copies it to
# the third_party directory, replacing all the files. If there are any local
# modifications made to the directory in third_party, these are re-applied as a
# separate change.

THIRD_PARTY_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
STAGING_DIR="/tmp/llvm-10-update"
SOURCE_DIR="${STAGING_DIR}/llvm"
TARGET_DIR="${THIRD_PARTY_DIR}/llvm-10.0/llvm"
BRANCH="release/10.x"

# Clone or update the staging directory
if [[ -d "$STAGING_DIR" ]]; then
  pushd "$STAGING_DIR"
else
  mkdir "$STAGING_DIR"
  pushd "$STAGING_DIR"
  git init
  git remote add origin https://github.com/llvm/llvm-project.git
  git config core.sparsecheckout true
  echo "/llvm/lib" >> .git/info/sparse-checkout
  echo "/llvm/include" >> .git/info/sparse-checkout
fi
git pull origin $BRANCH --depth=1
LLVM_LATEST=`git log HEAD -n 1 --pretty=format:'%h'`
popd

if [[ -d "$TARGET_DIR" ]]; then
  # Look for the last update change.
  LAST_TARGET_UPDATE=`git log --grep="^llvm-10-update: [0-9a-f]\{9\}$" -n 1 --pretty=format:'%h' ${TARGET_DIR}`
  if [[ ! -z "$LAST_TARGET_UPDATE" ]]; then
    # Get the LLVM commit hash from the update change.
    LAST_SOURCE_UPDATE=`git log $LAST_TARGET_UPDATE -n 1 | grep -oP "llvm-10-update: \K([0-9a-f]{9})"`
    if [ $LLVM_LATEST == $LAST_SOURCE_UPDATE ]; then
      echo "No new LLVM changes to apply"
      exit 0
    fi

    # Gather list of changes since last update
    pushd "$STAGING_DIR"
    LLVM_CHANGE_LOG=`git log $LAST_SOURCE_UPDATE..$LLVM_LATEST --pretty=format:'  %h %s'`
    LLVM_CHANGE_LOG="Changes:\n${LLVM_CHANGE_LOG}\n\n"
    popd

    # Gather local file changes made since last update.
    LOCAL_DIFFS=`git diff ${LAST_TARGET_UPDATE} HEAD ${TARGET_DIR}`
  fi

  # Delete the target directory. We're going to re-populate it.
  rm -fr "$TARGET_DIR"
fi

# Update SwiftShader's $TARGET_DIR with a clean copy of latest LLVM
mkdir -p "$TARGET_DIR"
cp -r "$SOURCE_DIR/." "$TARGET_DIR"
git add "$TARGET_DIR"
COMMIT_MSG=`echo -e "Update LLVM 10 to ${LLVM_LATEST}\n\nNote: This change reverts any local modifications.\n\n${LLVM_CHANGE_LOG}Commands:\n  third_party/update-llvm-10.sh\n\nllvm-10-update: ${LLVM_LATEST}\nBug: b/152339534"`
git commit -m "$COMMIT_MSG"

# We're now done with the staging dir. Delete it.
rm -fr "$STAGING_DIR"

# Re-apply any local diffs.
if [[ ! -z "$LOCAL_DIFFS" ]]; then
  echo "Applying local diffs"
  echo "$LOCAL_DIFFS" | git apply
  if [ $? -eq 0 ]; then
      git add "$TARGET_DIR"
      COMMIT_MSG=`echo -e "Re-applying local patches to LLVM 10\n\nBug: b/152339534"`
      git commit -m "$COMMIT_MSG"
  else
      echo "Failed to re-apply local patches to $TARGET_DIR.\nPlease fix the merge conflicts and commit."
  fi
fi
