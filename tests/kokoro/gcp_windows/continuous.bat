@echo on

SET PATH=%PATH%;C:\python27;C:\Program Files\cmake\bin

cd git\SwiftShader

git submodule update --init

SET MSBUILD="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild"
SET CONFIG=Debug

cd build

cmake .. -G "Visual Studio 15 2017 Win64" -Thost=x64 "-DREACTOR_BACKEND=%REACTOR_BACKEND%"

%MSBUILD% /p:Configuration=%CONFIG% SwiftShader.sln

SET SWIFTSHADER_DISABLE_DEBUGGER_WAIT_DIALOG=1

REM Run the unit tests. They must be run from project root
build\Debug\gles-unittests.exe
IF NOT "%REACTOR_BACKEND%"=="Subzero" (
    REM Currently vulkan does not work with Subzero.
    build\Debug\vk-unittests.exe
)