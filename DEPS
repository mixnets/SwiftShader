# This file is used to manage SwiftShader's dependencies in the Chromium src
# repo. It is used by gclient to determine what version of each dependency to
# check out, and where.

use_relative_paths = True

vars = {
  'chromium_git': 'https://chromium.googlesource.com/',
  # Current revision of subzero.
  'subzero_revision': '36bcf2d917adf63b67fd26dd24b2d50e92c53f4e',
}

deps = {
  'third_party/pnacl-subzero':
    Var('chromium_git') + '/native_client/pnacl-subzero@' +  Var('subzero_revision'),
}