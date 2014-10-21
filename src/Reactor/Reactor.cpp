#include "Reactor.hpp"
#include "Routine.hpp"

namespace sw
{
	class ForHandlerImpl
	{
	public:
		ForHandlerImpl(RValue<Bool> cond)
			: loopBB(beginLoop())
			, bodyBB(Nucleus::createBasicBlock())
			, endBB(Nucleus::createBasicBlock())
			, doIncrement(true)
		{
			branch(cond, bodyBB, endBB);
		}
		~ForHandlerImpl()
		{
			Nucleus::createBr(loopBB);
			Nucleus::setInsertBlock(endBB);
		}
		operator bool() const
		{
			return doIncrement;
		}
		void operator++()
		{
			doIncrement = false;
		}
	private:
		llvm::BasicBlock *loopBB;
		llvm::BasicBlock *bodyBB;
		llvm::BasicBlock *endBB;
		bool doIncrement;
	};

	ForHandler::ForHandler(RValue<Bool> cond) : impl(new ForHandlerImpl(cond)) {}
	ForHandler::~ForHandler() { delete impl; }
	ForHandler::operator bool() const { return *impl; }
	void ForHandler::operator++() { ++(*impl); }

	class DoUntilHandlerImpl
	{
	public:
		DoUntilHandlerImpl()
			: body(Nucleus::createBasicBlock())
		{
			Nucleus::createBr(body);
			Nucleus::setInsertBlock(body);
		}
		void until(RValue<Bool> cond)
		{
			llvm::BasicBlock *end = Nucleus::createBasicBlock();
			Nucleus::createCondBr(cond.value, end, body);
			Nucleus::setInsertBlock(end);
		}
	private:
		llvm::BasicBlock *body;
	};

	DoUntilHandler::DoUntilHandler() : impl(new DoUntilHandlerImpl()) {}
	DoUntilHandler::~DoUntilHandler() { delete impl; }
	void DoUntilHandler::until(RValue<Bool> cond) { impl->until(cond); }

	class IfHandlerImpl
	{
	public:
		IfHandlerImpl(RValue<Bool> cond)
			: trueBB(Nucleus::createBasicBlock())
			, falseBB(Nucleus::createBasicBlock())
			, endBB(Nucleus::createBasicBlock())
			, doIncrement(true)
		{
			branch(cond, trueBB, falseBB);
		}
		~IfHandlerImpl()
		{
			Nucleus::createBr(endBB);
			Nucleus::setInsertBlock(falseBB);
			Nucleus::createBr(endBB);
			Nucleus::setInsertBlock(endBB);
		}
		operator bool() const
		{
			return doIncrement;
		}
		void operator++()
		{
			doIncrement = false;
		}
	private:
		llvm::BasicBlock *trueBB;
		llvm::BasicBlock *falseBB;
		llvm::BasicBlock *endBB;
		bool doIncrement;
	};

	IfHandler::IfHandler(RValue<Bool> cond) : impl(new IfHandlerImpl(cond)) {}
	IfHandler::~IfHandler() { delete impl; }
	IfHandler::operator bool() const { return (*impl); }
	void IfHandler::operator++() { ++(*impl); }

	class ElseHandlerImpl
	{
	public:
		ElseHandlerImpl()
			: endBB(Nucleus::getInsertBlock())
			, falseBB(Nucleus::createBasicBlock())
			, doIncrement(true)
		{
			elseBlock(falseBB);
		}
		~ElseHandlerImpl()
		{
			Nucleus::createBr(endBB);
			Nucleus::setInsertBlock(endBB);
		}
		operator bool() const
		{
			return doIncrement;
		}
		void operator++()
		{
			doIncrement = false;
		}
	private:
		llvm::BasicBlock *endBB;
		llvm::BasicBlock *falseBB;
		bool doIncrement;
	};

	ElseHandler::ElseHandler() : impl(new ElseHandlerImpl()) {}
	ElseHandler::~ElseHandler() { delete impl; }
	ElseHandler::operator bool() const { return (*impl); }
	void ElseHandler::operator++() { ++(*impl); }
}