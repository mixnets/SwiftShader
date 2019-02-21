@echo on

cd git\SwiftShader

git submodule update --init

call "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

cd build
cmake ..
cmake --build .
