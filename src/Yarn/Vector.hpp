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

#ifndef yarn_vector_hpp
#define yarn_vector_hpp

#include "Debug.hpp"

#include <atomic>
#include <mutex>

namespace yarn {

////////////////////////////////////////////////////////////////////////////////
// Vector<T, BASE_CAPACITY>
////////////////////////////////////////////////////////////////////////////////
template <typename T, int BASE_CAPACITY>
class Vector
{
public:
    inline Vector() = default;
    inline Vector(const Vector<T, BASE_CAPACITY>& other);
    inline Vector(Vector<T, BASE_CAPACITY>&& other);
    inline ~Vector();
    inline Vector<T, BASE_CAPACITY>& operator = (const Vector<T, BASE_CAPACITY>&);
    inline Vector<T, BASE_CAPACITY>& operator = (Vector<T, BASE_CAPACITY>&&);
    inline void push_back(const T& el);
    inline void pop_back();
    inline T& front();
    inline T& back();
    inline T* begin();
    inline T* end();
    inline size_t size() const;
private:
    inline void reserve(size_t n);
    inline void free();

    size_t count = 0;
    size_t capacity = BASE_CAPACITY;
    T buffer[BASE_CAPACITY];
    T* elements = buffer;
};

template <typename T, int BASE_CAPACITY>
Vector<T, BASE_CAPACITY>::Vector(const Vector<T, BASE_CAPACITY>& other)
{
    *this = other;
}

template <typename T, int BASE_CAPACITY>
Vector<T, BASE_CAPACITY>::Vector(Vector<T, BASE_CAPACITY>&& other)
{
    *this = std::move(other);
}

template <typename T, int BASE_CAPACITY>
Vector<T, BASE_CAPACITY>::~Vector()
{
    free();
}

template <typename T, int BASE_CAPACITY>
Vector<T, BASE_CAPACITY>& Vector<T, BASE_CAPACITY>::operator = (const Vector<T, BASE_CAPACITY>& other)
{
    free();
    reserve(other.capacity);
    count = other.count;
    for (int i = 0; i < count; i++)
    {
        new(&elements[i]) T(other.elements[i]);
    }
}

template <typename T, int BASE_CAPACITY>
Vector<T, BASE_CAPACITY>& Vector<T, BASE_CAPACITY>::operator = (Vector<T, BASE_CAPACITY>&& other)
{
    free();
    capacity = other.capacity;
    count = other.count;
    if (other.elements == other.buffer)
    {
        for (int i = 0; i < count; i++)
        {
            new(&elements[i]) T(other.elements[i]);
        }
    }
    else
    {
        elements = other.elements;
        other.elements = buffer;
        other.capacity = BASE_CAPACITY2;
    }
}

template <typename T, int BASE_CAPACITY>
void Vector<T, BASE_CAPACITY>::push_back(const T& el)
{
    reserve(count + 1);
    new (&elements[count]) T(el);
    count++;
}

template <typename T, int BASE_CAPACITY>
void Vector<T, BASE_CAPACITY>::pop_back()
{
    YARN_ASSERT(count > 0, "pop_back() called on empty Vector");
    count--;
    elements[count].~T();
}

template <typename T, int BASE_CAPACITY>
T& Vector<T, BASE_CAPACITY>::front()
{
    YARN_ASSERT(count > 0, "front() called on empty Vector");
    return elements[0];
}

template <typename T, int BASE_CAPACITY>
T& Vector<T, BASE_CAPACITY>::back()
{
    YARN_ASSERT(count > 0, "back() called on empty Vector");
    return elements[count - 1];
}

template <typename T, int BASE_CAPACITY>
T* Vector<T, BASE_CAPACITY>::begin()
{
    return elements;
}

template <typename T, int BASE_CAPACITY>
T* Vector<T, BASE_CAPACITY>::end()
{
    return elements + count;
}

template <typename T, int BASE_CAPACITY>
size_t Vector<T, BASE_CAPACITY>::size() const
{
    return count;
}

template <typename T, int BASE_CAPACITY>
void Vector<T, BASE_CAPACITY>::reserve(size_t n)
{
    if (n >= capacity)
    {
        capacity = std::max<size_t>(n * 2, 8);
        auto grown = reinterpret_cast<T*>(new T[capacity]);
        for (int i = 0; i < count; i++)
        {
            new(&grown[i]) T(std::move(elements[i]));
        }
        free();
        elements = grown;
    }
}

template <typename T, int BASE_CAPACITY>
void Vector<T, BASE_CAPACITY>::free()
{
    for (int i = 0; i < count; i++)
    {
        elements[i].~T();
    }

    if (elements != buffer)
    {
        delete []elements;
        elements = nullptr;
    }
}

} // namespace yarn

#endif  // yarn_vector_hpp
