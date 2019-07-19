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

#ifndef yarn_storage_hpp
#define yarn_storage_hpp

#include "Debug.hpp"

namespace yarn {

template <typename T, int N>
class FixedSizeStorage
{
public:
    using Item = T;

    inline ~FixedSizeStorage();
    inline void put(const T& item);
    inline T take();
    inline size_t size() const;
    inline bool isFull() const;
private:
    using Storage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    Storage storage[N];
    size_t count = 0;
};

template <typename T, int N>
FixedSizeStorage<T, N>::~FixedSizeStorage()
{
    while (size > 0)
    {
        --size;
        auto item = reinterpret_cast<T*>(storage[size]);
        item->~T();
    }
}

template <typename T, int N>
void FixedSizeStorage<T, N>::put(const T& item)
{
    YARN_ASSERT(!isFull(), "FixedSizeStorage<T, N>::put() called when full");
    auto ptr = reinterpret_cast<T*>(storage[size]);
    new (ptr) T(item);
    ++size;
}

template <typename T, int N>
T FixedSizeStorage<T, N>::take()
{
    YARN_ASSERT(size > 0, "FixedSizeStorage<T, N>::take() called when empty");
    --size;
    auto item = reinterpret_cast<T*>(storage[size]);
    T out = std::move(*item);
    item->~T();
    return out;
}

template <typename T, int N>
inline size_t FixedSizeStorage<T, N>::size() const
{
    return size;
}

template <typename T, int N>
inline bool FixedSizeStorage<T, N>::isFull() const
{
    return size == N;
}

} // namespace yarn

#endif  // yarn_storage_hpp
