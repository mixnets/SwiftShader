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
		inline Stream(Routine *routine, Nucleus::CoroutineHandle handle)
			: StreamBase(routine, handle) {}
		inline bool await(T& out) { return StreamBase::await(&out); }
	};

	template<typename FunctionType>
	class Coroutine;

	template<typename Return, typename... Arguments>
	class Coroutine<Return(Arguments...)>
	{
	public:
		Coroutine();

		virtual ~Coroutine();

		template<int index>
		using CArgumentType = typename std::tuple_element<index, std::tuple<Arguments...>>::type;

		template<int index>
		using RArgumentType = CToReactor<CArgumentType<index>>;

		template<int index>
		Argument<RArgumentType<index>> Arg() const
		{
			Value *arg = Nucleus::getArgument(index);
			return Argument<RArgumentType<index>>(arg);
		}

		std::unique_ptr<Stream<Return>> operator()(Arguments...);

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

		Type *types[] = {CToReactor<Arguments>::getType()...};
		for(Type *type : types)
		{
			if(type != Void::getType())
			{
				arguments.push_back(type);
			}
		}

		Nucleus::createCoroutine(CToReactor<Return>::getType(), arguments);
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
	Coroutine<Return(Arguments...)>::operator()(Arguments... args)
	{
		finalize();

		using Sig = Nucleus::CoroutineBegin<Arguments...>;
		auto pfn = (Sig*)routine->getEntry(Nucleus::CoroutineEntryBegin);
		auto handle = pfn(args...);
		return std::unique_ptr<Stream<Return>>(new Stream<Return>(routine, handle));
	}

#ifdef Yield // Defined in WinBase.h
#undef Yield
#endif

	template<typename T>
	inline void Yield(RValue<T> val) { Nucleus::yield(val.value); }

	template<typename T>
	inline void Yield(Reference<T> val) { Yield(RValue<T>(val)); }

} // namespace rr

#endif // rr_ReactorCoroutine_hpp