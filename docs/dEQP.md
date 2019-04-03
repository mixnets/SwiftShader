dEQP
====

These steps are specifically for testing SwiftShader's OpenGL ES 3.0 implementation using dEQP on Windows (steps for Linux below the Windows instructions).

Prerequisites
-------------

1. Install the latest [Python 2.X](https://www.python.org/downloads/)
2. Install [Visual Studio](https://visualstudio.microsoft.com/vs/community/)
3. Install [CMake](https://cmake.org/download/)
4. Install [Go](https://golang.org/doc/install) 32-bit (Important: must be 32 bit)
5. Install [MinGW](http://www.mingw.org/)
6. Install [Git](https://git-scm.com/download/win)
7. Install [Android Studio](https://developer.android.com/studio/index.html)
8. Run Android Studio and install Android SDK.
9. Set environment variables: Config Panel -> System and Security -> System -> Advanced system settigns -> Environment Variables
  * Add `<path to python>` to your PATH environment variable
  * Add `<path to MinGW>\bin` to your PATH environment variable
  * Add `<path to adb>` to your PATH environment variable

    Note: abd is in the Android SDK, typically in `C:\Users\<username>\AppData\Local\Android\sdk\platform-tools`

10. Install GCC. In 'cmd', run:

    `mingw-get install gcc`

    Note: Using Cygwin GCC currently doesn't work.

11. (Optional) Install [TortoiseGit](https://tortoisegit.org/)

Getting the Code
----------------

12. Get dEQP (either in 'cmd' or by using TortoiseGit):

    `git clone https://android.googlesource.com/platform/external/deqp`

13. Get dEQP's dependencies. In your dEQP root directory, open 'cmd' and run:

    `python external\fetch_sources.py`

14. Get Cherry (either in 'cmd' or by using TortoiseGit):

    `git clone https://android.googlesource.com/platform/external/cherry`

15. Set environment variable (see point 9):

    Add new variable GOPATH='`<path to cherry>`'

Building the code
-----------------

16. Build dEQP's Visual Studio files using the CMake GUI, or, in the dEQP root dir, run:
    ```
    mkdir build
    cd build
    cmake ..
    ```
    Note: If you have multiple versions of Visual Studio installed and you want to make sure cmake is using the correct version of Visual Studio, you can specify it by calling, for example:

    `cmake .. -G "Visual Studio 15 2017 Win64"`

    Also note: don't call 'cmake .' directly in the root directory. It will make things fails later on. If you do, simply erase the files created by CMake and follow the steps above.

17. Build dEQP:

    Open `<path to dEQP>\build\dEQP-Core-default.sln` in Visual Studio and Build Solution

    Note: Choose a 'Release' build, unless you really mean to debug dEQP

18. Generate test cases:
    ```
    mkdir <path to cherry>\data
    cd <path to dEQP>
    python scripts\build_caselists.py <path to cherry>\data
    ```

Preparing the server
--------------------

19. Edit `<path to cherry>\cherry\data.go`
* Search for `../candy-build/deqp-wgl` and replace that by `<path to deqp>/build`
* Just above, add an option to CommandLine: `--deqp-gl-context-type=egl`
* Just below, modify the BinaryPath from 'Debug' to 'Release' if you did a Release build at step 17

Testing OpenGL ES
-----------------

20. a) Assuming you already built SwiftShader in the `build` folder, copy these two files:

    `libEGL.dll`\
    `libGLESv2.dll`

    From:

    `<path to SwiftShader>\build\Release_x64` or\
    `<path to SwiftShader>\build\Debug_x64`

    To:

    `<path to dEQP>\build\modules\gles3\Release` (Again, assuming you did a Release build at step 17)

Testing Vulkan
--------------

20. b) Assuming you already built SwiftShader, copy and rename this file:

    `<path to SwiftShader>\build\Release_x64\vk_swiftshader.dll` or\
    `<path to SwiftShader>\build\Debug_x64\vk_swiftshader.dll`

    To:

    `<path to dEQP>\build\external\vulkancts\modules\vulkan\vulkan-1.dll`

    This will cause dEQP to load SwiftShader's Vulkan implementatin directly, without going through a system-provided [loader](https://github.com/KhronosGroup/Vulkan-Loader/blob/master/loader/LoaderAndLayerInterface.md#the-loader) library or any layers.

    To use SwiftShader as an [Installable Client Driver](https://github.com/KhronosGroup/Vulkan-Loader/blob/master/loader/LoaderAndLayerInterface.md#installable-client-drivers) (ICD) instead:
    * Edit environment variables:
      * Define VK_ICD_FILENAMES to `<path to SwiftShader>\src\Vulkan\vk_swiftshader_icd.json`
    * If the location of `vk_swiftshader.dll` you're using is different than the one specified in `src\Vulkan\vk_swiftshader_icd.json`, modify it to point to the `vk_swiftshader.dll` file you want to use.

Running the tests
-----------------

21. Start the test server. Go to `<path to cherry>` and run:

    `go run server.go`

22. Open your favorite browser and navigate to `localhost:8080`

    Get Started -> Choose Device 'localhost' -> Select Tests 'dEQP-GLES3' -> Execute tests!

Mustpass sets
-------------

dEQP contains more tests than what is expected to pass by a conformant implementation (e.g. some tests are considered too strict, or assume certain undefined behavior). The [android\cts\master\gles3-master.txt](https://android.googlesource.com/platform/external/deqp/+/master/android/cts/master/gles3-master.txt) text file which can be loaded in Cherry's 'Test sets' tab to only run the latest tests expected to pass by certified Android devices.

Linux
-----

The Linux process is similar to Windows. However it doesn't use Release or Debug variants and it uses shared object files instead of DLLs.

1. Install the latest [Python 2.X](https://www.python.org/downloads/)
2. Install GCC and Make. In a terminal, run:

    `sudo apt-get install gcc make`

3. Install [CMake](https://cmake.org/download/)
4. Install [Go](https://golang.org/doc/install) 32-bit (Important: must be 32 bit)
5. Install Git. In a terminal, run:

    `sudo apt-get install git`

6. Install [Android Studio](https://developer.android.com/studio/index.html)
7. Run Android Studio and install Android SDK.
8. Download the [Vulkan SDK](https://vulkan.lunarg.com/) and unpack it into a location you like.

Getting the Code
----------------

9. Get Swiftshader. In a terminal, go to the location you want to keep Swiftshader, and run:

    `git clone https://swiftshader.googlesource.com/SwiftShader`

10. Get dEQP, similar to step 9:

    `git clone https://android.googlesource.com/platform/external/deqp`

11. Get dEQP's dependencies. In your dEQP root directory, run:

    `python external\fetch_sources.py`

12. Get Cherry, similar to step 9:

    `git clone https://android.googlesource.com/platform/external/cherry`

13. Set environment variable. Open ~/.bashrc in your preferred editor and add the following line:

    export GOPATH='`<path to cherry>`'

Building the code
-----------------

14. Build Swiftshader. In the Swiftshader root dir, run:
    ```
    cd build
    cmake ..
    make --jobs=8

    ./unittests
    ./OGLES2HelloAPI
    ```

16. Set your environment variables. Open ~/.bashrc in your preferred editor and add the following lines:

    ```
    export LD_LIBRARY_PATH="<Vulkan SDK location>/x86_64/lib:$LD_LIBRARY_PATH"
    export LD_LIBRARY_PATH="<Swiftshader location>/build:$LD_LIBRARY_PATH"
    export PATH="<Android SDK location>/platform-tools:$PATH"
    ```

    You may find the Android SDK at `~/Android/Sdk`.

    It's important that you perform this step before you build dEQP in the next step. CMake will search for library files in LD_LIBRARY_PATH. If it cannot discover Swiftshader's libEGL and libGLESv2 shared object files, then CMake will default to using your system's libEGL.so and libGLESv2.so files.

15. Build dEQP. In the dEQP root dir, run:
    ```
    mkdir build
    cd build
    cmake ..
    make --jobs=8
    ```

    Also note: don't call 'cmake .' directly in the root directory. It will make things fails later on. If you do, simply erase the files created by CMake and follow the steps above.

16. Generate test cases:
    ```
    mkdir <path to cherry>\data
    cd <path to dEQP>
    python scripts\build_caselists.py <path to cherry>\data
    ```

Preparing the server
--------------------

18. Edit `<path to cherry>\cherry\data.go`
* Search for "\.exe" and remove all instances.
* Search for `../candy-build/deqp-wgl/execserver/Release` and replace that by `<path to deqp>/build/execserver/`
* Just above, add an option to CommandLine: `--deqp-gl-context-type=egl`
* Just below, remove 'Debug/' from the BinaryPath.

Testing OpenGL ES
-----------------

19. a) Assuming you setup the LD_LIBRARY_PATH environment variable prior to running CMake in the dEQP build directory, you're all set.

Testing Vulkan
--------------

19. b) Use SwiftShader as an [Installable Client Driver](https://github.com/KhronosGroup/Vulkan-Loader/blob/master/loader/LoaderAndLayerInterface.md#installable-client-drivers) (ICD):
    * Edit ~/.bashrc and add the line:

      `export VK_ICD_FILENAMES="<path to SwiftShader>\src\Vulkan\vk_swiftshader_icd.json"`

Running the tests
-----------------

20. Start the test server. Go to `<path to cherry>` and run:

    `go run server.go`

21. Open your favorite browser and navigate to `localhost:8080`

    Get Started -> Choose Device 'localhost' -> Select Tests 'dEQP-GLES3' -> Execute tests!

22. If you want to run Vulkan tests in the command line, go to the build directory in dEQP root. Then run the following command:

    `external/vulkanacts/modules/vulkan/deqp-vk`

    You can also run individual tests with:

    `external/vulkanacts/modules/vulkan/deqp-vk --deqp-case=<test name>`

    And you can find a list of the test names in `<Swiftshader root>/tests/regres/testlists/vk-master.txt` However, deqp-vk will cease upon the first failure. It's recommended that you use cherry for your testing needs unless you know what you're doing.

Mustpass sets
-------------

dEQP contains more tests than what is expected to pass by a conformant implementation (e.g. some tests are considered too strict, or assume certain undefined behavior). The [android\cts\master\gles3-master.txt](https://android.googlesource.com/platform/external/deqp/+/master/android/cts/master/gles3-master.txt) text file which can be loaded in Cherry's 'Test sets' tab to only run the latest tests expected to pass by certified Android devices.
