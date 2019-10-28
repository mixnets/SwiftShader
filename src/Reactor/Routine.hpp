// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#ifndef rr_Routine_hpp
#define rr_Routine_hpp

#include <vector>

namespace rr
{
	//TODO: move somewhere...
	// Identity class which is used to carry parameter packs.
	template<typename... Args>
	struct Identity {};

	template<typename T>
	struct FunctionTraits;

	// Matches function types
	template<typename R, typename ...Args> 
	struct FunctionTraits<R(Args...)>
	{
		typedef R ResultType;
		typedef Identity<Args...> ArgumentsPackType;
	};

	// Matches function pointer types
	template<typename R, typename ...Args> 
	struct FunctionTraits<R(*)(Args...)>
	{
		typedef R ResultType;
		typedef Identity<Args...> ArgumentsPackType;
	};



	//TODO: Signature.hpp

	namespace detail
	{
		// Primary template
		template <typename Return, typename... Arguments>
		struct CreateSignatureImpl;

		// For Reactor types (default)
		template <typename Return, typename... Arguments>
		struct CreateSignatureImpl
		{
			static auto Create()
			{
				auto signature = Signature(Return::getType(), { Arguments::getType() ... });
				return signature;
			}
		};

		// For C types - note we rely on the fact that Arguments is wrapped in an Identity<>
		// to differentiate from Reactor types
		template <typename Return, typename... Arguments>
		struct CreateSignatureImpl<Return, Identity<Arguments...>>
		{
			static auto Create() { return Signature(CToReactor<Return>::getType(), { CToReactor<Arguments>::getType() ... }); }
		};
	}


	class Type;

	class Signature
	{
	public:
		Signature() = default;
		Signature(Type* returnType, std::vector<Type*> argTypes);

		template <typename Return, typename... Arguments>
		static Signature Create()
		{
			return detail::CreateSignatureImpl<Return, Arguments...>::Create();
		}

		bool operator==(const Signature& rhs)
		{
			return returnType == rhs.returnType && argTypes == rhs.argTypes;
		}

		bool operator!=(const Signature& rhs)
		{
			return !(*this == rhs);
		}

		void assertEquals(const Signature& rhs);

	private:
		Type* returnType{};
		std::vector<Type*> argTypes{};
	};








	class Routine
	{
	public:
		Routine() = default;
		virtual ~Routine() = default;

		// Returns function pointer of the specified type.
		// FunctionType can be either a function pointer type, e.g. (int)(*)(float,bool),
		// or a function type, e.g. (int)(float,bool). The returned value is a function
		// pointer.
		template <typename FunctionType>
		auto getEntry(int index = 0)
		{
			using FunctionPtrType = std::conditional_t<std::is_pointer_v<FunctionType>, FunctionType, std::add_pointer_t<FunctionType>>;

			validateSignature<FunctionPtrType>();
			return reinterpret_cast<FunctionPtrType>(getEntry(index));
		}

		// Same as getEntry, but does not validate signatures
		template <typename FunctionType>
		auto getEntryUnchecked(int index = 0)
		{
			using FunctionPtrType = std::conditional_t<std::is_pointer_v<FunctionType>, FunctionType, std::add_pointer_t<FunctionType>>;
			return reinterpret_cast<FunctionPtrType>(getEntry(index));
		}

	private:
		virtual const void *getEntry(int index = 0) = 0;

		template<typename>
		friend class Function; // Only Function can call setSignature

		void setSignature(Signature&& signature)
		{
			this->signature = std::move(signature);
		}

		template <typename FunctionType>
		void validateSignature()
		{
			using FuncTraits = FunctionTraits<FunctionType>;
			auto castSignature = Signature::Create<FuncTraits::ResultType, FuncTraits::ArgumentsPackType>();
			//if (castSignature != signature)
			//{
			//	int a = 0;
			//	a = a;
			//}
			castSignature.assertEquals(signature);
		}

		Signature signature;
	};
}

#endif   // rr_Routine_hpp
