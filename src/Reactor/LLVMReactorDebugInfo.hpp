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

#ifndef rr_LLVMReactorDebugInfo_hpp
#define rr_LLVMReactorDebugInfo_hpp

#include <unordered_map>
#include <memory>

// Forward declarations
namespace llvm
{
	class ConstantFolder;
	class DIBuilder;
	class DICompileUnit;
	class DIFile;
	class DIScope;
	class DISubprogram;
	class DIType;
	class Function;
	class IRBuilderDefaultInserter;
	class JITEventListener;
	class LLVMContext;
	class LoadedObjectInfo;
	class Module;
	class Type;

	namespace object
	{
		class ObjectFile;
	}

	template <typename T, typename Inserter> class IRBuilder;
} // namespace llvm

namespace rr
{
	class Type;
	class Value;

	class DebugInfo
	{
	public:
		using IRBuilder = llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>;

		DebugInfo(IRBuilder *builder,
				llvm::LLVMContext *context,
				llvm::Module *module,
				llvm::Function *function);

		void Finalize();

		void EmitLocation();
		void EmitVariable(Value *value);

		void NotifyObjectEmitted(const llvm::object::ObjectFile &Obj, const llvm::LoadedObjectInfo &L);
		void NotifyFreeingObject(const llvm::object::ObjectFile &Obj);

	private:
		using LineTokens = std::unordered_map<unsigned int, std::string>;

		struct Location
		{
			std::string function;
			std::string file;
			unsigned int line;
		};

		void registerBasicTypes();
		void emitLocation(Location const &loc);
		void emitValue(Value* value, const char* name, Location const& loc);
		Location getCalleeLocation() const;
		llvm::DIType *getOrCreateType(::llvm::Type* type);
		llvm::DIFile *getOrCreateFile(const char* path);
		LineTokens const *getOrParseFileTokens(const char* path);

		IRBuilder *builder;
		llvm::LLVMContext *context;
		llvm::Module *module;
		llvm::Function *function;

		llvm::DIBuilder *diBuilder;
		llvm::DICompileUnit *diCU;
		llvm::DISubprogram *diFunction;
		llvm::DIScope *diScope;
		std::unordered_map<std::string, llvm::DIFile*> diFiles;
		std::unordered_map<::llvm::Type*, llvm::DIType*> diTypes;
		std::unordered_map<std::string, std::unique_ptr<LineTokens>> fileTokens;
		llvm::JITEventListener *jitEventListener;
	};

} // namespace rr

#endif // rr_LLVMReactorDebugInfo_hpp
