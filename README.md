<<<<<<< HEAD   (782a98 Temporarily disable warnings-as-errors)
# SwiftShader

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Build Status](https://travis-ci.org/google/swiftshader.svg?branch=master)](https://travis-ci.org/google/swiftshader) [![Build status](https://ci.appveyor.com/api/projects/status/yrmyvb34j22jg1uj?svg=true)](https://ci.appveyor.com/project/c0d1f1ed/swiftshader)

Introduction
------------

SwiftShader is a high-performance CPU-based implementation of the OpenGL ES and Direct3D 9 graphics APIs<sup>1</sup><sup>2</sup>. Its goal is to provide hardware independence for advanced 3D graphics.

Building
--------

SwiftShader libraries can be built for Windows, Linux, and Mac OS X.\
Android and Chrome (OS) build environments are also supported.

* **Visual Studio**
\
  On Windows, open the [SwiftShader.sln](SwiftShader.sln) file using [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) or compatible version, and build the solution. Output DLLs will be placed in the _out_ subfolder. Sample executables such as _OGLES3ColourGrading_ can be found under the Tests solution folder and can be run from the IDE.

* **CMake**

  [Install CMake](https://cmake.org/download/) for Linux, Mac OS X, or Windows and use either [the IDE](https://cmake.org/runningcmake/) or run the following terminal commands:

      cd build
      cmake ..
      make --jobs=8

      ./gles-unittests
      ./OGLES2HelloAPI

Usage
-----

The SwiftShader libraries act as drop-in replacements for graphics drivers.

On Windows, most applications can be made to use SwiftShader's DLLs by placing them in the same folder as the executable. On Linux, the LD\_LIBRARY\_PATH environment variable or -rpath linker option can be used to direct applications to search for shared libraries in the indicated directory first.

Contributing
------------

See [CONTRIBUTING.txt](CONTRIBUTING.txt) for important contributing requirements.

The canonical repository for SwiftShader is hosted at:
https://swiftshader.googlesource.com/SwiftShader

All changes must be reviewed and approved in the [Gerrit](https://www.gerritcodereview.com/) review tool at:
https://swiftshader-review.googlesource.com

Authenticate your account here:
https://swiftshader-review.googlesource.com/new-password

All changes require a [Change-ID](https://gerrit-review.googlesource.com/Documentation/user-changeid.html) tag in the commit message. A commit hook may be used to add this tag automatically, and can be found at:
https://gerrit-review.googlesource.com/tools/hooks/commit-msg. To clone the repository and install the commit hook in one go:

    git clone https://swiftshader.googlesource.com/SwiftShader && (cd SwiftShader && curl -Lo `git rev-parse --git-dir`/hooks/commit-msg https://gerrit-review.googlesource.com/tools/hooks/commit-msg ; chmod +x `git rev-parse --git-dir`/hooks/commit-msg)

Changes are uploaded to Gerrit by executing:

    git push origin HEAD:refs/for/master

Testing
-------

SwiftShader's OpenGL ES implementation can be tested using the [dEQP](https://source.android.com/devices/graphics/testing) test suite.

See [docs/dEQP.md](docs/dEQP.md) for details.

Third-Party Dependencies
------------------------

The [third_party](third_party/) directory contains projects which originated outside of SwiftShader:

[subzero](third_party/subzero/) contains a fork of the [Subzero](https://chromium.googlesource.com/native_client/pnacl-subzero/) project. It is part of Google Chrome's (Portable) [Native Client](https://developer.chrome.com/native-client) project. Its authoritative source is at [https://chromium.googlesource.com/native_client/pnacl-subzero/](https://chromium.googlesource.com/native_client/pnacl-subzero/). The fork was made using [git-subtree](https://github.com/git/git/blob/master/contrib/subtree/git-subtree.txt) to include all of Subzero's history, and until further notice it should **not** diverge from the upstream project. Contributions must be tested using the [README](third_party/subzero/docs/README.rst) instructions, reviewed at [https://chromium-review.googlesource.com](https://chromium-review.googlesource.com/q/project:native_client%252Fpnacl-subzero), and then pulled into the SwiftShader repository.

[llvm-subzero](third_party/llvm-subzero/) contains a minimized set of LLVM dependencies of the Subzero project.

[PowerVR_SDK](third_party/PowerVR_SDK/) contains a subset of the [PowerVR Graphics Native SDK](https://github.com/powervr-graphics/Native_SDK) for running several sample applications.

[googletest](third_party/googletest/) contains the [Google Test](https://github.com/google/googletest) project, as a Git submodule. It is used for running unit tests for Chromium, and Reactor unit tests. Run `git submodule update --init` to obtain/update the code. Any contributions should be made upstream.

Documentation
-------------

See [docs/Index.md](docs/Index.md).

Contact
-------

Public mailing list: [swiftshader@googlegroups.com](https://groups.google.com/forum/#!forum/swiftshader)

General bug tracker:  https://g.co/swiftshaderbugs\
Chrome specific bugs: https://bugs.chromium.org/p/swiftshader

License
-------

The SwiftShader project is licensed under the Apache License Version 2.0. You can find a copy of it in [LICENSE.txt](LICENSE.txt).

Files in the third_party folder are subject to their respective license.

Authors and Contributors
------------------------

The legal authors for copyright purposes are listed in [AUTHORS.txt](AUTHORS.txt).

[CONTRIBUTORS.txt](CONTRIBUTORS.txt) contains a list of names of individuals who have contributed to SwiftShader. If you're not on the list, but you've signed the [Google CLA](https://cla.developers.google.com/clas) and have contributed more than a formatting change, feel free to request to be added.

Disclaimer
----------

1. Trademarks are the property of their respective owners.
2. We do not claim official conformance with any graphics APIs at this moment.
3. This is not an official Google product.
=======
# SPIR-V Headers

This repository contains machine-readable files for the
[SPIR-V Registry](https://www.khronos.org/registry/spir-v/).
This includes:

* Header files for various languages.
* JSON files describing the grammar for the SPIR-V core instruction set
  and the extended instruction sets.
* The XML registry file.
* A tool to build the headers from the JSON grammar.

Headers are provided in the [include](include) directory, with up-to-date
headers in the `unified1` subdirectory. Older headers are provided according to
their version.

In contrast, the XML registry file has a linear history, so it is
not tied to SPIR-V specification versions.

## How is this repository updated?

When a new version or revision of the SPIR-V specification is published,
the SPIR-V Working Group will push new commits onto master, updating
the files under [include](include).

The SPIR-V XML registry file is updated by Khronos whenever a new enum range is allocated.

Pull requests can be made to
- request allocation of new enum ranges in the XML registry file
- reserve specific tokens in the JSON grammar

### Reserving tokens in the JSON grammar

Care should be taken to follow existing precedent in populating the details of reserved tokens. This includes:
- pointing to what extension has more information, when possible
- keeping enumerants in numeric order
- when there are aliases, listing the preferred spelling first
- adding the statement `"version" : "None"`

## How to install the headers

```
mkdir build
cd build
cmake ..
cmake --build . --target install
```

Then, for example, you will have `/usr/local/include/spirv/unified1/spirv.h`

If you want to install them somewhere else, then use
`-DCMAKE_INSTALL_PREFIX=/other/path` on the first `cmake` command.

## Using the headers without installing

A CMake-based project can use the headers without installing, as follows:

1. Add an `add_subdirectory` directive to include this source tree.
2. Use `${SPIRV-Headers_SOURCE_DIR}/include}` in a `target_include_directories`
   directive.
3. In your C or C++ source code use `#include` directives that explicitly mention
   the `spirv` path component.
```
#include "spirv/unified1/GLSL.std.450.h"
#include "spirv/unified1/OpenCL.std.h"
#include "spirv/unified1/spirv.hpp"
```

See also the [example](example/) subdirectory.  But since that example is
*inside* this repostory, it doesn't use and `add_subdirectory` directive.

## Generating the headers from the JSON grammar

This will generally be done by Khronos, for a change to the JSON grammar.
However, the project for the tool to do this is included in this repository,
and can be used to test a PR, or even to include the results in the PR.
This is not required though.

The header-generation project is under the `tools/buildHeaders` directory.
Use CMake to build the project, in a `build` subdirectory (under `tools/buildHeaders`).
There is then a bash script at `bin/makeHeaders` that shows how to use the built
header-generator binary to generate the headers from the JSON grammar.
(Execute `bin/makeHeaders` from the `tools/buildHeaders` directory.)

Notes:
- this generator is used in a broader context within Khronos to generate the specification,
  and that influences the languages used, for legacy reasons
- the C++ structures built may similarly include more than strictly necessary, for the same reason

## FAQ

* *How are different versions published?*

  The multiple versions of the headers have been simplified into a
  single `unified1` view. The JSON grammar has a "version" field saying
  what version things first showed up in.

* *How do you handle the evolution of extended instruction sets?*

  Extended instruction sets evolve asynchronously from the core spec.
  Right now there is only a single version of both the GLSL and OpenCL
  headers.  So we don't yet have a problematic example to resolve.

## License
<a name="license"></a>
```
Copyright (c) 2015-2018 The Khronos Group Inc.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and/or associated documentation files (the
"Materials"), to deal in the Materials without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Materials, and to
permit persons to whom the Materials are furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Materials.

MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
   https://www.khronos.org/registry/

THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
```
>>>>>>> BRANCH (a890b1 Squashed 'third_party/SPIRV-Headers/' changes from 79b6681aa)
