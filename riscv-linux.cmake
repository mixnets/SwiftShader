#[[
First compile tablegen on your host system.
(This happens automatically when compiling Swiftshader.)
Then provide the path to the binary.

cmake -DCMAKE_C_COMPILER=/path/to/riscv/clang \
-DCMAKE_CXX_COMPILER=/path/to/riscv/clang++ \
-DCMAKE_TOOLCHAIN_FILE=riscv-linux.cmake \
-DLLVM_DEFAULT_TARGET_TRIPLE=riscv64-unknown-linux-gnu \
-DLLVM_TARGET_ARCH=RISCV64 \
-DLLVM_TARGETS_TO_BUILD=RISCV \
-DLLVM_TABLEGEN=/path/to/host-built/third_party/llvm-project/llvm/bin/llvm-tblgen \
-DREACTOR_BACKEND="LLVM-Submodule" ..
]]

set(triple riscv64-unknown-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER clang++ CACHE FILEPATH "C++ cross-compiler for RISC-V")
set(CMAKE_CXX_COMPILER_TARGET ${triple})
set(CMAKE_C_COMPILER clang CACHE FILEPATH "C cross-compiler for RISC-V")
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR rv64)
set(CMAKE_CROSSCOMPILING True)