Variables Semantics and Implementation
======================================

Similar to [C++ value categories](https://en.cppreference.com/w/cpp/language/value_category), Reactor expressions have different characteristics that affect their semantics. Some of these characteristics are externally visible, while others are implementation details.

At its lowest level (namely the Nucleus interface), Reactor operates on `Value` objects. These correspond to [SSA values](https://en.wikipedia.org/wiki/Static_single_assignment_form). Their implementation is backend-specific.

The statement below declares a `Variable` of type `Int`. Like in C++, it is an lvalue. The `LValue<T>` class, which `Int` derives from, implements the lvalue characteristics of this variable. Logically, `LValue<T>` variables have a stack address, which is a `Value`. Reading the variable's *stored* value is a memory load operation, while writing it is a store operation.
```C++
Int x;
```
Unlike C++, however, `LValue<T>` does not represent other lvalues. It only represents stack variables. There is only one other Reactor type which corresponds with a C++ lvalue, the `Reference<T>`. References are the result of dereferencing a `Pointer<T>` or indexing an `Array<T>`, and can appear on the left-hand side of expressions:
```C++
*Pointer<Float>(destination + offset) = *Pointer<Float>(source + offset);
```
The above example also illustrates implicit uses of `RValue<T>` values. The `source + offset` expression computes an intermediate result which doesn't have a stack address. It doesn't have a name in this case, but unlike C++, Reactor allows creating named rvalues:
```C++
auto pointer = source + offset;
```
`pointer` will be of type RValue<Pointer<Byte>>. In contrast to C++, it does *not* allow assigning a new value to `pointer`. One has to explicitly declare it as a `Pointer<Byte>` for that to work.

Note that `RValue<T>` is very lightweight. It is essentially just a type-safe wrapper for `Value`. This makes it tempting to use it whenever possible. This might be OK for those familiar with functional programming, but can get confusing considering that Reactor is embedded within the imperative C++ language. Fortunately, extensive use of `RValue<T>` for performance reasons is not necessary, due to delayed materialization...

Materialization
---------------

As an optimization, Reactor does not immediately allocate stack space for variables. Logical store operations take an rvalue, while load operations produce an rvalue. As long as we're only reading and writing a variable within a single basic block, we only have to keep track of which rvalue was stored last.

This is accomplished within the `Variable` class, which `LValue<T>` derives from. In other words, a `Variable` can be in multiple states. It starts out without having a stack address, only storing a pointer to a `Value` for the last rvalue that was assigned. When a stack address is required, we 'materialize' the variable by allocating the stack space and storing the rvalue. This happens when a branch is encountered. To illustrate why this requires the variable to materialize, take the following the example:
```C++
Int x = a;

If(condition)
{
    x = b;
}

Int y = x;
```
The stored value of `x` is either that of `a` or `b` at the end of this sample, so it can't symbolically track just one of them. We need to actually store the value of `b` to the stack memory conditionally, and load its value from memory before storing it in `y` (note the latter starts out unmaterialized).

Immediate Values
----------------

While delayed materialization results in fewer or no load/store instructions being generated for many variables, there's still a cost to creating `Value` objects in the backend. Specifically, constant values are very common and require the backend to perform constant folding and propagation to reduce the number of actual instructions being emitted. Subzero doesn't support constant folding, and LLVM has a sizable overhead to representing constants and folding them. Therefore we perform some constant folding and constant propagation at the Reactor level.

This is implemented by having specializations of `RValue<T>` and subclasses of `LValue<T>` such as `Int` store an `Immediate<T>` value. The `Immediate<T>` class just provides storage for a constant value, but also keeps indicates whether or not the value is provided by Reactor code, or just an 'uninitialized' placeholder value.

When a `Variable` is materialized, we check whether the rvalue is an immediate, and promote it to a constant `Value` known by the backend, before storing it in stack memory, but only when it's an intialized one.

In summary, the humble Reactor `Variable` can be in four different states: lvalue, rvalue, immediate, or uninitialized. Externally it acts the same as a C++ local variable.