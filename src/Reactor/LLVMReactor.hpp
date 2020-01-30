// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef rr_LLVMReactor_hpp
#define rr_LLVMReactor_hpp

#include "Nucleus.hpp"

#ifdef _MSC_VER
__pragma(warning(push))
    __pragma(warning(disable : 4146))  // unary minus operator applied to unsigned type, result still unsigned
#endif

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

#ifdef _MSC_VER
    __pragma(warning(pop))
#endif

#include <memory>

        namespace llvm
{

	class Type;
	class Value;

}  // namespace llvm

namespace rr {

class Type;
class Value;

llvm::Type *T(Type *t);

inline Type *T(llvm::Type *t)
{
	return reinterpret_cast<Type *>(t);
}

inline llvm::Value *V(Value *t)
{
	return reinterpret_cast<llvm::Value *>(t);
}

inline Value *V(llvm::Value *t)
{
	return reinterpret_cast<Value *>(t);
}

// Emits a no-op instruction that will not be optimized away.
// Useful for emitting something that can have a source location without
// effect.
void Nop();

class Routine;
class Config;

// JITBuilder holds all the LLVM state for building routines.
class JITBuilder
{
public:
	JITBuilder(const rr::Config &config);

	void optimize(const rr::Config &cfg);

	std::shared_ptr<rr::Routine> acquireRoutine(llvm::Function **funcs, size_t count, const rr::Config &cfg);

	const Config config;
	llvm::LLVMContext context;
	std::unique_ptr<llvm::Module> module;
	std::unique_ptr<llvm::IRBuilder<>> builder;
	llvm::Function *function = nullptr;

	struct CoroutineState
	{
		llvm::Function *await = nullptr;
		llvm::Function *destroy = nullptr;
		llvm::Value *handle = nullptr;
		llvm::Value *id = nullptr;
		llvm::Value *promise = nullptr;
		llvm::Type *yieldType = nullptr;
		llvm::BasicBlock *entryBlock = nullptr;
		llvm::BasicBlock *suspendBlock = nullptr;
		llvm::BasicBlock *endBlock = nullptr;
		llvm::BasicBlock *destroyBlock = nullptr;
	};
	CoroutineState coroutine;

#ifdef ENABLE_RR_DEBUG_INFO
	std::unique_ptr<rr::DebugInfo> debugInfo;
#endif
};

}  // namespace rr

#endif  // rr_LLVMReactor_hpp
