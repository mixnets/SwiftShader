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

#include "SpirvShader.hpp"

#ifdef ENABLE_VK_DEBUGGER

#include "Vulkan/Debug/Context.hpp"
#include "Vulkan/Debug/File.hpp"
#include "Vulkan/Debug/Thread.hpp"
#include "Vulkan/Debug/Variable.hpp"

#include "spirv-tools/libspirv.h"
#include "OpenCLDebugInfo100.h"

#include <algorithm>

namespace {

// ArgTy<F>::type resolves to the single argument type of the function F.
template <typename F>
struct ArgTy {
	using type = typename ArgTy<decltype(&F::operator())>::type;
};

template <typename R, typename C, typename Arg>
struct ArgTy<R (C::*)(Arg) const> {
	using type = typename std::decay<Arg>::type;
};

template <typename T>
using ArgTyT = typename ArgTy<T>::type;

}  // anonymous namespace

namespace spvtools {

// Function implemented in third_party/SPIRV-Tools/source/disassemble.cpp
// but with no public header.
// This is a C++ function, so the name is mangled, and signature changes will
// result in a linker error instead of runtime signature mismatches.
std::string spvInstructionBinaryToText(const spv_target_env env,
                                       const uint32_t* inst_binary,
                                       const size_t inst_word_count,
                                       const uint32_t* binary,
                                       const size_t word_count,
                                       const uint32_t options);

}  // namespace spvtools

namespace {

const char* laneNames[] = { "Lane 0", "Lane 1", "Lane 2", "Lane 3" };
static_assert(sizeof(laneNames) / sizeof(laneNames[0]) == sw::SIMD::Width,
	"laneNames must have SIMD::Width entries");

template<typename T>
std::string tostring(const T& s) { return std::to_string(s); }
std::string tostring(const char* s) { return s; }
std::string tostring(sw::SpirvShader::Object::ID id) { return "%" + std::to_string(id.value()); }

namespace debug
{

struct Member;

struct Object
{
	enum class Kind
	{
		// Scopes
		CompilationUnit, LexicalBlock,

		// Types
		BasicType, VectorType, FunctionType, CompositeType,

		// Others
		Source, Member, Function, InlinedAt, LocalVariable, Declare, Value,
		Operation, Expression
	};

	using ID = sw::SpirvID<Object>;
	inline Object(Kind kind) : kind(kind) {}
	const Kind kind;
	static constexpr bool kindof(Object::Kind kind) { return true; }
};

template <typename TYPE, typename BASE, Object::Kind KIND>
struct ObjectImpl : public BASE
{
	ObjectImpl() : BASE(KIND) {}
	static_assert(BASE::kindof(KIND), "BASE::kindof() returned false");
	static constexpr bool kindof(Object::Kind kind) { return kind == KIND; }
	using ID = sw::SpirvID<TYPE>;
};

template <typename TO, typename FROM>
std::shared_ptr<TO> cast(const std::shared_ptr<FROM>& obj)
{
	return (TO::kindof(obj->kind)) ? std::static_pointer_cast<TO>(obj) : nullptr;
}

struct Scope : public Object
{
	using ID = sw::SpirvID<Scope>;
	inline Scope(Kind kind) : Object(kind) {}
	static constexpr bool kindof(Kind kind)
	{
		return kind == Kind::CompilationUnit || kind == Kind::LexicalBlock;
	}
};

struct Type : public Object
{
	using ID = sw::SpirvID<Type>;
	inline Type(Kind kind) : Object(kind) {}
	static constexpr bool kindof(Kind kind)
	{
		return kind == Kind::BasicType ||
		       kind == Kind::VectorType ||
		       kind == Kind::FunctionType ||
		       kind == Kind::CompositeType;
	}
};

struct CompilationUnit : public ObjectImpl<CompilationUnit, Scope, Object::Kind::CompilationUnit>
{

};

struct Source : public ObjectImpl<Source, Object, Object::Kind::Source>
{
	spv::SourceLanguage language;
	uint32_t version;
	std::string file;
	std::string source;
};

struct BasicType : public ObjectImpl<BasicType, Object, Object::Kind::BasicType>
{
	std::string name;
	uint32_t size = 0; // in bits.
	OpenCLDebugInfo100DebugBaseTypeAttributeEncoding encoding;
};

struct VectorType : public ObjectImpl<VectorType, Object, Object::Kind::VectorType>
{
	std::shared_ptr<Type> base;
	uint32_t components = 0;
};

struct FunctionType : public ObjectImpl<FunctionType, Object, Object::Kind::FunctionType>
{
	uint32_t flags = 0; // OR'd from OpenCLDebugInfo100DebugInfoFlags
	std::shared_ptr<Type> returnTy;
	std::vector<std::shared_ptr<Type>> paramTys;
};

struct CompositeType : public ObjectImpl<CompositeType, Object, Object::Kind::CompositeType>
{
	std::string name;
	OpenCLDebugInfo100DebugCompositeType tag;
	std::shared_ptr<Source> source;
	uint32_t line = 0;
	uint32_t column = 0;
	std::shared_ptr<Object> parent;
	std::string linkage;
	uint32_t size = 0; // in bits.
	uint32_t flags = 0; // OR'd from OpenCLDebugInfo100DebugInfoFlags
	std::vector<std::shared_ptr<Member>> members;
};

struct Member : public ObjectImpl<Member, Object, Object::Kind::Member>
{
	std::string name;
	std::shared_ptr<Type> type;
	std::shared_ptr<Source> source;
	uint32_t line = 0;
	uint32_t column = 0;
	std::shared_ptr<CompositeType> parent;
	uint32_t offset = 0; // in bits
	uint32_t size = 0; // in bits
	uint32_t flags = 0; // OR'd from OpenCLDebugInfo100DebugInfoFlags
};

struct Function : public ObjectImpl<Function, Object, Object::Kind::Function>
{
	std::string name;
	std::shared_ptr<FunctionType> type;
	std::shared_ptr<Source> source;
	uint32_t line = 0;
	uint32_t column = 0;
	std::shared_ptr<Scope> parent;
	std::string linkage;
	uint32_t flags = 0; // OR'd from OpenCLDebugInfo100DebugInfoFlags
	uint32_t scopeLine = 0;
	sw::SpirvShader::Function::ID function;
};

struct LexicalBlock : public ObjectImpl<LexicalBlock, Scope, Object::Kind::LexicalBlock>
{
	std::shared_ptr<Source> source;
	uint32_t line = 0;
	uint32_t column = 0;
	std::shared_ptr<Scope> parent;
	std::string name;
};

struct InlinedAt : public ObjectImpl<InlinedAt, Object, Object::Kind::InlinedAt>
{
	uint32_t line = 0;
	std::shared_ptr<Scope> scope;
	std::shared_ptr<InlinedAt> inlined;
};

struct LocalVariable : public ObjectImpl<LocalVariable, Object, Object::Kind::LocalVariable>
{
	static constexpr uint32_t NoArg = ~uint32_t(0);

	std::string name;
	std::shared_ptr<Type> type;
	std::shared_ptr<Source> source;
	uint32_t line = 0;
	uint32_t column = 0;
	std::shared_ptr<Scope> parent;
	uint32_t arg = NoArg;
};

struct Operation : public ObjectImpl<Operation, Object, Object::Kind::Operation>
{
	uint32_t opcode;
	std::vector<uint32_t> operands;
};

struct Expression : public ObjectImpl<Expression, Object, Object::Kind::Expression>
{
	std::vector<std::shared_ptr<Operation>> operations;
};

struct Declare : public ObjectImpl<Declare, Object, Object::Kind::Declare>
{
	std::shared_ptr<LocalVariable> local;
	sw::SpirvShader::Object::ID variable;
	std::shared_ptr<Expression> expression;
};

struct Value : public ObjectImpl<Value, Object, Object::Kind::Value>
{
	std::shared_ptr<LocalVariable> local;
	sw::SpirvShader::Object::ID variable;
	std::shared_ptr<Expression> expression;
	std::vector<uint32_t> indexes;
};

}  // namespace debug
}  // anonymous namespace

namespace rr {

////////////////////////////////////////////////////////////////////////////////
// rr::CToReactor<T> specializations.
////////////////////////////////////////////////////////////////////////////////
template<>
struct CToReactor<sw::SpirvShader::Object::ID>
{
	using type = rr::Int;
	static rr::Int cast(sw::SpirvShader::Object::ID id) { return rr::Int(id.value()); }
};

template<typename T>
struct CToReactor<vk::dbg::ID<T>>
{
	using type = rr::Int;
	static rr::Int cast(vk::dbg::ID<T> id) { return rr::Int(id.value()); }
};

} // namespace rr

namespace sw {

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger
//
// Private struct holding debugger information for the SpirvShader.
////////////////////////////////////////////////////////////////////////////////
struct SpirvShader::Impl::Debugger
{
	class Context;
	class Group;
	class State;

	enum class Pass
	{
		Define,
		Emit
	};

	void process(const SpirvShader *shader, const InsnIterator &insn, Pass pass);

	// exposeVariable exposes the variable with the given ID to the
	// debugger using the specified key.
	template<typename Key>
	void exposeVariable(const SpirvShader* shader, const Key& key, Object::ID id, EmitState *state) const;

	// exposeVariable exposes the variable with the given ID to the
	// debugger under the specified group, for the specified SIMD lane.
	template<typename Key>
	void exposeVariable(const SpirvShader* shader, const Group& group, int lane, const Key& key, Object::ID id, EmitState *state) const;

	template<typename ID, typename T>
	void add(ID id, const std::shared_ptr<T>&);

	template<typename T>
	std::shared_ptr<T> get(SpirvID<T> id) const;

	std::shared_ptr<vk::dbg::Context> ctx;
	std::shared_ptr<vk::dbg::File> spirvFile;
	std::unordered_map<const void*, int> spirvLineMappings;  // instruction pointer to line
	std::unordered_map<const void*, Object::ID> results; // instruction pointer to result ID

private:
	// use get() and add() to access these
	std::unordered_map<debug::Object::ID, std::shared_ptr<debug::Object>> objects;

	template<typename F, typename T = typename ArgTyT<F>::element_type>
	void defineOrEmit(InsnIterator insn, Pass pass, F&& emit);
};

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger::State
//
// State holds the runtime data structures for the shader debug session.
////////////////////////////////////////////////////////////////////////////////
class SpirvShader::Impl::Debugger::State
{
public:
	static State* create(const Debugger* debugger, const char* name);
	static void destroy(State*);

	State(const Debugger* debugger, const char* stackBase, vk::dbg::Context::Lock& lock);
	~State();

	void enter(vk::dbg::Context::Lock& lock, const char* name);
	void exit();
	void updateActiveLaneMask(int lane, bool enabled);
	void update(int line, vk::dbg::File::ID fileID);

	vk::dbg::VariableContainer* registers();
	vk::dbg::VariableContainer* locals();
	vk::dbg::VariableContainer* hovers();

	vk::dbg::VariableContainer* registersLane(int lane);
	vk::dbg::VariableContainer* localsLane(int lane);

	template<typename K>
	vk::dbg::VariableContainer* group(vk::dbg::VariableContainer* vc, K key);

	template<typename K, typename V>
	void put(vk::dbg::VariableContainer* vc, K key, V value);

	const Debugger* debugger;
	const std::shared_ptr<vk::dbg::Thread> thread;
	std::array<std::shared_ptr<vk::dbg::VariableContainer>, sw::SIMD::Width> registersByLane;
	std::array<std::shared_ptr<vk::dbg::VariableContainer>, sw::SIMD::Width> localsByLane;
};

SpirvShader::Impl::Debugger::State* SpirvShader::Impl::Debugger::State::create(const Debugger* debugger, const char* name)
{
	auto lock = debugger->ctx->lock();
	return new State(debugger, name, lock);
}

void SpirvShader::Impl::Debugger::State::destroy(State* state)
{
	delete state;
}

SpirvShader::Impl::Debugger::State::State(const Debugger* debugger, const char* stackBase, vk::dbg::Context::Lock& lock)
	: debugger(debugger)
	, thread(lock.currentThread())
{
	enter(lock, stackBase);
	for (int i = 0; i < sw::SIMD::Width; i++) {
		registersByLane[i] = lock.createVariableContainer();
		localsByLane[i] = lock.createVariableContainer();
		thread->registers()->put(laneNames[i], registersByLane[i]);
		thread->locals()->put(laneNames[i], localsByLane[i]);
	}
}

SpirvShader::Impl::Debugger::State::~State() { exit(); }

void SpirvShader::Impl::Debugger::State::enter(vk::dbg::Context::Lock& lock, const char* name) {
	thread->enter(lock, debugger->spirvFile, name);
}

void SpirvShader::Impl::Debugger::State::exit() { thread->exit(); }

void SpirvShader::Impl::Debugger::State::updateActiveLaneMask(int lane, bool enabled)
{
	registersByLane[lane]->put("enabled", vk::dbg::make_constant(enabled));
}

void SpirvShader::Impl::Debugger::State::update(int line, vk::dbg::File::ID fileID)
{
	auto file = debugger->ctx->lock().get(fileID);
	thread->update({line, file});
}

vk::dbg::VariableContainer* SpirvShader::Impl::Debugger::State::registers() { return thread->registers().get(); }
vk::dbg::VariableContainer* SpirvShader::Impl::Debugger::State::locals() { return thread->locals().get(); }
vk::dbg::VariableContainer* SpirvShader::Impl::Debugger::State::hovers() { return thread->hovers().get(); }

vk::dbg::VariableContainer* SpirvShader::Impl::Debugger::State::registersLane(int i) { return registersByLane[i].get(); }
vk::dbg::VariableContainer* SpirvShader::Impl::Debugger::State::localsLane(int i) { return localsByLane[i].get(); }

template<typename K>
vk::dbg::VariableContainer* SpirvShader::Impl::Debugger::State::group(vk::dbg::VariableContainer* vc, K key)
{
	auto out = debugger->ctx->lock().createVariableContainer();
	vc->put(tostring(key), out);
	return out.get();
}

template<typename K, typename V>
void SpirvShader::Impl::Debugger::State::put(vk::dbg::VariableContainer* vc, K key, V value)
{
	vc->put(tostring(key), vk::dbg::make_constant(value));
}

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger::Group
//
// This provides a convenient C++ interface for adding debugger values to
// VariableContainers.
////////////////////////////////////////////////////////////////////////////////
class SpirvShader::Impl::Debugger::Group
{
public:
	using Ptr = rr::Pointer<rr::Byte>;

	Group(Ptr ctx, Ptr group);

	template<typename K, typename RK>
	Group group(RK key) const;

	template<typename K, typename V, typename RK, typename RV>
	void put(RK key, RV value) const;

	template<typename K, typename V, typename RK, typename RV>
	void put(RK key, RV x, RV y) const;

	template<typename K, typename V, typename RK, typename RV>
	void put(RK key, RV x, RV y, RV z) const;

	template<typename K, typename V, typename RK, typename RV>
	void put(RK key, RV x, RV y, RV z, RV w) const;

	template<typename K, typename V, typename VEC>
	void putVec3(K key, const VEC& v) const;

private:
	Ptr ctx;
	Ptr ptr;
};

SpirvShader::Impl::Debugger::Group::Group(Ptr ctx, Ptr group) : ctx(ctx), ptr(group) {}

template<typename K, typename RK>
SpirvShader::Impl::Debugger::Group SpirvShader::Impl::Debugger::Group::group(RK key) const
{
	return Group(ctx, rr::Call(&State::group<K>, ctx, ptr, key));
}

template<typename K, typename V, typename RK, typename RV>
void SpirvShader::Impl::Debugger::Group::put(RK key, RV value) const
{
	rr::Call(&State::put<K, V>, ctx, ptr, key, value);
}

template<typename K, typename V, typename RK, typename RV>
void SpirvShader::Impl::Debugger::Group::put(RK key, RV x, RV y) const
{
	auto vec = group<K>(key);
	vec.template put<const char*, V>("x", x);
	vec.template put<const char*, V>("y", y);
}

template<typename K, typename V, typename RK, typename RV>
void SpirvShader::Impl::Debugger::Group::put(RK key, RV x, RV y, RV z) const
{
	auto vec = group<K>(key);
	vec.template put<const char*, V>("x", x);
	vec.template put<const char*, V>("y", y);
	vec.template put<const char*, V>("z", z);
}

template<typename K, typename V, typename RK, typename RV>
void SpirvShader::Impl::Debugger::Group::put(RK key, RV x, RV y, RV z, RV w) const
{
	auto vec = group<K>(key);
	vec.template put<const char*, V>("x", x);
	vec.template put<const char*, V>("y", y);
	vec.template put<const char*, V>("z", z);
	vec.template put<const char*, V>("w", w);
}

template<typename K, typename V, typename VEC>
void SpirvShader::Impl::Debugger::Group::putVec3(K key, const VEC& v) const
{
	auto vec = group<K>(key);
	vec.template put<const char*, V>("x", Extract(v, 0));
	vec.template put<const char*, V>("y", Extract(v, 1));
	vec.template put<const char*, V>("z", Extract(v, 2));
}

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger::Context
//
// This provides the compile-time interface to control the debug context state.
////////////////////////////////////////////////////////////////////////////////
class SpirvShader::Impl::Debugger::Context {
public:
	using Ptr = rr::Pointer<rr::Byte>;

	Context(Ptr state);


	void update(int line, vk::dbg::File::ID file) const;

	void updateActiveLaneMask(int lane, rr::Int enabled) const;

	Group registers() const;
	Group locals() const;
	Group hovers() const;

	Group registersLane(uint32_t lane) const;
	Group localsLane(uint32_t lane) const;

private:
	Ptr state;
};

SpirvShader::Impl::Debugger::Context::Context(Ptr state) : state(state) {}

void SpirvShader::Impl::Debugger::Context::update(int line, vk::dbg::File::ID file) const
{
	rr::Call(&State::update, state, line, file);
}

void SpirvShader::Impl::Debugger::Context::updateActiveLaneMask(int lane, rr::Int enabled) const
{
	rr::Call(&State::updateActiveLaneMask, state, lane, enabled != 0);
}

SpirvShader::Impl::Debugger::Group SpirvShader::Impl::Debugger::Context::registers() const
{
	return Group(state, rr::Call(&State::registers, state));
}

SpirvShader::Impl::Debugger::Group SpirvShader::Impl::Debugger::Context::locals() const
{
	return Group(state, rr::Call(&State::locals, state));
}

SpirvShader::Impl::Debugger::Group SpirvShader::Impl::Debugger::Context::hovers() const
{
	return Group(state, rr::Call(&State::hovers, state));
}

SpirvShader::Impl::Debugger::Group SpirvShader::Impl::Debugger::Context::registersLane(uint32_t lane) const
{
	return Group(state, rr::Call(&State::registersLane, state, lane));
}

SpirvShader::Impl::Debugger::Group SpirvShader::Impl::Debugger::Context::localsLane(uint32_t lane) const
{
	return Group(state, rr::Call(&State::localsLane, state, lane));
}

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger methods
////////////////////////////////////////////////////////////////////////////////
template<typename F, typename T>
void SpirvShader::Impl::Debugger::defineOrEmit(InsnIterator insn, Pass pass, F&& emit)
{
	auto id = SpirvID<T>(insn.word(2));
	switch (pass)
	{
	case Pass::Define:
	{
		add(id, std::make_shared<T>());
		break;
	}
	case Pass::Emit:
		emit(get<T>(id));
		break;
	}
}

void SpirvShader::Impl::Debugger::process(const SpirvShader *shader, const InsnIterator &insn, Pass pass)
{
	auto extInstIndex = insn.word(4);
	switch (extInstIndex)
	{
	case OpenCLDebugInfo100DebugCompilationUnit:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::CompilationUnit>&) {});
		break;
	case OpenCLDebugInfo100DebugTypeBasic:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::BasicType>& type) {
			type->name = shader->getString(insn.word(5));
			type->size = shader->GetConstScalarInt(insn.word(6));
			type->encoding = static_cast<OpenCLDebugInfo100DebugBaseTypeAttributeEncoding>(insn.word(7));
		});
		break;
	case OpenCLDebugInfo100DebugTypeVector:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::VectorType>& type) {
			type->base = get(debug::Type::ID(insn.word(5)));
			type->components = insn.word(6);
		});
		break;
	case OpenCLDebugInfo100DebugTypeFunction:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::FunctionType>& type) {
			type->flags = insn.word(5);
			type->returnTy = get(debug::Type::ID(insn.word(6)));
			for (uint32_t i = 7; i < insn.wordCount(); i++)
			{
				type->paramTys.push_back(get(debug::Type::ID(insn.word(i))));
			}
		});
		break;
	case OpenCLDebugInfo100DebugTypeComposite:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::CompositeType>& type) {
			type->name = shader->getString(insn.word(5));
			type->tag = static_cast<OpenCLDebugInfo100DebugCompositeType>(insn.word(6));
			type->source = get(debug::Source::ID(insn.word(7)));
			type->line = insn.word(8);
			type->column = insn.word(9);
			type->parent = get(debug::Object::ID(insn.word(10)));
			type->linkage = shader->getString(insn.word(11));
			type->size = shader->GetConstScalarInt(insn.word(12));
			type->flags = insn.word(13);
			for (uint32_t i = 14; i < insn.wordCount(); i++)
			{
				auto obj = get(debug::Object::ID(insn.word(i)));
				if (auto member = debug::cast<debug::Member>(obj)) // Can also be Function or TypeInheritance, which we don't care about.
				{
					type->members.push_back(member);
				}
			}
		});
		break;
	case OpenCLDebugInfo100DebugTypeMember:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::Member>& member) {
			member->name = shader->getString(insn.word(5));
			member->type = get(debug::Type::ID(insn.word(6)));
			member->source = get(debug::Source::ID(insn.word(7)));
			member->line = insn.word(8);
			member->column = insn.word(9);
			member->parent = get(debug::CompositeType::ID(insn.word(10)));
			member->offset = shader->GetConstScalarInt(insn.word(11));
			member->size = shader->GetConstScalarInt(insn.word(12));
			member->flags = insn.word(13);
		});
		break;
	case OpenCLDebugInfo100DebugFunction:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::Function>& func) {
			func->name = shader->getString(insn.word(5));
			func->type = get(debug::FunctionType::ID(insn.word(6)));
			func->source = get(debug::Source::ID(insn.word(7)));
			func->line = insn.word(8);
			func->column = insn.word(9);
			func->parent = get(debug::Scope::ID(insn.word(10)));
			func->linkage = shader->getString(insn.word(11));
			func->flags = insn.word(12);
			func->scopeLine = insn.word(13);
			func->function = Function::ID(insn.word(14));
			// declaration: word(13)
		});
		break;
	case OpenCLDebugInfo100DebugLexicalBlock:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::LexicalBlock>& scope) {
			scope->source = get(debug::Source::ID(insn.word(5)));
			scope->line = insn.word(6);
			scope->column = insn.word(7);
			scope->parent = get(debug::Scope::ID(insn.word(8)));
			if (insn.wordCount() > 9)
			{
				scope->name = shader->getString(insn.word(9));
			}
		});
		break;
	case OpenCLDebugInfo100DebugScope:
		// TODO
		// defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::Scope>& scope) {
		// 	if (insn.wordCount() >= 6)
		// 	{
		// 		auto inlinedAt = get(debug::InlinedAt::ID(insn.word(6)));
		// 		(void) inlinedAt;
		// 		// TODO: Use.
		// 	}
		// });
		break;
	case OpenCLDebugInfo100DebugNoScope:
		break;
	case OpenCLDebugInfo100DebugLocalVariable:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::LocalVariable>& var) {
			var->name = shader->getString(insn.word(5));
			var->type = get(debug::Type::ID(insn.word(6)));
			var->source = get(debug::Source::ID(insn.word(7)));
			var->line = insn.word(8);
			var->column = insn.word(9);
			var->parent = get(debug::Scope::ID(insn.word(10)));
			if (insn.wordCount() >= 11)
			{
				var->arg = insn.word(11);
			}
		});
		break;
	case OpenCLDebugInfo100DebugDeclare:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::Declare>& decl) {
			decl->local = get(debug::LocalVariable::ID(insn.word(5)));
			decl->variable = Object::ID(insn.word(6));
			decl->expression = get(debug::Expression::ID(insn.word(7)));
		});
		break;
	case OpenCLDebugInfo100DebugValue:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::Value>& value) {
			value->local = get(debug::LocalVariable::ID(insn.word(5)));
			value->variable = Object::ID(insn.word(6));
			value->expression = get(debug::Expression::ID(insn.word(7)));
			for (uint32_t i = 8; i < insn.wordCount(); i++)
			{
				value->indexes.push_back(insn.word(i));
			}
		});
		break;
	case OpenCLDebugInfo100DebugExpression:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::Expression>& expr) {
			for (uint32_t i = 5; i < insn.wordCount(); i++)
			{
				expr->operations.push_back(get(debug::Operation::ID(insn.word(i))));
			}
		});
		break;
	case OpenCLDebugInfo100DebugSource:
		defineOrEmit(insn, pass, [&](const std::shared_ptr<debug::Source>& source) {
			source->file = shader->getString(insn.word(5));
			if (insn.wordCount() >= 6)
			{
				source->source = shader->getString(insn.word(6));
			}
		});
		break;
	default:
		UNSUPPORTED("Unsupported OpenCLDebugInfo100 instruction %d", int(extInstIndex));
	}
}

template<typename Key>
void SpirvShader::Impl::Debugger::exposeVariable(const SpirvShader* shader, const Key& key, Object::ID id, EmitState *state) const
{
	auto ctx = Context(state->routine->dbgState);
	auto hover = ctx.hovers().group<Key>(key);
	for (int lane = 0; lane < SIMD::Width; lane++) {
		exposeVariable(shader, ctx.localsLane(lane), lane, key, id, state);
		exposeVariable(shader, hover, lane, laneNames[lane], id, state);
	}
}

template<typename ID, typename T>
void SpirvShader::Impl::Debugger::add(ID id, const std::shared_ptr<T>& obj)
{
	auto added = objects.emplace(debug::Object::ID(id.value()), obj).second;
	ASSERT_MSG(added, "Debug object with %d already exists", id.value());
}

template <typename T>
std::shared_ptr<T> SpirvShader::Impl::Debugger::get(SpirvID<T> id) const
{
	auto it = objects.find(debug::Object::ID(id.value()));
	ASSERT_MSG(it != objects.end(), "Unknown debug object %d", id.value());
	auto ptr = debug::cast<T>(it->second);
	ASSERT_MSG(ptr, "Debug object %d is not of the correct type", id.value());
	return ptr;
}

template<typename Key>
void SpirvShader::Impl::Debugger::exposeVariable(const SpirvShader* shader, const Group& group, int l, const Key& key, Object::ID id, EmitState *state) const
{
	GenericValue val(shader, state, id);
	auto ctx = Context(state->routine->dbgState);
	switch (shader->getType(val.type).opcode()) {
	case spv::OpTypeInt:
		group.put<Key, int>(key, Extract(val.Int(0), l));
		break;
	case spv::OpTypeFloat:
		group.put<Key, float>(key, Extract(val.Float(0), l));
		break;
	case spv::OpTypeVector: {
		auto count = shader->getType(val.type).definition.word(3);
		switch (count) {
			case 1:
				group.put<Key, float>(key, Extract(val.Float(0), l));
				break;
			case 2:
				group.put<Key, float>(key, Extract(val.Float(0), l), Extract(val.Float(1), l));
				break;
			case 3:
				group.put<Key, float>(key, Extract(val.Float(0), l), Extract(val.Float(1), l), Extract(val.Float(2), l));
				break;
			case 4:
				group.put<Key, float>(key, Extract(val.Float(0), l), Extract(val.Float(1), l), Extract(val.Float(2), l), Extract(val.Float(3), l));
				break;
			default: {
				auto vec = group.group<Key>(key);
				for (uint32_t i = 0; i < count; i++) {
					vec.template put<int, float>(i, Extract(val.Float(i), l));
				}
				break;
			}
		}
		break;
	}
	case spv::OpTypePointer: {
		auto objectTy = shader->getType(shader->getObject(id).type);
		bool interleavedByLane = IsStorageInterleavedByLane(objectTy.storageClass);
		auto ptr = state->getPointer(id);
		auto ptrGroup = group.group<Key>(key);
		shader->VisitMemoryObject(id, [&](const MemoryElement& el) {
			auto p = ptr + el.offset;
			if (interleavedByLane) { p = InterleaveByLane(p); }  // TODO: Interleave once, then add offset?
			auto simd = p.Load<SIMD::Float>(sw::OutOfBoundsBehavior::Nullify, state->activeLaneMask());
			ptrGroup.template put<int, float>(el.index, Extract(simd, l));
		});
		break;
	}
	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader
////////////////////////////////////////////////////////////////////////////////
void SpirvShader::dbgInit(const std::shared_ptr<vk::dbg::Context>& dbgctx)
{
	impl.debugger = new Impl::Debugger();
	impl.debugger->ctx = dbgctx;
}

void SpirvShader::dbgTerm()
{
	if (impl.debugger)
	{
		delete impl.debugger;
	}
}

void SpirvShader::dbgCreateFile()
{
	auto dbg = impl.debugger;
	if (!dbg) { return; }

	int currentLine = 1;
	std::string source;
	for (auto insn : *this) {
		auto instruction = spvtools::spvInstructionBinaryToText(
			SPV_ENV_VULKAN_1_1,
			insn.wordPointer(0),
			insn.wordCount(),
			insns.data(),
			insns.size(),
			SPV_BINARY_TO_TEXT_OPTION_NO_HEADER) + "\n";
		dbg->spirvLineMappings[insn.wordPointer(0)] = currentLine;
		currentLine += std::count(instruction.begin(), instruction.end(), '\n');
		source += instruction;
	}
	std::string name;
	switch (executionModel)
	{
		case spv::ExecutionModelVertex:    name = "VertexShader";   break;
		case spv::ExecutionModelFragment:  name = "FragmentShader"; break;
		case spv::ExecutionModelGLCompute: name = "ComputeShader";  break;
		default: name = "SPIR-V Shader"; break;
	}
	static std::atomic<int> id = { 0 };
	name += std::to_string(id++) + ".spvasm";
	dbg->spirvFile = dbg->ctx->lock().createVirtualFile(name.c_str(), source.c_str());
}

void SpirvShader::dbgBeginEmit(EmitState *state) const
{
	auto dbg = impl.debugger;
	if (!dbg) { return; }

	auto routine = state->routine;

	auto type = "SPIR-V";
	switch (executionModel)
	{
		case spv::ExecutionModelVertex:    type = "VertexShader";   break;
		case spv::ExecutionModelFragment:  type = "FragmentShader"; break;
		case spv::ExecutionModelGLCompute: type = "ComputeShader";  break;
		default: type = "SPIR-V Shader"; break;
	}
	routine->dbgState = rr::Call(&Impl::Debugger::State::create, this, type);

	auto ctx = Impl::Debugger::Context(routine->dbgState);
	SetActiveLaneMask(state->activeLaneMask(), state);

	auto locals = ctx.locals();
	locals.put<const char*, int>("subgroupSize", routine->invocationsPerSubgroup);

	switch (executionModel) {
	case spv::ExecutionModelGLCompute:
		locals.putVec3<const char*, int>("numWorkgroups", routine->numWorkgroups);
		locals.putVec3<const char*, int>("workgroupID", routine->workgroupID);
		locals.putVec3<const char*, int>("workgroupSize", routine->workgroupSize);
		locals.put<const char*, int>("numSubgroups", routine->subgroupsPerWorkgroup);
		locals.put<const char*, int>("subgroupIndex", routine->subgroupIndex);

		for (int i = 0; i < SIMD::Width; i++) {
			auto lane = ctx.localsLane(i);
			lane.put<const char*, int>("globalInvocationId",
				rr::Extract(routine->globalInvocationID[0], i),
				rr::Extract(routine->globalInvocationID[1], i),
				rr::Extract(routine->globalInvocationID[2], i));
			lane.put<const char*, int>("localInvocationId",
				rr::Extract(routine->localInvocationID[0], i),
				rr::Extract(routine->localInvocationID[1], i),
				rr::Extract(routine->localInvocationID[2], i));
			lane.put<const char*, int>("localInvocationIndex", rr::Extract(routine->localInvocationIndex, i));
		}
		break;

	case spv::ExecutionModelFragment:
		locals.put<const char*, int>("viewIndex", routine->viewID);
		for (int i = 0; i < SIMD::Width; i++) {
			auto lane = ctx.localsLane(i);
			lane.put<const char*, float>("fragCoord",
				rr::Extract(routine->fragCoord[0], i),
				rr::Extract(routine->fragCoord[1], i),
				rr::Extract(routine->fragCoord[2], i),
				rr::Extract(routine->fragCoord[3], i));
			lane.put<const char*, float>("pointCoord",
				rr::Extract(routine->pointCoord[0], i),
				rr::Extract(routine->pointCoord[1], i));
			lane.put<const char*, int>("windowSpacePosition",
				rr::Extract(routine->windowSpacePosition[0], i),
				rr::Extract(routine->windowSpacePosition[1], i));
			lane.put<const char*, int>("helperInvocation", rr::Extract(routine->helperInvocation, i));
		}
		break;

	case spv::ExecutionModelVertex:
		break;

	default:
		break;
	}
}

void SpirvShader::dbgEndEmit(EmitState *state) const
{
	auto dbg = impl.debugger;
	if (!dbg) { return; }

	rr::Call(&Impl::Debugger::State::destroy, state->routine->dbgState);
}

void SpirvShader::dbgBeginEmitInstruction(InsnIterator insn, EmitState *state) const
{
			auto instruction = spvtools::spvInstructionBinaryToText(
				SPV_ENV_VULKAN_1_1,
				insn.wordPointer(0),
				insn.wordCount(),
				insns.data(),
				insns.size(),
				SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
			printf("%s\n", instruction.c_str());

	auto dbg = impl.debugger;
	if (!dbg) { return; }

	auto line = dbg->spirvLineMappings.at(insn.wordPointer(0));
	Impl::Debugger::Context(state->routine->dbgState).update(line, dbg->spirvFile->id);
}

void SpirvShader::dbgEndEmitInstruction(InsnIterator insn, EmitState *state) const
{
	auto dbg = impl.debugger;
	if (!dbg) { return; }

	auto resIt = dbg->results.find(insn.wordPointer(0));
	if (resIt != dbg->results.end())
	{
		auto id = resIt->second;
		dbgExposeIntermediate(id, state);
	}
}

void SpirvShader::dbgExposeIntermediate(Object::ID id, EmitState *state) const
{
	auto dbg = impl.debugger;
	if (!dbg) { return; }

	dbg->exposeVariable(this, id, id, state);
}

void SpirvShader::dbgUpdateActiveLaneMask(RValue<SIMD::Int> mask, EmitState *state) const
{
	auto dbg = impl.debugger;
	if (!dbg) { return; }

	auto ctx = Impl::Debugger::Context(state->routine->dbgState);
	for (int lane = 0; lane < SIMD::Width; lane++) {
		ctx.updateActiveLaneMask(lane, rr::Extract(mask, lane));
	}
}

void SpirvShader::dbgDeclareResult(const InsnIterator &insn, Object::ID resultId) const
{
	auto dbg = impl.debugger;
	if (!dbg) { return; }

	dbg->results.emplace(insn.wordPointer(0), resultId);
}

void SpirvShader::DefineOpenCLDebugInfo100(const InsnIterator &insn)
{
	auto dbg = impl.debugger;
	if (!dbg) { return; }

	dbg->process(this, insn, Impl::Debugger::Pass::Define);
}

SpirvShader::EmitResult SpirvShader::EmitOpenCLDebugInfo100(InsnIterator insn, EmitState *state) const
{
	if (auto dbg = impl.debugger)
	{
		dbg->process(this, insn, Impl::Debugger::Pass::Emit);
	}
	return EmitResult::Continue;
}

} // namespace sw

#else // ENABLE_VK_DEBUGGER

// Stub implementations of the dbgXXX functions.
namespace sw {

void SpirvShader::dbgInit(const std::shared_ptr<vk::dbg::Context>& dbgctx) {}
void SpirvShader::dbgTerm() {}
void SpirvShader::dbgCreateFile() {}
void SpirvShader::dbgBeginEmit(EmitState *state) const {}
void SpirvShader::dbgEndEmit(EmitState *state) const {}
void SpirvShader::dbgBeginEmitInstruction(InsnIterator insn, EmitState *state) const {}
void SpirvShader::dbgEndEmitInstruction(InsnIterator insn, EmitState *state) const {}
void SpirvShader::dbgExposeIntermediate(Object::ID id, EmitState *state) const {}
void SpirvShader::dbgUpdateActiveLaneMask(RValue<SIMD::Int> mask, EmitState *state) const {}
void SpirvShader::dbgDeclareResult(const InsnIterator &insn, Object::ID resultId) const {}

SpirvShader::EmitResult SpirvShader::EmitOpenCLDebugInfo100(InsnIterator insn, EmitState *state) const
{
	return EmitResult::Continue;
}

} // namespace sw

#endif // ENABLE_VK_DEBUGGER

