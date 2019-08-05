// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "OSFiber_asm_x64.h"

#include <assert.h>

#include <functional>
#include <memory>

extern "C"
{

extern void yarn_fiber_set_target(yarn_fiber_context*, void* stack, uint32_t stack_size, void(*target)(void*), void* arg);
extern void yarn_fiber_swap(yarn_fiber_context* from, const yarn_fiber_context* to);

} // extern "C"

namespace yarn {

class OSFiber
{
public:
    static inline OSFiber* createFiberFromCurrentThread(size_t stackSize);
    static inline OSFiber* createFiber(size_t stackSize, const std::function<void()>& func);
    inline void switchTo(OSFiber*);

private:
    static void run(OSFiber* self);

    yarn_fiber_context context;
    std::function<void()> target;
    std::unique_ptr<uint8_t[]> stack;
    uint32_t stack_size;
};

OSFiber* OSFiber::createFiberFromCurrentThread(size_t stackSize)
{
    auto out = new OSFiber();
    out->context = {};
    out->stack = std::unique_ptr<uint8_t[]>(new uint8_t[stackSize]);
    out->stack_size = stackSize;
    return out;
}

OSFiber* OSFiber::createFiber(size_t stackSize, const std::function<void()>& func)
{
    auto out = new OSFiber();
    out->context = {};
    out->target = func;
    out->stack = std::unique_ptr<uint8_t[]>(new uint8_t[stackSize]);
    out->stack_size = stackSize;
    yarn_fiber_set_target(&out->context, out->stack.get(), stackSize, reinterpret_cast<void (*)(void*)>(&OSFiber::run), out);
    return out;
}

void OSFiber::run(OSFiber* self)
{
    std::function<void()> func;
    std::swap(func, self->target);
    func();
}

void OSFiber::switchTo(OSFiber* fiber)
{
    yarn_fiber_swap(&context, &fiber->context);
}

}  // namespace yarn
