<<<<<<< HEAD   (e14609 Update SPIR-V Headers to 7845730ca)
# SwiftShader

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Introduction
------------

SwiftShader is a high-performance CPU-based implementation of the Vulkan graphics APIs<sup>1</sup><sup>2</sup>. Its goal is to provide hardware independence for advanced 3D graphics.

> NOTE: SwiftShader's OpenGL ES frontend is no longer supported, and will eventually be removed. Read more about our recommendation to use [ANGLE on top of SwiftShader Vulkan here](ANGLE.md).

Building
--------

SwiftShader libraries can be built for Windows, Linux, and Mac OS X.\
Android and Chrome (OS) build environments are also supported.

* **CMake**
\
  [Install CMake](https://cmake.org/download/) for Linux, Mac OS X, or Windows and use either [the GUI](https://cmake.org/runningcmake/) or run the following terminal commands:
  ```
  cd build
  cmake ..
  cmake --build . --parallel

  ./vk-unittests
  ```

* **Visual Studio**
\
  To build the Vulkan ICD library, use [Visual Studio 2019](https://visualstudio.microsoft.com/vs/community/) to open the project folder and wait for it to run CMake. Open the [CMake Targets View](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019#ide-integration) in the Solution Explorer and select the vk_swiftshader project to [build](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019#building-cmake-projects) it.


Usage
-----

The SwiftShader libraries act as drop-in replacements for graphics drivers.

On Windows, most applications can be made to use SwiftShader's DLLs by placing them in the same folder as the executable. On Linux, the `LD_LIBRARY_PATH` environment variable or `-rpath` linker option can be used to direct applications to search for shared libraries in the indicated directory first.

In general, Vulkan applications look for a shared library named `vulkan-1.dll` on Windows (`vulkan-1.so` on Linux). SwiftShader's equivalent is named `libvk_swiftshader.dll`, which can be renamed to `vulkan-1.dll`. Alternatively, you can set the `VK_ICD_FILENAMES` environment variable to the path to `vk_swiftshader_icd.json` file that is generated under the build directory (e.g. `.\SwiftShader\build\Windows\vk_swiftshader_icd.json`). To learn more about how Vulkan loading works, read the [official documentation here](https://github.com/KhronosGroup/Vulkan-Loader/blob/master/loader/LoaderAndLayerInterface.md).

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

When ready, [add](https://gerrit-review.googlesource.com/Documentation/intro-user.html#adding-reviewers) a project [owner](OWNERS) as a reviewer on your change.

Some tests will automatically be run against the change. Notably, [presubmit.sh](tests/presubmit.sh) verifies the change has been formatted using [clang-format 10.0](tests/kokoro/gcp_ubuntu/check_style.sh). Most IDEs come with clang-format support, but may require downgrading to [clang-format version 10.0](https://github.com/llvm/llvm-project/releases/tag/llvmorg-10.0.0).

Testing
-------

SwiftShader's OpenGL ES implementation can be tested using the [dEQP](https://github.com/KhronosGroup/VK-GL-CTS) test suite.

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
2. This is not an official Google product.
=======
# SPIR-V Tools

## Overview

The SPIR-V Tools project provides an API and commands for processing SPIR-V
modules.

The project includes an assembler, binary module parser, disassembler,
validator, and optimizer for SPIR-V. Except for the optimizer, all are based
on a common static library.  The library contains all of the implementation
details, and is used in the standalone tools whilst also enabling integration
into other code bases directly. The optimizer implementation resides in its
own library, which depends on the core library.

The interfaces have stabilized:
We don't anticipate making a breaking change for existing features.

SPIR-V is defined by the Khronos Group Inc.
See the [SPIR-V Registry][spirv-registry] for the SPIR-V specification,
headers, and XML registry.

## Downloads

[![Build status](https://ci.appveyor.com/api/projects/status/gpue87cesrx3pi0d/branch/master?svg=true)](https://ci.appveyor.com/project/Khronoswebmaster/spirv-tools/branch/master)
<img alt="Linux" src="kokoro/img/linux.png" width="20px" height="20px" hspace="2px"/>[![Linux Build Status](https://storage.googleapis.com/spirv-tools/badges/build_status_linux_clang_release.svg)](https://storage.googleapis.com/spirv-tools/badges/build_link_linux_clang_release.html)
<img alt="MacOS" src="kokoro/img/macos.png" width="20px" height="20px" hspace="2px"/>[![MacOS Build Status](https://storage.googleapis.com/spirv-tools/badges/build_status_macos_clang_release.svg)](https://storage.googleapis.com/spirv-tools/badges/build_link_macos_clang_release.html)
<img alt="Windows" src="kokoro/img/windows.png" width="20px" height="20px" hspace="2px"/>[![Windows Build Status](https://storage.googleapis.com/spirv-tools/badges/build_status_windows_release.svg)](https://storage.googleapis.com/spirv-tools/badges/build_link_windows_vs2017_release.html)

[More downloads](docs/downloads.md)

## Versioning SPIRV-Tools

See [`CHANGES`](CHANGES) for a high level summary of recent changes, by version.

SPIRV-Tools project version numbers are of the form `v`*year*`.`*index* and with
an optional `-dev` suffix to indicate work in progress.  For example, the
following versions are ordered from oldest to newest:

* `v2016.0`
* `v2016.1-dev`
* `v2016.1`
* `v2016.2-dev`
* `v2016.2`

Use the `--version` option on each command line tool to see the software
version.  An API call reports the software version as a C-style string.

## Supported features

### Assembler, binary parser, and disassembler

* Support for SPIR-V 1.0, through 1.5
  * Based on SPIR-V syntax described by JSON grammar files in the
    [SPIRV-Headers](https://github.com/KhronosGroup/SPIRV-Headers) repository.
  * Usually, support for a new version of SPIR-V is ready within days after
    publication.
* Support for extended instruction sets:
  * GLSL std450 version 1.0 Rev 3
  * OpenCL version 1.0 Rev 2
* Assembler only does basic syntax checking.  No cross validation of
  IDs or types is performed, except to check literal arguments to
  `OpConstant`, `OpSpecConstant`, and `OpSwitch`.

See [`docs/syntax.md`](docs/syntax.md) for the assembly language syntax.

### Validator

The validator checks validation rules described by the SPIR-V specification.

Khronos recommends that tools that create or transform SPIR-V modules use the
validator to ensure their outputs are valid, and that tools that consume SPIR-V
modules optionally use the validator to protect themselves from bad inputs.
This is especially encouraged for debug and development scenarios.

The validator has one-sided error: it will only return an error when it has
implemented a rule check and the module violates that rule.

The validator is incomplete.
See the [CHANGES](CHANGES) file for reports on completed work, and
the [Validator
sub-project](https://github.com/KhronosGroup/SPIRV-Tools/projects/1) for planned
and in-progress work.

*Note*: The validator checks some Universal Limits, from section 2.17 of the SPIR-V spec.
The validator will fail on a module that exceeds those minimum upper bound limits.
It is [future work](https://github.com/KhronosGroup/SPIRV-Tools/projects/1#card-1052403)
to parameterize the validator to allow larger
limits accepted by a more than minimally capable SPIR-V consumer.


### Optimizer

The optimizer is a collection of code transforms, or "passes".
Transforms are written for a diverse set of reasons:

* To restructure, simplify, or normalize the code for further processing.
* To eliminate undesirable code.
* To improve code quality in some metric such as size or performance.
  **Note**: These transforms are not guaranteed to actually improve any
  given metric. Users should always measure results for their own situation.

As of this writing, there are 67 transforms including examples such as:
* Simplification
  * Strip debug info
  * Strip reflection info
* Specialization Constants
  * Set spec constant default value
  * Freeze spec constant to default value
  * Fold `OpSpecConstantOp` and `OpSpecConstantComposite`
  * Unify constants
  * Eliminate dead constant
* Code Reduction
  * Inline all function calls exhaustively
  * Convert local access chains to inserts/extracts
  * Eliminate local load/store in single block
  * Eliminate local load/store with single store
  * Eliminate local load/store with multiple stores
  * Eliminate local extract from insert
  * Eliminate dead instructions (aggressive)
  * Eliminate dead branches
  * Merge single successor / single predecessor block pairs
  * Eliminate common uniform loads
  * Remove duplicates: Capabilities, extended instruction imports, types, and
    decorations.
* Normalization
  * Compact IDs
  * CFG cleanup
  * Flatten decorations
  * Merge returns
  * Convert AMD-specific instructions to KHR instructions
* Code improvement
  * Conditional constant propagation
  * If-conversion
  * Loop fission
  * Loop fusion
  * Loop-invariant code motion
  * Loop unroll
* Other
  * Generate WebGPU initializers
  * Graphics robust access
  * Upgrade memory model to VulkanKHR

Additionally, certain sets of transformations have been packaged into
higher-level recipes.  These include:

* Optimization for size (`spirv-opt -Os`)
* Optimization for performance (`spirv-opt -O`)

For the latest list with detailed documentation, please refer to
[`include/spirv-tools/optimizer.hpp`](include/spirv-tools/optimizer.hpp).

For suggestions on using the code reduction options, please refer to this [white paper](https://www.lunarg.com/shader-compiler-technologies/white-paper-spirv-opt/).


### Linker

*Note:* The linker is still under development.

Current features:
* Combine multiple SPIR-V binary modules together.
* Combine into a library (exports are retained) or an executable (no symbols
  are exported).

See the [CHANGES](CHANGES) file for reports on completed work, and the [General
sub-project](https://github.com/KhronosGroup/SPIRV-Tools/projects/2) for
planned and in-progress work.


### Reducer

*Note:* The reducer is still under development.

The reducer simplifies and shrinks a SPIR-V module with respect to a
user-supplied *interestingness function*.  For example, given a large
SPIR-V module that cause some SPIR-V compiler to fail with a given
fatal error message, the reducer could be used to look for a smaller
version of the module that causes the compiler to fail with the same
fatal error message.

To suggest an additional capability for the reducer, [file an
issue](https://github.com/KhronosGroup/SPIRV-Tools/issues]) with
"Reducer:" as the start of its title.


### Fuzzer

*Note:* The fuzzer is still under development.

The fuzzer applies semantics-preserving transformations to a SPIR-V binary
module, to produce an equivalent module.  The original and transformed modules
should produce essentially identical results when executed on identical inputs:
their results should differ only due to floating-point round-off, if at all.
Significant differences in results can pinpoint bugs in tools that process
SPIR-V binaries, such as miscompilations.  This *metamorphic testing* approach
is similar to the method used by the [GraphicsFuzz
project](https://github.com/google/graphicsfuzz) for fuzzing of GLSL shaders.

To suggest an additional capability for the fuzzer, [file an
issue](https://github.com/KhronosGroup/SPIRV-Tools/issues]) with
"Fuzzer:" as the start of its title.


### Extras

* [Utility filters](#utility-filters)
* Build target `spirv-tools-vimsyntax` generates file `spvasm.vim`.
  Copy that file into your `$HOME/.vim/syntax` directory to get SPIR-V assembly syntax
  highlighting in Vim.  This build target is not built by default.

## Contributing

The SPIR-V Tools project is maintained by members of the The Khronos Group Inc.,
and is hosted at https://github.com/KhronosGroup/SPIRV-Tools.

Consider joining the `public_spirv_tools_dev@khronos.org` mailing list, via
[https://www.khronos.org/spir/spirv-tools-mailing-list/](https://www.khronos.org/spir/spirv-tools-mailing-list/).
The mailing list is used to discuss development plans for the SPIRV-Tools as an open source project.
Once discussion is resolved,
specific work is tracked via issues and sometimes in one of the
[projects][spirv-tools-projects].

(To provide feedback on the SPIR-V _specification_, file an issue on the
[SPIRV-Headers][spirv-headers] GitHub repository.)

See [`docs/projects.md`](docs/projects.md) to see how we use the
[GitHub Project
feature](https://help.github.com/articles/tracking-the-progress-of-your-work-with-projects/)
to organize planned and in-progress work.

Contributions via merge request are welcome. Changes should:
* Be provided under the [Apache 2.0](#license).
* You'll be prompted with a one-time "click-through"
  [Khronos Open Source Contributor License Agreement][spirv-tools-cla]
  (CLA) dialog as part of submitting your pull request or
  other contribution to GitHub.
* Include tests to cover updated functionality.
* C++ code should follow the [Google C++ Style Guide][cpp-style-guide].
* Code should be formatted with `clang-format`.
  [kokoro/check-format/build.sh](kokoro/check-format/build.sh)
  shows how to download it. Note that we currently use
  `clang-format version 5.0.0` for SPIRV-Tools. Settings are defined by
  the included [.clang-format](.clang-format) file.

We intend to maintain a linear history on the GitHub `master` branch.

### Source code organization

* `example`: demo code of using SPIRV-Tools APIs
* `external/googletest`: Intended location for the
  [googletest][googletest] sources, not provided
* `external/effcee`: Location of [Effcee][effcee] sources, if the `effcee` library
  is not already configured by an enclosing project.
* `external/re2`: Location of [RE2][re2] sources, if the `re2` library is not already
  configured by an enclosing project.
  (The Effcee project already requires RE2.)
* `include/`: API clients should add this directory to the include search path
* `external/spirv-headers`: Intended location for
  [SPIR-V headers][spirv-headers], not provided
* `include/spirv-tools/libspirv.h`: C API public interface
* `source/`: API implementation
* `test/`: Tests, using the [googletest][googletest] framework
* `tools/`: Command line executables

Example of getting sources, assuming SPIRV-Tools is configured as a standalone project:

    git clone https://github.com/KhronosGroup/SPIRV-Tools.git   spirv-tools
    git clone https://github.com/KhronosGroup/SPIRV-Headers.git spirv-tools/external/spirv-headers
    git clone https://github.com/google/googletest.git          spirv-tools/external/googletest
    git clone https://github.com/google/effcee.git              spirv-tools/external/effcee
    git clone https://github.com/google/re2.git                 spirv-tools/external/re2

### Tests

The project contains a number of tests, used to drive development
and ensure correctness.  The tests are written using the
[googletest][googletest] framework.  The `googletest`
source is not provided with this project.  There are two ways to enable
tests:
* If SPIR-V Tools is configured as part of an enclosing project, then the
  enclosing project should configure `googletest` before configuring SPIR-V Tools.
* If SPIR-V Tools is configured as a standalone project, then download the
  `googletest` source into the `<spirv-dir>/external/googletest` directory before
  configuring and building the project.

*Note*: You must use a version of googletest that includes
[a fix][googletest-pull-612] for [googletest issue 610][googletest-issue-610].
The fix is included on the googletest master branch any time after 2015-11-10.
In particular, googletest must be newer than version 1.7.0.

### Dependency on Effcee

Some tests depend on the [Effcee][effcee] library for stateful matching.
Effcee itself depends on [RE2][re2].

* If SPIRV-Tools is configured as part of a larger project that already uses
  Effcee, then that project should include Effcee before SPIRV-Tools.
* Otherwise, SPIRV-Tools expects Effcee sources to appear in `external/effcee`
  and RE2 sources to appear in `external/re2`.


## Build

Instead of building manually, you can also download the binaries for your
platform directly from the [master-tot release][master-tot-release] on GitHub.
Those binaries are automatically uploaded by the buildbots after successful
testing and they always reflect the current top of the tree of the master
branch.

In order to build the code, you first need to sync the external repositories
that it depends on. Assume that `<spirv-dir>` is the root directory of the
checked out code:

```sh
cd <spirv-dir>
git clone https://github.com/KhronosGroup/SPIRV-Headers.git external/spirv-headers
git clone https://github.com/google/effcee.git external/effcee
git clone https://github.com/google/re2.git external/re2
git clone https://github.com/google/googletest.git external/googletest # optional

```

*Note*:
The script `utils/git-sync-deps` can be used to checkout and/or update the
contents of the repos under `external/` instead of manually maintaining them.

### Build using CMake
You can build the project using [CMake][cmake]:

```sh
cd <spirv-dir>
mkdir build && cd build
cmake [-G <platform-generator>] <spirv-dir>
```

Once the build files have been generated, build using the appropriate build
command (e.g. `ninja`, `make`, `msbuild`, etc.; this depends on the platform
generator used above), or use your IDE, or use CMake to run the appropriate build
command for you:

```sh
cmake --build . [--config Debug]  # runs `make` or `ninja` or `msbuild` etc.
```

#### Note about the fuzzer

The SPIR-V fuzzer, `spirv-fuzz`, can only be built via CMake, and is disabled by
default. To build it, clone protobuf and use the `SPIRV_BUILD_FUZZER` CMake
option, like so:

```sh
# In <spirv-dir> (the SPIRV-Tools repo root):
git clone --depth=1 --branch v3.13.0 https://github.com/protocolbuffers/protobuf external/protobuf

# In your build directory:
cmake [-G <platform-generator>] <spirv-dir> -DSPIRV_BUILD_FUZZER=ON
cmake --build . --config Debug
```

You can also add `-DSPIRV_ENABLE_LONG_FUZZER_TESTS=ON` to build additional
fuzzer tests.


### Build using Bazel
You can also use [Bazel](https://bazel.build/) to build the project.
```sh
cd <spirv-dir>
bazel build :all
```

### Tools you'll need

For building and testing SPIRV-Tools, the following tools should be
installed regardless of your OS:

- [CMake](http://www.cmake.org/): if using CMake for generating compilation
targets, you need to install CMake Version 2.8.12 or later.
- [Python 3](http://www.python.org/): for utility scripts and running the test
suite.
- [Bazel](https://bazel.build/) (optional): if building the source with Bazel,
you need to install Bazel Version 0.29.1 on your machine. Other versions may
also work, but are not verified.

SPIRV-Tools is regularly tested with the following compilers:

On Linux
- GCC version 4.8.5
- Clang version 3.8

On MacOS
- AppleClang 10.0

On Windows
- Visual Studio 2015
- Visual Studio 2017

Other compilers or later versions may work, but they are not tested.

### CMake options

The following CMake options are supported:

* `SPIRV_BUILD_FUZZER={ON|OFF}`, default `OFF` - Build the spirv-fuzz tool.
* `SPIRV_COLOR_TERMINAL={ON|OFF}`, default `ON` - Enables color console output.
* `SPIRV_SKIP_TESTS={ON|OFF}`, default `OFF`- Build only the library and
  the command line tools.  This will prevent the tests from being built.
* `SPIRV_SKIP_EXECUTABLES={ON|OFF}`, default `OFF`- Build only the library, not
  the command line tools and tests.
* `SPIRV_USE_SANITIZER=<sanitizer>`, default is no sanitizing - On UNIX
  platforms with an appropriate version of `clang` this option enables the use
  of the sanitizers documented [here][clang-sanitizers].
  This should only be used with a debug build.
* `SPIRV_WARN_EVERYTHING={ON|OFF}`, default `OFF` - On UNIX platforms enable
  more strict warnings.  The code might not compile with this option enabled.
  For Clang, enables `-Weverything`.  For GCC, enables `-Wpedantic`.
  See [`CMakeLists.txt`](CMakeLists.txt) for details.
* `SPIRV_WERROR={ON|OFF}`, default `ON` - Forces a compilation error on any
  warnings encountered by enabling the compiler-specific compiler front-end
  option.  No compiler front-end options are enabled when this option is OFF.

Additionally, you can pass additional C preprocessor definitions to SPIRV-Tools
via setting `SPIRV_TOOLS_EXTRA_DEFINITIONS`. For example, by setting it to
`/D_ITERATOR_DEBUG_LEVEL=0` on Windows, you can disable checked iterators and
iterator debugging.

### Android

SPIR-V Tools supports building static libraries `libSPIRV-Tools.a` and
`libSPIRV-Tools-opt.a` for Android:

```
cd <spirv-dir>

export ANDROID_NDK=/path/to/your/ndk

mkdir build && cd build
mkdir libs
mkdir app

$ANDROID_NDK/ndk-build -C ../android_test     \
                      NDK_PROJECT_PATH=.      \
                      NDK_LIBS_OUT=`pwd`/libs \
                      NDK_APP_OUT=`pwd`/app
```

### Updating DEPS
Occasionally the entries in DEPS will need to be updated. This is done on demand
when there is a request to do this, often due to downstream breakages. There is
a script `utils/roll_deps.sh` provided, which will generate a patch with the
updated DEPS values. This will still need to be tested in your checkout to
confirm that there are no integration issues that need to be resolved.

## Library

### Usage

The internals of the library use C++11 features, and are exposed via both a C
and C++ API.

In order to use the library from an application, the include path should point
to `<spirv-dir>/include`, which will enable the application to include the
header `<spirv-dir>/include/spirv-tools/libspirv.h{|pp}` then linking against
the static library in `<spirv-build-dir>/source/libSPIRV-Tools.a` or
`<spirv-build-dir>/source/SPIRV-Tools.lib`.
For optimization, the header file is
`<spirv-dir>/include/spirv-tools/optimizer.hpp`, and the static library is
`<spirv-build-dir>/source/libSPIRV-Tools-opt.a` or
`<spirv-build-dir>/source/SPIRV-Tools-opt.lib`.

* `SPIRV-Tools` CMake target: Creates the static library:
  * `<spirv-build-dir>/source/libSPIRV-Tools.a` on Linux and OS X.
  * `<spirv-build-dir>/source/libSPIRV-Tools.lib` on Windows.
* `SPIRV-Tools-opt` CMake target: Creates the static library:
  * `<spirv-build-dir>/source/libSPIRV-Tools-opt.a` on Linux and OS X.
  * `<spirv-build-dir>/source/libSPIRV-Tools-opt.lib` on Windows.

#### Entry points

The interfaces are still under development, and are expected to change.

There are five main entry points into the library in the C interface:

* `spvTextToBinary`: An assembler, translating text to a binary SPIR-V module.
* `spvBinaryToText`: A disassembler, translating a binary SPIR-V module to
  text.
* `spvBinaryParse`: The entry point to a binary parser API.  It issues callbacks
  for the header and each parsed instruction.  The disassembler is implemented
  as a client of `spvBinaryParse`.
* `spvValidate` implements the validator functionality. *Incomplete*
* `spvValidateBinary` implements the validator functionality. *Incomplete*

The C++ interface is comprised of three classes, `SpirvTools`, `Optimizer` and
`Linker`, all in the `spvtools` namespace.
* `SpirvTools` provides `Assemble`, `Disassemble`, and `Validate` methods.
* `Optimizer` provides methods for registering and running optimization passes.
* `Linker` provides methods for combining together multiple binaries.

## Command line tools

Command line tools, which wrap the above library functions, are provided to
assemble or disassemble shader files.  It's a convention to name SPIR-V
assembly and binary files with suffix `.spvasm` and `.spv`, respectively.

### Assembler tool

The assembler reads the assembly language text, and emits the binary form.

The standalone assembler is the executable called `spirv-as`, and is located in
`<spirv-build-dir>/tools/spirv-as`.  The functionality of the assembler is implemented
by the `spvTextToBinary` library function.

* `spirv-as` - the standalone assembler
  * `<spirv-dir>/tools/as`

Use option `-h` to print help.

### Disassembler tool

The disassembler reads the binary form, and emits assembly language text.

The standalone disassembler is the executable called `spirv-dis`, and is located in
`<spirv-build-dir>/tools/spirv-dis`. The functionality of the disassembler is implemented
by the `spvBinaryToText` library function.

* `spirv-dis` - the standalone disassembler
  * `<spirv-dir>/tools/dis`

Use option `-h` to print help.

The output includes syntax colouring when printing to the standard output stream,
on Linux, Windows, and OS X.

### Linker tool

The linker combines multiple SPIR-V binary modules together, resulting in a single
binary module as output.

This is a work in progress.
The linker does not support OpenCL program linking options related to math
flags. (See section 5.6.5.2 in OpenCL 1.2)

* `spirv-link` - the standalone linker
  * `<spirv-dir>/tools/link`

### Optimizer tool

The optimizer processes a SPIR-V binary module, applying transformations
in the specified order.

This is a work in progress, with initially only few available transformations.

* `spirv-opt` - the standalone optimizer
  * `<spirv-dir>/tools/opt`

### Validator tool

*Warning:* This functionality is under development, and is incomplete.

The standalone validator is the executable called `spirv-val`, and is located in
`<spirv-build-dir>/tools/spirv-val`. The functionality of the validator is implemented
by the `spvValidate` library function.

The validator operates on the binary form.

* `spirv-val` - the standalone validator
  * `<spirv-dir>/tools/val`

### Reducer tool

The reducer shrinks a SPIR-V binary module, guided by a user-supplied
*interestingness test*.

This is a work in progress, with initially only shrinks a module in a few ways.

* `spirv-reduce` - the standalone reducer
  * `<spirv-dir>/tools/reduce`

Run `spirv-reduce --help` to see how to specify interestingness.

### Fuzzer tool

The fuzzer transforms a SPIR-V binary module into a semantically-equivalent
SPIR-V binary module by applying transformations in a randomized fashion.

This is a work in progress, with initially only a few semantics-preserving
transformations.

* `spirv-fuzz` - the standalone fuzzer
  * `<spirv-dir>/tools/fuzz`

Run `spirv-fuzz --help` for a detailed list of options.

### Control flow dumper tool

The control flow dumper prints the control flow graph for a SPIR-V module as a
[GraphViz](http://www.graphviz.org/) graph.

This is experimental.

* `spirv-cfg` - the control flow graph dumper
  * `<spirv-dir>/tools/cfg`

### Utility filters

* `spirv-lesspipe.sh` - Automatically disassembles `.spv` binary files for the
  `less` program, on compatible systems.  For example, set the `LESSOPEN`
  environment variable as follows, assuming both `spirv-lesspipe.sh` and
  `spirv-dis` are on your executable search path:
  ```
   export LESSOPEN='| spirv-lesspipe.sh "%s"'
  ```
  Then you page through a disassembled module as follows:
  ```
  less foo.spv
  ```
  * The `spirv-lesspipe.sh` script will pass through any extra arguments to
    `spirv-dis`.  So, for example, you can turn off colours and friendly ID
    naming as follows:
    ```
    export LESSOPEN='| spirv-lesspipe.sh "%s" --no-color --raw-id'
    ```

* [vim-spirv](https://github.com/kbenzie/vim-spirv) - A vim plugin which
  supports automatic disassembly of `.spv` files using the `:edit` command and
  assembly using the `:write` command. The plugin also provides additional
  features which include; syntax highlighting; highlighting of all ID's matching
  the ID under the cursor; and highlighting errors where the `Instruction`
  operand of `OpExtInst` is used without an appropriate `OpExtInstImport`.

* `50spirv-tools.el` - Automatically disassembles '.spv' binary files when
  loaded into the emacs text editor, and re-assembles them when saved,
  provided any modifications to the file are valid.  This functionality
  must be explicitly requested by defining the symbol
  SPIRV_TOOLS_INSTALL_EMACS_HELPERS as follows:
  ```
  cmake -DSPIRV_TOOLS_INSTALL_EMACS_HELPERS=true ...
  ```

  In addition, this helper is only installed if the directory /etc/emacs/site-start.d
  exists, which is typically true if emacs is installed on the system.

  Note that symbol IDs are not currently preserved through a load/edit/save operation.
  This may change if the ability is added to spirv-as.


### Tests

Tests are only built when googletest is found. Use `ctest` to run all the
tests.

## Future Work
<a name="future"></a>

_See the [projects pages](https://github.com/KhronosGroup/SPIRV-Tools/projects)
for more information._

### Assembler and disassembler

* The disassembler could emit helpful annotations in comments.  For example:
  * Use variable name information from debug instructions to annotate
    key operations on variables.
  * Show control flow information by annotating `OpLabel` instructions with
    that basic block's predecessors.
* Error messages could be improved.

### Validator

This is a work in progress.

### Linker

* The linker could accept math transformations such as allowing MADs, or other
  math flags passed at linking-time in OpenCL.
* Linkage attributes can not be applied through a group.
* Check decorations of linked functions attributes.
* Remove dead instructions, such as OpName targeting imported symbols.

## Licence
<a name="license"></a>
Full license terms are in [LICENSE](LICENSE)
```
Copyright (c) 2015-2016 The Khronos Group Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

[spirv-tools-cla]: https://cla-assistant.io/KhronosGroup/SPIRV-Tools
[spirv-tools-projects]: https://github.com/KhronosGroup/SPIRV-Tools/projects
[spirv-tools-mailing-list]: https://www.khronos.org/spir/spirv-tools-mailing-list
[spirv-registry]: https://www.khronos.org/registry/spir-v/
[spirv-headers]: https://github.com/KhronosGroup/SPIRV-Headers
[googletest]: https://github.com/google/googletest
[googletest-pull-612]: https://github.com/google/googletest/pull/612
[googletest-issue-610]: https://github.com/google/googletest/issues/610
[effcee]: https://github.com/google/effcee
[re2]: https://github.com/google/re2
[CMake]: https://cmake.org/
[cpp-style-guide]: https://google.github.io/styleguide/cppguide.html
[clang-sanitizers]: http://clang.llvm.org/docs/UsersManual.html#controlling-code-generation
[master-tot-release]: https://github.com/KhronosGroup/SPIRV-Tools/releases/tag/master-tot
>>>>>>> BRANCH (3eb4dd Squashed 'third_party/SPIRV-Tools/' changes from 8a5500656..)
