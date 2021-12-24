<<<<<<< HEAD   (87c78a Use X11 shm extension for presenting)
# SwiftShader

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Introduction
------------

SwiftShader is a high-performance CPU-based implementation of the Vulkan graphics API<sup>1</sup><sup>2</sup>. Its goal is to provide hardware independence for advanced 3D graphics.

> NOTE: SwiftShader's OpenGL ES implementation is no longer supported, and will be removed. Read more about our recommendation to use [ANGLE on top of SwiftShader Vulkan here](docs/ANGLE.md) (aka. "SwANGLE").

Building
--------

SwiftShader libraries can be built for Windows, Linux, and macOS.\
Android and Chrome (OS) build environments are also supported.

* **CMake**
\
  [Install CMake](https://cmake.org/download/) for Linux, macOS, or Windows and use either [the GUI](https://cmake.org/runningcmake/) or run the following terminal commands:
  ```
  cd build
  cmake ..
  cmake --build . --parallel

  ./vk-unittests
  ```
  Tip: Set the [CMAKE_BUILD_PARALLEL_LEVEL](https://cmake.org/cmake/help/latest/envvar/.CMAKE_BUILD_PARALLEL_LEVEL.html#envvar:CMAKE_BUILD_PARALLEL_LEVEL) environment variable to control the level of parallelism.


* **Visual Studio**
\
  To build the Vulkan ICD library, use [Visual Studio 2019](https://visualstudio.microsoft.com/vs/community/) to open the project folder and wait for it to run CMake. Open the [CMake Targets View](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019#ide-integration) in the Solution Explorer and select the vk_swiftshader project to [build](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019#building-cmake-projects) it.


Usage
-----

The SwiftShader libraries act as drop-in replacements for graphics drivers.

On Windows, most applications can be made to use SwiftShader's DLLs by placing them in the same folder as the executable. On Linux, the `LD_LIBRARY_PATH` environment variable or `-rpath` linker option can be used to direct applications to search for shared libraries in the indicated directory first.

In general, Vulkan applications look for a shared library named `vulkan-1.dll` on Windows (`vulkan-1.so` on Linux). This 'loader' library then redirects API calls to the actual Installable Client Driver (ICD). SwiftShader's ICD is named `libvk_swiftshader.dll`, but it can be renamed to `vulkan-1.dll` to be loaded directly by the application. Alternatively, you can set the `VK_ICD_FILENAMES` environment variable to the path to `vk_swiftshader_icd.json` file that is generated under the build directory (e.g. `.\SwiftShader\build\Windows\vk_swiftshader_icd.json`). To learn more about how Vulkan loading works, read the [official documentation here](https://github.com/KhronosGroup/Vulkan-Loader/blob/master/loader/LoaderAndLayerInterface.md).

Contributing
------------

See [CONTRIBUTING.txt](CONTRIBUTING.txt) for important contributing requirements.

The canonical repository for SwiftShader is hosted at:
https://swiftshader.googlesource.com/SwiftShader.

All changes must be reviewed and approved in the [Gerrit](https://www.gerritcodereview.com/) review tool at:
https://swiftshader-review.googlesource.com.

Authenticate your account here:
https://swiftshader-review.googlesource.com/new-password (use the same e-mail address as the one configured as the [Git commit author](https://git-scm.com/book/en/v2/Getting-Started-First-Time-Git-Setup#_your_identity)).

All changes require a [Change-ID](https://gerrit-review.googlesource.com/Documentation/user-changeid.html) tag in the commit message. A [commit hook](https://git-scm.com/book/en/v2/Customizing-Git-Git-Hooks) may be used to add this tag automatically, and can be found at:
https://gerrit-review.googlesource.com/tools/hooks/commit-msg. You can execute `git clone https://swiftshader.googlesource.com/SwiftShader` and manually place the commit hook in `SwiftShader/.git/hooks/`, or to clone the repository and install the commit hook in one go:

    git clone https://swiftshader.googlesource.com/SwiftShader && (cd SwiftShader && git submodule update --init --recursive third_party/git-hooks && ./third_party/git-hooks/install_hooks.sh)

On Windows, this command line requires using the [Git Bash Shell](https://www.atlassian.com/git/tutorials/git-bash).

Changes are uploaded to Gerrit by executing:

    git push origin HEAD:refs/for/master

When ready, [add](https://gerrit-review.googlesource.com/Documentation/intro-user.html#adding-reviewers) a project [owner](OWNERS) as a reviewer on your change.

Some tests will automatically be run against the change. Notably, [presubmit.sh](tests/presubmit.sh) verifies the change has been formatted using [clang-format 11.0.1](tests/kokoro/gcp_ubuntu/check_style.sh). Most IDEs come with clang-format support, but may require upgrading/downgrading to the [clang-format version 11.0.0](https://github.com/llvm/llvm-project/releases/tag/llvmorg-11.0.0) *release* version (notably Chromium's buildtools has a clang-format binary which can be an in-between revision which produces different formatting results).

Testing
-------

SwiftShader's Vulkan implementation can be tested using the [dEQP](https://github.com/KhronosGroup/VK-GL-CTS) test suite.

See [docs/dEQP.md](docs/dEQP.md) for details.

Third-Party Dependencies
------------------------

The [third_party](third_party/) directory contains projects which originated outside of SwiftShader:

[subzero](third_party/subzero/) contains a fork of the [Subzero](https://chromium.googlesource.com/native_client/pnacl-subzero/) project. It originates from Google Chrome's (Portable) [Native Client](https://developer.chrome.com/native-client) project. The fork was made using [git-subtree](https://github.com/git/git/blob/master/contrib/subtree/git-subtree.txt) to include all of Subzero's history.

[llvm-subzero](third_party/llvm-subzero/) contains a minimized set of LLVM dependencies of the Subzero project.

[PowerVR_SDK](third_party/PowerVR_SDK/) contains a subset of the [PowerVR Graphics Native SDK](https://github.com/powervr-graphics/Native_SDK) for running several sample applications.

[googletest](third_party/googletest/) contains the [Google Test](https://github.com/google/googletest) project, as a Git submodule. It is used for running unit tests for Chromium, and Reactor unit tests. Run `git submodule update --init` to obtain/update the code. Any contributions should be made upstream.

Documentation
-------------

See [docs/Index.md](docs/Index.md).

Contact
-------

Public mailing list: [swiftshader@googlegroups.com](https://groups.google.com/forum/#!forum/swiftshader)

General bug tracker:  https://g.co/swiftshaderbugs \
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
# Marl

Marl is a hybrid thread / fiber task scheduler written in C++ 11.

## About

Marl is a C++ 11 library that provides a fluent interface for running tasks across a number of threads.

Marl uses a combination of fibers and threads to allow efficient execution of tasks that can block, while keeping a fixed number of hardware threads.

Marl supports Windows, macOS, Linux, FreeBSD, Fuchsia, Android and iOS (arm, aarch64, mips64, ppc64, rv64, x86 and x64).

Marl has no dependencies on other libraries (with an exception on googletest for building the optional unit tests).

Example:

```cpp
#include "marl/defer.h"
#include "marl/event.h"
#include "marl/scheduler.h"
#include "marl/waitgroup.h"

#include <cstdio>

int main() {
  // Create a marl scheduler using all the logical processors available to the process.
  // Bind this scheduler to the main thread so we can call marl::schedule()
  marl::Scheduler scheduler(marl::Scheduler::Config::allCores());
  scheduler.bind();
  defer(scheduler.unbind());  // Automatically unbind before returning.

  constexpr int numTasks = 10;

  // Create an event that is manually reset.
  marl::Event sayHello(marl::Event::Mode::Manual);

  // Create a WaitGroup with an initial count of numTasks.
  marl::WaitGroup saidHello(numTasks);

  // Schedule some tasks to run asynchronously.
  for (int i = 0; i < numTasks; i++) {
    // Each task will run on one of the 4 worker threads.
    marl::schedule([=] {  // All marl primitives are capture-by-value.
      // Decrement the WaitGroup counter when the task has finished.
      defer(saidHello.done());

      printf("Task %d waiting to say hello...\n", i);

      // Blocking in a task?
      // The scheduler will find something else for this thread to do.
      sayHello.wait();

      printf("Hello from task %d!\n", i);
    });
  }

  sayHello.signal();  // Unblock all the tasks.

  saidHello.wait();  // Wait for all tasks to complete.

  printf("All tasks said hello.\n");

  // All tasks are guaranteed to complete before the scheduler is destructed.
}
```

## Benchmarks

Graphs of several microbenchmarks can be found [here](https://google.github.io/marl/benchmarks).

## Building

Marl contains many unit tests and examples that can be built using CMake.

Unit tests require fetching the `googletest` external project, which can be done by typing the following in your terminal:

```bash
cd <path-to-marl>
git submodule update --init
```

### Linux and macOS

To build the unit tests and examples, type the following in your terminal:

```bash
cd <path-to-marl>
mkdir build
cd build
cmake .. -DMARL_BUILD_EXAMPLES=1 -DMARL_BUILD_TESTS=1
make
```

The resulting binaries will be found in `<path-to-marl>/build`

### Windows

Marl can be built using [Visual Studio 2019's CMake integration](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019).

### Using Marl in your CMake project

You can build and link Marl using `add_subdirectory()` in your project's `CMakeLists.txt` file:

```cmake
set(MARL_DIR <path-to-marl>) # example <path-to-marl>: "${CMAKE_CURRENT_SOURCE_DIR}/third_party/marl"
add_subdirectory(${MARL_DIR})
```

This will define the `marl` library target, which you can pass to `target_link_libraries()`:

```cmake
target_link_libraries(<target> marl) # replace <target> with the name of your project's target
```

You may also wish to specify your own paths to the third party libraries used by `marl`.
You can do this by setting any of the following variables before the call to `add_subdirectory()`:

```cmake
set(MARL_THIRD_PARTY_DIR <third-party-root-directory>) # defaults to ${MARL_DIR}/third_party
set(MARL_GOOGLETEST_DIR  <path-to-googletest>)         # defaults to ${MARL_THIRD_PARTY_DIR}/googletest
add_subdirectory(${MARL_DIR})
```

### Usage Recommendations

#### Capture marl synchronization primitves by value

All marl synchronization primitves aside from `marl::ConditionVariable` should be lambda-captured by **value**:

```c++
marl::Event event;
marl::schedule([=]{ // [=] Good, [&] Bad.
  event.signal();
})
```

Internally, these primitives hold a shared pointer to the primitive state. By capturing by value we avoid common issues where the primitive may be destructed before the last reference is used.

#### Create one instance of `marl::Scheduler`, use it for the lifetime of the process

The `marl::Scheduler` constructor can be expensive as it may spawn a number of hardware threads. \
Destructing the `marl::Scheduler` requires waiting on all tasks to complete.

Multiple `marl::Scheduler`s may fight each other for hardware thread utilization.

For these reasons, it is recommended to create a single `marl::Scheduler` for the lifetime of your process.

For example:

```c++
int main() {
  marl::Scheduler scheduler(marl::Scheduler::Config::allCores());
  scheduler.bind();
  defer(scheduler.unbind());

  return do_program_stuff();
}
```

#### Bind the scheduler to externally created threads

In order to call `marl::schedule()` the scheduler must be bound to the calling thread. Failure to bind the scheduler to the thread before calling `marl::schedule()` will result in undefined behavior.

`marl::Scheduler` may be simultaneously bound to any number of threads, and the scheduler can be retrieved from a bound thread with `marl::Scheduler::get()`.

A typical way to pass the scheduler from one thread to another would be:

```c++
std::thread spawn_new_thread() {
  // Grab the scheduler from the currently running thread.
  marl::Scheduler* scheduler = marl::Scheduler::get();

  // Spawn the new thread.
  return std::thread([=] {
    // Bind the scheduler to the new thread.
    scheduler->bind();
    defer(scheduler->unbind());

    // You can now safely call `marl::schedule()`
    run_thread_logic();
  });
}

```

Always remember to unbind the scheduler before terminating the thread. Forgetting to unbind will result in the `marl::Scheduler` destructor blocking indefinitely.

#### Don't use externally blocking calls in marl tasks

The `marl::Scheduler` internally holds a number of worker threads which will execute the scheduled tasks. If a marl task becomes blocked on a marl synchronization primitive, marl can yield from the blocked task and continue execution of other scheduled tasks.

Calling a non-marl blocking function on a marl worker thread will prevent that worker thread from being able to switch to execute other tasks until the blocking function has returned. Examples of these non-marl blocking functions include: [`std::mutex::lock()`](https://en.cppreference.com/w/cpp/thread/mutex/lock), [`std::condition_variable::wait()`](https://en.cppreference.com/w/cpp/thread/condition_variable/wait), [`accept()`](http://man7.org/linux/man-pages/man2/accept.2.html).

Short blocking calls are acceptable, such as a mutex lock to access a data structure. However be careful that you do not use a marl blocking call with a `std::mutex` lock held - the marl task may yield with the lock held, and block other tasks from re-locking the mutex. This sort of situation may end up with a deadlock.

If you need to make a blocking call from a marl worker thread, you may wish to use [`marl::blocking_call()`](https://github.com/google/marl/blob/main/include/marl/blockingcall.h), which will spawn a new thread for performing the call, allowing the marl worker to continue processing other scheduled tasks.

---

Note: This is not an officially supported Google product
>>>>>>> BRANCH (3b7e76 Squashed 'third_party/marl/' changes from 49602432d..7b43abf)
