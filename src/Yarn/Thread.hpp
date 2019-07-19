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

#ifndef yarn_thread_hpp
#define yarn_thread_hpp

#include <bitset>

namespace yarn {

class Thread
{
public:
    using AffinityMask = std::bitset<1024>;

    static void setName(const char* fmt, ...);
    static void setAffinity(const AffinityMask& mask);
    static unsigned int getNumCPUs();
};

} // namespace yarn

#endif  // yarn_thread_hpp
