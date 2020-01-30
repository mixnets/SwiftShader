// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "LLVMReactor.hpp"

#include "Debug.hpp"
#include "ExecutableMemory.hpp"
#include "Routine.hpp"

#if defined(__clang__)
// LLVM has occurrences of the extra-semi warning in its headers, which will be
// treated as an error in SwiftShader targets.
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wextra-semi"
#endif  // defined(__clang__)

#ifdef _MSC_VER
__pragma(warning(push))
    __pragma(warning(disable : 4146))  // unary minus operator applied to unsigned type, result still unsigned
#endif

#include "llvm/Analysis/LoopPass.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Intrinsics.h"
#if LLVM_VERSION_MAJOR >= 8
#	include "llvm/IR/IntrinsicsX86.h"
#endif
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Coroutines.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

#if defined(__clang__)
#	pragma clang diagnostic pop
#endif  // defined(__clang__)

#ifdef _MSC_VER
    __pragma(warning(pop))
#endif

#include <unordered_map>

        namespace rr
{

	void *resolveExternalSymbol(const char *);

}  // namespace rr

namespace {

// Default configuration settings. Must be accessed under mutex lock.
std::mutex defaultConfigLock;
rr::Config &defaultConfig()
{
	// This uses a static in a function to avoid the cost of a global static
	// initializer. See http://neugierig.org/software/chromium/notes/2011/08/static-initializers.html
	static rr::Config config = rr::Config::Edit()
	                               .add(rr::Optimization::Pass::ScalarReplAggregates)
	                               .add(rr::Optimization::Pass::InstructionCombining)
	                               .apply({});
	return config;
}

// Cache provides a simple, thread-safe key-value store.
template<typename KEY, typename VALUE>
class Cache
{
public:
	Cache() = default;
	Cache(const Cache &other);
	VALUE getOrCreate(KEY key, std::function<VALUE()> create);

private:
	mutable std::mutex mutex;  // mutable required for copy constructor.
	std::unordered_map<KEY, VALUE> map;
};

template<typename KEY, typename VALUE>
Cache<KEY, VALUE>::Cache(const Cache &other)
{
	std::unique_lock<std::mutex> lock(other.mutex);
	map = other.map;
}

template<typename KEY, typename VALUE>
VALUE Cache<KEY, VALUE>::getOrCreate(KEY key, std::function<VALUE()> create)
{
	std::unique_lock<std::mutex> lock(mutex);
	auto it = map.find(key);
	if(it != map.end())
	{
		return it->second;
	}
	auto value = create();
	map.emplace(key, value);
	return value;
}

// JITGlobals is a singleton that holds all the immutable machine specific
// information for the host device.
class JITGlobals
{
public:
	using TargetMachineSPtr = std::shared_ptr<llvm::TargetMachine>;

	static JITGlobals *get();

	const std::string mcpu;
	const std::vector<std::string> mattrs;
	const char *const march;
	const llvm::TargetOptions targetOptions;
	const llvm::DataLayout dataLayout;

	TargetMachineSPtr getTargetMachine(rr::Optimization::Level optlevel);

private:
	static JITGlobals create();
	static llvm::CodeGenOpt::Level toLLVM(rr::Optimization::Level level);
	JITGlobals(const char *mcpu,
	           const std::vector<std::string> &mattrs,
	           const char *march,
	           const llvm::TargetOptions &targetOptions,
	           const llvm::DataLayout &dataLayout);
	JITGlobals(const JITGlobals &) = default;

	Cache<rr::Optimization::Level, TargetMachineSPtr> targetMachines;
};

JITGlobals *JITGlobals::get()
{
	static JITGlobals instance = create();
	return &instance;
}

JITGlobals::TargetMachineSPtr JITGlobals::getTargetMachine(rr::Optimization::Level optlevel)
{
#ifdef ENABLE_RR_DEBUG_INFO
	auto llvmOptLevel = toLLVM(rr::Optimization::Level::None);
#else   // ENABLE_RR_DEBUG_INFO
	auto llvmOptLevel = toLLVM(optlevel);
#endif  // ENABLE_RR_DEBUG_INFO

	return targetMachines.getOrCreate(optlevel, [&]() {
		return TargetMachineSPtr(llvm::EngineBuilder()
		                             .setOptLevel(llvmOptLevel)
		                             .setMCPU(mcpu)
		                             .setMArch(march)
		                             .setMAttrs(mattrs)
		                             .setTargetOptions(targetOptions)
		                             .selectTarget());
	});
}

JITGlobals JITGlobals::create()
{
	struct LLVMInitializer
	{
		LLVMInitializer()
		{
			llvm::InitializeNativeTarget();
			llvm::InitializeNativeTargetAsmPrinter();
			llvm::InitializeNativeTargetAsmParser();
		}
	};
	static LLVMInitializer initializeLLVM;

	auto mcpu = llvm::sys::getHostCPUName();

	llvm::StringMap<bool> features;
	bool ok = llvm::sys::getHostCPUFeatures(features);

#if defined(__i386__) || defined(__x86_64__) || \
    (defined(__linux__) && (defined(__arm__) || defined(__aarch64__)))
	ASSERT_MSG(ok, "llvm::sys::getHostCPUFeatures returned false");
#else
	(void)ok;  // getHostCPUFeatures always returns false on other platforms
#endif

	std::vector<std::string> mattrs;
	for(auto &feature : features)
	{
		if(feature.second) { mattrs.push_back(feature.first()); }
	}

	const char *march = nullptr;
#if defined(__x86_64__)
	march = "x86-64";
#elif defined(__i386__)
	march = "x86";
#elif defined(__aarch64__)
	march = "arm64";
#elif defined(__arm__)
	march = "arm";
#elif defined(__mips__)
#	if defined(__mips64)
	march = "mips64el";
#	else
	march = "mipsel";
#	endif
#elif defined(__powerpc64__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	march = "ppc64le";
#else
#	error "unknown architecture"
#endif

	llvm::TargetOptions targetOptions;
	targetOptions.UnsafeFPMath = false;

	auto targetMachine = std::unique_ptr<llvm::TargetMachine>(
	    llvm::EngineBuilder()
	        .setOptLevel(llvm::CodeGenOpt::None)
	        .setMCPU(mcpu)
	        .setMArch(march)
	        .setMAttrs(mattrs)
	        .setTargetOptions(targetOptions)
	        .selectTarget());

	auto dataLayout = targetMachine->createDataLayout();

	return JITGlobals(mcpu.data(), mattrs, march, targetOptions, dataLayout);
}

llvm::CodeGenOpt::Level JITGlobals::toLLVM(rr::Optimization::Level level)
{
	switch(level)
	{
		case rr::Optimization::Level::None: return ::llvm::CodeGenOpt::None;
		case rr::Optimization::Level::Less: return ::llvm::CodeGenOpt::Less;
		case rr::Optimization::Level::Default: return ::llvm::CodeGenOpt::Default;
		case rr::Optimization::Level::Aggressive: return ::llvm::CodeGenOpt::Aggressive;
		default: UNREACHABLE("Unknown Optimization Level %d", int(level));
	}
	return ::llvm::CodeGenOpt::Default;
}

JITGlobals::JITGlobals(const char *mcpu,
                       const std::vector<std::string> &mattrs,
                       const char *march,
                       const llvm::TargetOptions &targetOptions,
                       const llvm::DataLayout &dataLayout)
    : mcpu(mcpu)
    , mattrs(mattrs)
    , march(march)
    , targetOptions(targetOptions)
    , dataLayout(dataLayout)
{
}

class MemoryMapper : public llvm::SectionMemoryManager::MemoryMapper
{
public:
	MemoryMapper() {}
	~MemoryMapper() final {}

	llvm::sys::MemoryBlock allocateMappedMemory(
	    llvm::SectionMemoryManager::AllocationPurpose purpose,
	    size_t numBytes, const llvm::sys::MemoryBlock *const nearBlock,
	    unsigned flags, std::error_code &errorCode) final
	{
		errorCode = std::error_code();

		// Round up numBytes to page size.
		size_t pageSize = rr::memoryPageSize();
		numBytes = (numBytes + pageSize - 1) & ~(pageSize - 1);

		bool need_exec =
		    purpose == llvm::SectionMemoryManager::AllocationPurpose::Code;
		void *addr = rr::allocateMemoryPages(
		    numBytes, flagsToPermissions(flags), need_exec);
		if(!addr)
			return llvm::sys::MemoryBlock();
		return llvm::sys::MemoryBlock(addr, numBytes);
	}

	std::error_code protectMappedMemory(const llvm::sys::MemoryBlock &block,
	                                    unsigned flags)
	{
		// Round down base address to align with a page boundary. This matches
		// DefaultMMapper behavior.
		void *addr = block.base();
#if LLVM_VERSION_MAJOR >= 8
		size_t size = block.allocatedSize();
#else
		size_t size = block.size();
#endif
		size_t pageSize = rr::memoryPageSize();
		addr = reinterpret_cast<void *>(
		    reinterpret_cast<uintptr_t>(addr) & ~(pageSize - 1));
		size += reinterpret_cast<uintptr_t>(block.base()) -
		        reinterpret_cast<uintptr_t>(addr);

		rr::protectMemoryPages(addr, size, flagsToPermissions(flags));
		return std::error_code();
	}

	std::error_code releaseMappedMemory(llvm::sys::MemoryBlock &block)
	{
#if LLVM_VERSION_MAJOR >= 8
		size_t size = block.allocatedSize();
#else
		size_t size = block.size();
#endif

		rr::deallocateMemoryPages(block.base(), size);
		return std::error_code();
	}

private:
	int flagsToPermissions(unsigned flags)
	{
		int result = 0;
		if(flags & llvm::sys::Memory::MF_READ)
		{
			result |= rr::PERMISSION_READ;
		}
		if(flags & llvm::sys::Memory::MF_WRITE)
		{
			result |= rr::PERMISSION_WRITE;
		}
		if(flags & llvm::sys::Memory::MF_EXEC)
		{
			result |= rr::PERMISSION_EXECUTE;
		}
		return result;
	}
};

// JITRoutine is a rr::Routine that holds a LLVM JIT session, compiler and
// object layer as each routine may require different target machine
// settings and no Reactor routine directly links against another.
class JITRoutine : public rr::Routine
{
#if LLVM_VERSION_MAJOR >= 8
	using ObjLayer = llvm::orc::LegacyRTDyldObjectLinkingLayer;
	using CompileLayer = llvm::orc::LegacyIRCompileLayer<ObjLayer, llvm::orc::SimpleCompiler>;
#else
	using ObjLayer = llvm::orc::RTDyldObjectLinkingLayer;
	using CompileLayer = llvm::orc::IRCompileLayer<ObjLayer, llvm::orc::SimpleCompiler>;
#endif

public:
	JITRoutine(
	    std::unique_ptr<llvm::Module> module,
	    llvm::Function **funcs,
	    size_t count,
	    const rr::Config &config)
	    : resolver(createLegacyLookupResolver(
	          session,
	          [&](const std::string &name) {
		          void *func = rr::resolveExternalSymbol(name.c_str());
		          if(func != nullptr)
		          {
			          return llvm::JITSymbol(
			              reinterpret_cast<uintptr_t>(func), llvm::JITSymbolFlags::Absolute);
		          }
		          return objLayer.findSymbol(name, true);
	          },
	          [](llvm::Error err) {
		          if(err)
		          {
			          // TODO: Log the symbol resolution errors.
			          return;
		          }
	          }))
	    , targetMachine(JITGlobals::get()->getTargetMachine(config.getOptimization().getLevel()))
	    , compileLayer(objLayer, llvm::orc::SimpleCompiler(*targetMachine))
	    , objLayer(
	          session,
	          [this](llvm::orc::VModuleKey) {
		          return ObjLayer::Resources{ std::make_shared<llvm::SectionMemoryManager>(&memoryMapper), resolver };
	          },
	          ObjLayer::NotifyLoadedFtor(),
	          [](llvm::orc::VModuleKey, const llvm::object::ObjectFile &Obj, const llvm::RuntimeDyld::LoadedObjectInfo &L) {
#ifdef ENABLE_RR_DEBUG_INFO
		          rr::DebugInfo::NotifyObjectEmitted(Obj, L);
#endif  // ENABLE_RR_DEBUG_INFO
	          },
	          [](llvm::orc::VModuleKey, const llvm::object::ObjectFile &Obj) {
#ifdef ENABLE_RR_DEBUG_INFO
		          rr::DebugInfo::NotifyFreeingObject(Obj);
#endif  // ENABLE_RR_DEBUG_INFO
	          })
	    , addresses(count)
	{
		std::vector<std::string> mangledNames(count);
		for(size_t i = 0; i < count; i++)
		{
			auto func = funcs[i];
			static size_t numEmittedFunctions = 0;
			std::string name = "f" + llvm::Twine(numEmittedFunctions++).str();
			func->setName(name);
			func->setLinkage(llvm::GlobalValue::ExternalLinkage);
			func->setDoesNotThrow();

			llvm::raw_string_ostream mangledNameStream(mangledNames[i]);
			llvm::Mangler::getNameWithPrefix(mangledNameStream, name, JITGlobals::get()->dataLayout);
		}

		auto moduleKey = session.allocateVModule();

		// Once the module is passed to the compileLayer, the
		// llvm::Functions are freed. Make sure funcs are not referenced
		// after this point.
		funcs = nullptr;

		llvm::cantFail(compileLayer.addModule(moduleKey, std::move(module)));

		// Resolve the function addresses.
		for(size_t i = 0; i < count; i++)
		{
			auto symbol = compileLayer.findSymbolIn(moduleKey, mangledNames[i], false);
			if(auto address = symbol.getAddress())
			{
				addresses[i] = reinterpret_cast<void *>(static_cast<intptr_t>(address.get()));
			}
		}
	}

	const void *getEntry(int index) const override
	{
		return addresses[index];
	}

private:
	std::shared_ptr<llvm::orc::SymbolResolver> resolver;
	std::shared_ptr<llvm::TargetMachine> targetMachine;
	llvm::orc::ExecutionSession session;
	CompileLayer compileLayer;
	MemoryMapper memoryMapper;
	ObjLayer objLayer;
	std::vector<const void *> addresses;
};

}  // anonymous namespace

namespace rr {

JITBuilder::JITBuilder(const rr::Config &config)
    : config(config)
    , module(new llvm::Module("", context))
    , builder(new llvm::IRBuilder<>(context))
{
	module->setDataLayout(JITGlobals::get()->dataLayout);
}

void JITBuilder::optimize(const rr::Config &cfg)
{

#ifdef ENABLE_RR_DEBUG_INFO
	if(debugInfo != nullptr)
	{
		return;  // Don't optimize if we're generating debug info.
	}
#endif  // ENABLE_RR_DEBUG_INFO

	std::unique_ptr<llvm::legacy::PassManager> passManager(
	    new llvm::legacy::PassManager());

	for(auto pass : cfg.getOptimization().getPasses())
	{
		switch(pass)
		{
			case rr::Optimization::Pass::Disabled: break;
			case rr::Optimization::Pass::CFGSimplification: passManager->add(llvm::createCFGSimplificationPass()); break;
			case rr::Optimization::Pass::LICM: passManager->add(llvm::createLICMPass()); break;
			case rr::Optimization::Pass::AggressiveDCE: passManager->add(llvm::createAggressiveDCEPass()); break;
			case rr::Optimization::Pass::GVN: passManager->add(llvm::createGVNPass()); break;
			case rr::Optimization::Pass::InstructionCombining: passManager->add(llvm::createInstructionCombiningPass()); break;
			case rr::Optimization::Pass::Reassociate: passManager->add(llvm::createReassociatePass()); break;
			case rr::Optimization::Pass::DeadStoreElimination: passManager->add(llvm::createDeadStoreEliminationPass()); break;
			case rr::Optimization::Pass::SCCP: passManager->add(llvm::createSCCPPass()); break;
			case rr::Optimization::Pass::ScalarReplAggregates: passManager->add(llvm::createSROAPass()); break;
			case rr::Optimization::Pass::EarlyCSEPass: passManager->add(llvm::createEarlyCSEPass()); break;
			default:
				UNREACHABLE("pass: %d", int(pass));
		}
	}

	passManager->run(*module);
}

std::shared_ptr<rr::Routine> JITBuilder::acquireRoutine(llvm::Function **funcs, size_t count, const rr::Config &cfg)
{
	ASSERT(module);
	return std::make_shared<JITRoutine>(std::move(module), funcs, count, cfg);
}

void Nucleus::setDefaultConfig(const Config &cfg)
{
	std::unique_lock<std::mutex> lock(::defaultConfigLock);
	::defaultConfig() = cfg;
}

void Nucleus::adjustDefaultConfig(const Config::Edit &cfgEdit)
{
	std::unique_lock<std::mutex> lock(::defaultConfigLock);
	auto &config = ::defaultConfig();
	config = cfgEdit.apply(config);
}

Config Nucleus::getDefaultConfig()
{
	std::unique_lock<std::mutex> lock(::defaultConfigLock);
	return ::defaultConfig();
}

}  // namespace rr
