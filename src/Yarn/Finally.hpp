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

class Finally
{
public:
    virtual ~Finally() = default;
};

template <typename F>
class FinallyImpl : public Finally
{
public:
    inline FinallyImpl(const F& func);
    inline FinallyImpl(F&& func);
    inline FinallyImpl(FinallyImpl<F>&& other);
    inline ~FinallyImpl();

private:
    FinallyImpl(const FinallyImpl<F>& other) = delete;
    FinallyImpl<F>& operator = (const FinallyImpl<F>& other) = delete;
    FinallyImpl<F>& operator = (FinallyImpl<F>&&) = delete;
    F func;
    bool valid = true;
};

template<typename F>
FinallyImpl<F>::FinallyImpl(const F& func) : func(func) {}

template<typename F>
FinallyImpl<F>::FinallyImpl(F&& func) : func(std::move(func)) {}

template<typename F>
FinallyImpl<F>::FinallyImpl(FinallyImpl<F>&& other) : func(std::move(other.func))
{
    other.valid = false;
}

template<typename F>
FinallyImpl<F>::~FinallyImpl()
{
    if (valid)
    {
        func();
    }
}

template<typename F>
inline FinallyImpl<F> make_finally(F&& f) { return FinallyImpl<F>(std::move(f)); }

template<typename F>
inline std::shared_ptr<Finally> make_shared_finally(F&& f) { return std::make_shared<FinallyImpl<F>>(std::move(f)); }

} // namespace yarn

#endif  // yarn_finally_hpp
