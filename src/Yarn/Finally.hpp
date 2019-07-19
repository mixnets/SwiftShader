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

#ifndef yarn_finally_hpp
#define yarn_finally_hpp

#include <functional>
#include <memory>

namespace yarn {

template <typename F = std::function<void()>>
class Finally
{
public:
    inline Finally(const F& func);
    inline Finally(F&& func);
    inline Finally(Finally<F>&& other);
    inline ~Finally();

    inline void touch() {}
private:
    Finally(const Finally<F>& other) = delete;
    Finally<F>& operator = (const Finally<F>& other) = delete;
    Finally<F>& operator = (Finally<F>&&) = delete;
    F func;
    bool valid = true;
};

template<typename F>
Finally<F>::Finally(const F& func) : func(func) {}

template<typename F>
Finally<F>::Finally(F&& func) : func(std::move(func)) {}

template<typename F>
Finally<F>::Finally(Finally<F>&& other) : func(std::move(other.func))
{
    other.valid = false;
}

template<typename F>
Finally<F>::~Finally()
{
    if (valid)
    {
        func();
    }
}

template<typename F>
inline Finally<F> make_finally(F&& f) { return Finally<F>(std::move(f)); }

template<typename F>
inline std::shared_ptr<Finally<F>> make_shared_finally(F&& f) { return std::make_shared<Finally<F>>(std::move(f)); }

} // namespace yarn

#endif  // yarn_finally_hpp
