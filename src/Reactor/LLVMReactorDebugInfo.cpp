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

#include "LLVMReactorDebugInfo.hpp"

#include "Reactor.hpp"
#include "LLVMReactor.hpp"

#ifdef ENABLE_RR_DEBUG_INFO

#if REACTOR_LLVM_VERSION < 7
#error "ENABLE_RR_DEBUG_INFO can currently only be used with LLVM 7+"
#endif

#include "backtrace.h"

#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IRBuilder.h"

#include <cctype>
#include <fstream>
#include <string>
#include <sstream>

namespace
{
	std::pair<llvm::StringRef, llvm::StringRef> splitPath(const char* path)
	{
		return llvm::StringRef(path).rsplit('/');
	}
} // Anonymous namespaces

namespace rr
{
	DebugInfo::DebugInfo(
			llvm::IRBuilder<> *builder,
			llvm::LLVMContext *context,
			llvm::Module *module,
			llvm::Function *function)
		: builder(builder), context(context), module(module), function(function)
	{
		using namespace ::llvm;

		auto fileAndDir = splitPath(__FILE__);
		diBuilder = new llvm::DIBuilder(*module);
		diCU = diBuilder->createCompileUnit(
			::llvm::dwarf::DW_LANG_C,
			diBuilder->createFile(fileAndDir.first, fileAndDir.second),
			"Reactor",
			0, "", 0);

		diScope = diCU;
		jitEventListener = llvm::JITEventListener::createGDBRegistrationListener();
		registerBasicTypes();

		SmallVector<Metadata *, 8> EltTys;
		auto funcTy = diBuilder->createSubroutineType(diBuilder->getOrCreateTypeArray(EltTys));

		auto file = getOrCreateFile(__FILE__);
		auto sp = diBuilder->createFunction(
					file, // scope
					"ReactorFunction", // function name
					StringRef(), // linkage
					file, // file
					__LINE__, // line
					funcTy,
					false, // internal linkage
					true, // definition
					__LINE__, // scope line
					DINode::FlagPrototyped,
					false);
		diFunction = sp;
		diScope = sp;
		function->setSubprogram(sp);
		builder->SetCurrentDebugLocation(DebugLoc::get(__LINE__, 1, sp));
	}

	void DebugInfo::Finalize()
	{
		diBuilder->finalize();
	}

	void DebugInfo::EmitLocation()
	{
		emitLocation(getCalleeLocation());
	}

	void DebugInfo::EmitVariable(Value *value)
	{
		auto loc = getCalleeLocation();
		auto tokens = getOrParseFileTokens(loc.file.c_str());
		auto tokIt = tokens->find(loc.line);
		if (tokIt != tokens->end())
		{
			auto const &name = tokIt->second;
			emitValue(value, name.c_str(), loc);
		}
	}

	void DebugInfo::emitLocation(Location const &loc)
	{
		auto file = getOrCreateFile(loc.file.c_str());

		static std::unordered_map<std::string, ::llvm::DIScope*> scopes;

		auto key = std::string(loc.function) + ":" + loc.function;
		auto it = scopes.find(key);

		if (it == scopes.end())
		{
			diScope = diBuilder->createLexicalBlock(diFunction, file, loc.line, 0);
			scopes.emplace(key, diScope);
		}
		else
		{
			diScope = it->second;
		}

		builder->SetCurrentDebugLocation(::llvm::DebugLoc::get(loc.line, 0, diScope));
	}

	void DebugInfo::emitValue(Value* value, const char* name, Location const& loc)
	{
		auto file = getOrCreateFile(loc.file.c_str());
		auto val = V(value);
		auto dbgTy = getOrCreateType(val->getType());

		val->setName(name);

		auto *dbgVar = diBuilder->createAutoVariable(
			diScope, name, file, loc.line, dbgTy
		);

		auto dbgLoc = ::llvm::DebugLoc::get(loc.line, 0, diScope);
		if (::llvm::isa<::llvm::AllocaInst>(val))
		{
			diBuilder->insertDeclare(
				val, dbgVar, diBuilder->createExpression(), dbgLoc,
				builder->GetInsertBlock());
		}
		else
		{
			diBuilder->insertDbgValueIntrinsic(
				val, dbgVar, diBuilder->createExpression(), dbgLoc,
				builder->GetInsertBlock());
		}
	}

	void DebugInfo::NotifyObjectEmitted(const llvm::object::ObjectFile &Obj, const llvm::LoadedObjectInfo &L)
	{
		jitEventListener->NotifyObjectEmitted(Obj, static_cast<const llvm::RuntimeDyld::LoadedObjectInfo&>(L));
	}

	void DebugInfo::NotifyFreeingObject(const llvm::object::ObjectFile &Obj)
	{
		jitEventListener->NotifyFreeingObject(Obj);
	}

	void DebugInfo::registerBasicTypes()
	{
		using namespace rr;
		using namespace llvm;

		auto vec4 = diBuilder->getOrCreateArray(diBuilder->getOrCreateSubrange(0, 4));

		diTypes.emplace(T(Bool::getType()), diBuilder->createBasicType("Bool", sizeof(bool), dwarf::DW_ATE_boolean));
		diTypes.emplace(T(Byte::getType()), diBuilder->createBasicType("Byte", 8, dwarf::DW_ATE_unsigned_char));
		diTypes.emplace(T(SByte::getType()), diBuilder->createBasicType("SByte", 8, dwarf::DW_ATE_signed_char));
		diTypes.emplace(T(Short::getType()), diBuilder->createBasicType("Short", 16, dwarf::DW_ATE_signed));
		diTypes.emplace(T(UShort::getType()), diBuilder->createBasicType("UShort", 16, dwarf::DW_ATE_unsigned));
		diTypes.emplace(T(Int::getType()), diBuilder->createBasicType("Int", 32, dwarf::DW_ATE_signed));
		diTypes.emplace(T(UInt::getType()), diBuilder->createBasicType("UInt", 32, dwarf::DW_ATE_unsigned));
		diTypes.emplace(T(Long::getType()), diBuilder->createBasicType("Long", 64, dwarf::DW_ATE_signed));
		diTypes.emplace(T(Half::getType()), diBuilder->createBasicType("Half", 16, dwarf::DW_ATE_float));
		diTypes.emplace(T(Float::getType()), diBuilder->createBasicType("Float", 32, dwarf::DW_ATE_float));
//		diTypes.emplace(T(Byte4::getType()), diBuilder->createVectorType(4, 128, diTypes[T(Byte::getType())], {}));
//		diTypes.emplace(T(SByte4::getType()), diBuilder->createVectorType(4, 128, diTypes[T(SByte::getType())], {}));
//		diTypes.emplace(T(Byte8::getType()), diBuilder->createVectorType(8, 128, diTypes[T(Byte::getType())], {}));
//		diTypes.emplace(T(SByte8::getType()), diBuilder->createVectorType(8, 128, diTypes[T(SByte::getType())], {}));
//		diTypes.emplace(T(Byte16::getType()), diBuilder->createVectorType(16, 128, diTypes[T(Byte::getType())], {}));
//		diTypes.emplace(T(SByte16::getType()), diBuilder->createVectorType(16, 128, diTypes[T(SByte::getType())], {}));
//		diTypes.emplace(T(Short2::getType()), diBuilder->createVectorType(2, 128, diTypes[T(Short::getType())], {}));
//		diTypes.emplace(T(UShort2::getType()), diBuilder->createVectorType(2, 128, diTypes[T(UShort::getType())], {}));
//		diTypes.emplace(T(Short4::getType()), diBuilder->createVectorType(4, 128, diTypes[T(Short::getType())], {}));
//		diTypes.emplace(T(UShort4::getType()), diBuilder->createVectorType(4, 128, diTypes[T(UShort::getType())], {}));
//		diTypes.emplace(T(Short8::getType()), diBuilder->createVectorType(8, 128, diTypes[T(Short::getType())], {}));
//		diTypes.emplace(T(UShort8::getType()), diBuilder->createVectorType(8, 128, diTypes[T(UShort::getType())], {}));
//		diTypes.emplace(T(Int2::getType()), diBuilder->createVectorType(2, 128, diTypes[T(Int::getType())], {}));
//		diTypes.emplace(T(UInt2::getType()), diBuilder->createVectorType(2, 128, diTypes[T(UInt::getType())], {}));
		diTypes.emplace(T(Int4::getType()), diBuilder->createVectorType(128, 128, diTypes[T(Int::getType())], {vec4}));
		diTypes.emplace(T(UInt4::getType()), diBuilder->createVectorType(128, 128, diTypes[T(UInt::getType())], {vec4}));
//		diTypes.emplace(T(Float2::getType()), diBuilder->createVectorType(2, 128, diTypes[T(Float::getType())], {}));
		diTypes.emplace(T(Float4::getType()), diBuilder->createVectorType(128, 128, diTypes[T(Float::getType())], {vec4}));
	}

	DebugInfo::Location DebugInfo::getCalleeLocation() const
	{
		struct callbacks
		{
			static void onError(void *data, const char *msg, int errnum)
			{
				fprintf(stderr, "BACKTRACE ERROR %d: %s\n", errnum, msg);
			}

			static int onPCInfo(void *data, uintptr_t pc, const char *file, int line, const char *function)
			{
				if (file == nullptr) { return 0; }

				auto const &fileSR = ::llvm::StringRef(file);
				if (fileSR.endswith("ReactorDebugInfo.cpp") ||
					fileSR.endswith("Reactor.cpp") ||
					fileSR.endswith("Reactor.hpp"))
				{
					return 0;
				}

				auto cb = reinterpret_cast<callbacks*>(data);
				cb->location.file = file;
				cb->location.line = line;
				cb->location.function = function;
				return 1;
			}

			Location location;
		};

		callbacks callbacks;
		static auto state = backtrace_create_state(nullptr, 0, &callbacks::onError, nullptr);
		backtrace_full(state, 1, &callbacks::onPCInfo, &callbacks::onError, &callbacks);
		return callbacks.location;
	}

	::llvm::DIType *DebugInfo::getOrCreateType(::llvm::Type* type)
	{
		auto it = diTypes.find(type);
		if (it != diTypes.end()) { return it->second; }

		if(type->isPointerTy())
		{
			auto dbgTy = diBuilder->createPointerType(
				getOrCreateType(type->getPointerElementType()),
				sizeof(void*)*8, alignof(void*)*8);
			diTypes.emplace(type, dbgTy);
			return dbgTy;
		}
		assert(false);
	}

	llvm::DIFile *DebugInfo::getOrCreateFile(const char* path)
	{
		auto it = diFiles.find(path);
		if (it != diFiles.end()) { return it->second; }
		auto dirAndName = splitPath(path);
		auto file = diBuilder->createFile(dirAndName.second, dirAndName.first);
		diFiles.emplace(path, file);
		return file;
	}

	DebugInfo::LineTokens const *DebugInfo::getOrParseFileTokens(const char* path)
	{
		auto it = fileTokens.find(path);
		if (it != fileTokens.end())
		{
			return it->second.get();
		}

		auto tokens = std::unique_ptr<LineTokens>(new LineTokens());
		std::stringstream tok;
		auto file = std::unique_ptr<FILE, decltype(&fclose)>(fopen(path, "rb"), &fclose);
		unsigned int line = 1;
		uint8_t p = 0;
		uint8_t c;
		while (fread(&c, 1, 1, file.get()) != 0)
		{
			switch (c)
			{
			case '\t': case ' ': case '<': case '>':
				break; // skip
			case '\n':
				line++;
				break;
			case '=':
			{
				auto s = tok.str();
				if (s.length() > 0)
				{
					(*tokens)[line] = s;
				}
				break;
			}
			default:
				if (!isalnum(p))
				{
					std::stringstream temp;
					tok.swap(temp);
				}
				if (isalnum(c))
				{
					tok << c;
				}
			}
			p = c;
		}

		auto out = tokens.get();
		fileTokens.emplace(path, std::move(tokens));
		return out;
	}

} // namespace rr

#endif // ENABLE_RR_DEBUG_INFO
