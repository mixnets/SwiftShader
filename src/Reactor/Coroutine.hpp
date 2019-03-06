// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Reactor.hpp"

#include <memory>

#ifndef rr_ReactorCoroutine_hpp
#define rr_ReactorCoroutine_hpp

namespace rr
{
	template<typename T>
	struct ReactorToC;

	template<> struct ReactorToC<Int> { using type = int; };
	template<> struct ReactorToC<Float> { using type = float; };
	template<> struct ReactorToC<Byte> { using type = uint8_t; };

	template<typename T>
	struct ReactorToC<Pointer<T>> { using type = typename ReactorToC<T>::type*; };

	class StreamBase
	{
	protected:
		StreamBase(Routine *routine, Nucleus::CoroutineHandle handle)
			: routine(routine), handle(handle) {}

		~StreamBase()
		{
			auto pfn = (Nucleus::CoroutineTerminate*)routine->getEntry(Nucleus::CoroutineEntryTerminate);
			pfn(handle);
		}

		bool await(void* out)
		{
			auto pfn = (Nucleus::CoroutineAwait*)routine->getEntry(Nucleus::CoroutineEntryAwait);
			return pfn(handle, out);
		}

private:
		Routine *routine;
		Nucleus::CoroutineHandle handle;
	};

	template<typename T>
	class Stream : public StreamBase
	{
	public:
		using CType = typename ReactorToC<T>::type;
		inline Stream(Routine *routine, Nucleus::CoroutineHandle handle)
			: StreamBase(routine, handle) {}
		inline bool await(CType& out) { return StreamBase::await(&out); }
	};

	template<typename FunctionType>
	class Coroutine;

	template<typename Return, typename... Arguments>
	class Coroutine<Return(Arguments...)>
	{
	public:
		using CReturn = typename ReactorToC<Return>::type;

		Coroutine();

		virtual ~Coroutine();

		template<int index>
		Argument<typename std::tuple_element<index, std::tuple<Arguments...>>::type> Arg() const
		{
			Value *arg = Nucleus::getArgument(index);
			return Argument<typename std::tuple_element<index, std::tuple<Arguments...>>::type>(arg);
		}

		std::unique_ptr<Stream<Return>> operator()(typename ReactorToC<Arguments>::type...);

	protected:
		inline void finalize();

		Nucleus *core;
		Routine *routine;
		std::vector<Type*> arguments;
	};

	template<typename Return, typename... Arguments>
	Coroutine<Return(Arguments...)>::Coroutine() : routine{}
	{
		core = new Nucleus();

		Type *types[] = {Arguments::getType()...};
		for(Type *type : types)
		{
			if(type != Void::getType())
			{
				arguments.push_back(type);
			}
		}

		Nucleus::createCoroutine(Return::getType(), arguments);
	}

	template<typename Return, typename... Arguments>
	Coroutine<Return(Arguments...)>::~Coroutine()
	{
		if (core != nullptr) { delete core; }
		if (routine != nullptr) { delete routine; }
	}

	template<typename Return, typename... Arguments>
	void Coroutine<Return(Arguments...)>::finalize()
	{
		if (core != nullptr) {
			routine = core->acquireCoroutine("coroutine", true);
			delete core;
			core = nullptr;
		}
	}

	template<typename Return, typename... Arguments>
	std::unique_ptr<Stream<Return>>
	Coroutine<Return(Arguments...)>::operator()(typename ReactorToC<Arguments>::type... args)
	{
		finalize();

		using Sig = Nucleus::CoroutineBegin<typename ReactorToC<Arguments>::type...>;
		auto pfn = (Sig*)routine->getEntry(Nucleus::CoroutineEntryBegin);
		auto handle = pfn(args...);
		return std::unique_ptr<Stream<Return>>(new Stream<Return>(routine, handle));
	}

	void Yield(Value*);

	template<typename T>
	inline void Yield(RValue<T> val) { return Yield(val.value); }

	template<typename T>
	inline void Yield(Reference<T> val) { return Yield(RValue<T>(val)); }

} // namespace rr

#endif // rr_ReactorCoroutine_hpp