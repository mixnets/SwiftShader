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

#ifndef yarn_util_hpp
#define yarn_util_hpp

#include "Scheduler.hpp"
#include "WaitGroup.hpp"

namespace yarn {

template <typename Function, typename COUNTER>
inline void parallelize(COUNTER numTotal, COUNTER numPerTask, const Function& f)
{
    auto numTasks = (numTotal + numPerTask - 1) / numPerTask;
    WaitGroup wg(numTasks - 1);
    for (unsigned int task = 1; task < numTasks; task++)
    {
        schedule([=] {
            auto first = task * numPerTask;
            auto count = std::min(first + numPerTask, numTotal) - first;
            f(task, first, count);
            wg.done();
        });
    }

    f(0, 0, std::min(numPerTask, numTotal));
    wg.wait();
}

} // namespace yarn

#endif // yarn_util_hpp
