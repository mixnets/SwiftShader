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

#ifndef yarn_fiber_hpp
#define yarn_fiber_hpp

#include "Scheduler.hpp"

#include <functional>
#include <stddef.h>
#include <stdint.h>

namespace yarn {

class OSFiber;

class Fiber
{
public:
	~Fiber();

	void schedule();
    void yield();
	void switchTo(Fiber*);

    static Fiber* current();
	static Fiber* create(uint32_t id, size_t stackSize, const std::function<void()>& func);
	static Fiber* createFromCurrentThread(uint32_t id, size_t stackSize);

	OSFiber* const impl;
	uint32_t const id;

private:
	Fiber(OSFiber*, uint32_t id);
	Scheduler::Worker* const worker;
};

} // namespace yarn

#endif  // yarn_fiber_hpp
