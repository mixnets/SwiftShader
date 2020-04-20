@echo on

SETLOCAL ENABLEDELAYEDEXPANSION

SET PATH=C:\python36;C:\Program Files\cmake\bin;%PATH%
set SRC=%cd%\git\SwiftShader

cd %SRC%
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!


IF "%LLVM_VERSION%"=="10.0" (
  ECHO "TODO(b/152339534): LLVM 10 migration is still in progress"
  EXIT 0
)

# Lower the amount of debug info, to reduce Kokoro build times.
SET LESS_DEBUG_INFO=1

cd %SRC%\build
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

cmake --version

REM cmake .. ^
REM     -G "Visual Studio 15 2017 Win64" ^
REM     -Thost=x64 ^
REM     "-DCMAKE_BUILD_TYPE=%BUILD_TYPE%" ^
REM     "-DREACTOR_BACKEND=%REACTOR_BACKEND%" ^
REM     "-DSWIFTSHADER_LLVM_VERSION=%LLVM_VERSION%" ^
REM     "-DREACTOR_VERIFY_LLVM_IR=1" ^
REM     "-DLESS_DEBUG_INFO=%LESS_DEBUG_INFO%"
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

REM cmake --build .
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

REM REM Run the unit tests. Some must be run from project root
REM cd %SRC%
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!
REM SET SWIFTSHADER_DISABLE_DEBUGGER_WAIT_DIALOG=1

REM build\Debug\ReactorUnitTests.exe
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

REM build\Debug\gles-unittests.exe
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

REM build\Debug\system-unittests.exe
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

REM build\Debug\vk-unittests.exe
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

REM Rem Incrementally build and run rr::Print unit tests
REM cd %SRC%\build
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

REM cmake "-DREACTOR_ENABLE_PRINT=1" ..
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

REM cmake --build . --target ReactorUnitTests
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

REM cd %SRC%
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!
REM SET SWIFTSHADER_DISABLE_DEBUGGER_WAIT_DIALOG=1

REM build\Debug\ReactorUnitTests.exe --gtest_filter=ReactorUnitTests.Print*
REM if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!
