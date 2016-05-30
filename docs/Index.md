SwiftShader Documentation
=========================

SwiftShader provides high-performance graphics rendering on the CPU. It eliminates the dependency on graphics hardware or its capabilities.

Architecture
------------

SwiftShader provides shared libraries (DLLs) which implement standardized graphics APIs. Applications already using these APIs thus don't require any changes to use SwiftShader. It can run entirely in user space, or as a driver (for Android), and output to either a frame buffer, window, or an offscreen buffer.

There are four major layers:

* API
* Renderer
* Reactor
* JIT

The API layer is an implementation of a graphics API, such as OpenGL (ES) or Direct3D, on top of the Renderer interface. It is responsible for managing API-level resources and rendering state, as well as compiling high-level shaders to bytecode form. 

The Renderer layer generates specialized processing routines for draw calls and coordinates the execution of rendering tasks. It defines the data structures used and how the processing is performed.

Reactor is an embedded language for C++ to dynamically generate code in a WYSIWYG fashion. It allows to specialize the processing routines for the state and shaders used by each draw call. Its syntax closely resembles C and shading languages, to make the code generation easily readable.

The JIT layer is a run-time compiler, such as LLVM's JIT. Reactor records its operations in an in-memory intermediate form which can be materialized by the JIT into a function which can be called directly.

Design
------

### Reactor

To generate code for an expression such as `float y = 1 - x;` directly with LLVM, we'd need code like `Value *valueY = BinaryOperator::CreateSub(ConstantInt::get(Type::getInt32Ty(Context), 1), valueX, "y", basicBlock);`. This is very verbose and becomes hard to read for longer expressions. Using C++ operator overloading, [Reactor](../src/Reactor/) simplifies this to `Float y = 1 - x;`. Note that Reactor types have the same names as C types, but starting with a capital letter.

Likewise `If()`, `Else`, and `For(,,)` implement their C counterparts. While making these constructs so similiar to the C++ in which they are written might cause some confusion at first, it provides a powerful abstraction for code specialization. For example to produce the code for an addition or a subtraction, one could write `x = addOrSub ? x + y : x - y;`. Note that only one operation ends up in the generated code.

We refer to the functions generate by Reactor code as [Routine](../src/Reactor/Routine.hpp)s.

### Renderer

The [Renderer](../src/Renderer/) is implemented in three main parts: the VertexProcessor, SetupProcessor, and PixelProcessor. Each "processor" produces a corresponding Reactor Routine, and 

### OpenGL

The OpenGL (ES) and EGL APIs are implemented in [src/OpenGL/](../src/OpenGL/).

The GLSL compiler is implemented in [src/OpenGL/compiler/](../src/OpenGL/compiler/). It uses [Flex](http://flex.sourceforge.net/) and [Bison](https://www.gnu.org/software/bison/) to tokenize and parse GLSL shader source. It produces an [abstract syntax tree](https://en.wikipedia.org/wiki/Abstract_syntax_tree) (AST), which is then traversed to output assembly-level instructions in [OutputASM.cpp](../src/OpenGL/compiler/OutputASM.cpp).

The [EGL](https://www.khronos.org/registry/egl/specs/eglspec.1.4.20110406.pdf) API is implemented in [src/OpenGL/libEGL/](../src/OpenGL/libEGL/). Its entry functions are listed in [libEGL.def](../src/OpenGL/libEGL/libEGL.def) (for Windows) and [exports.map](../src/OpenGL/libEGL/exports.map) (for Linux), and defined in [main.cpp](../src/OpenGL/libEGL/main.cpp) and implemented in [libEGL.cpp](../src/OpenGL/libEGL/libEGL.cpp).