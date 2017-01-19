# This file is used to manage the SwiftShader's dependencies in the Chromium src
# repo. It is used by gclient to determine what version of each dependency to
# check out, and where.

use_relative_paths = True

vars = {
  'chromium_git': 'https://chromium.googlesource.com/',
  # Current revision of subzero.
  'subzero_revision': '5c4d677c58b0bb1fc22c8515c4141c8b77cf6bf1',
}

deps = {
  'third_party/pnacl-subzero':
    Var('chromium_git') + '/native_client/pnacl-subzero@' +  Var('subzero_revision'),
}