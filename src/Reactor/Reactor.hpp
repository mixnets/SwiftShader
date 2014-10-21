// SwiftShader Software Renderer
//
// Copyright(c) 2005-2011 TransGaming Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of TransGaming Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

#ifndef sw_Reactor_hpp
#define sw_Reactor_hpp

#include "Nucleus.hpp"
#include "Routine.hpp"

namespace sw
{
	class ForHandler
	{
	public:
		ForHandler(RValue<Bool> cond)
			: loopBB(beginLoop())
			, bodyBB(Nucleus::createBasicBlock())
			, endBB(Nucleus::createBasicBlock())
			, doLoop(true)
		{
			branch(cond, bodyBB, endBB);
		}
		~ForHandler()
		{
			Nucleus::createBr(loopBB);
			Nucleus::setInsertBlock(endBB);
		}
		bool loop() const { return doLoop; }
		void setLoopDone() { doLoop = false; }
	private:
		llvm::BasicBlock *loopBB;
		llvm::BasicBlock *bodyBB;
		llvm::BasicBlock *endBB;
		bool doLoop;
	};

	#define For(init, cond, inc) \
	init;                        \
	for (ForHandler fc(cond); fc.loop(); inc, fc.setLoopDone())

	#define While(cond) \
	for (ForHandler fc(cond); fc.loop(); fc.setLoopDone())

	class DoUntilHandler
	{
	public:
		DoUntilHandler()
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

	#define Do \
	do { \
	DoUntilHandler duh;

	#define Until(cond) \
	duh.until(cond); \
	} while (false)

	class IfHandler
	{
	public:
		IfHandler(RValue<Bool> cond)
			: trueBB(Nucleus::createBasicBlock())
			, falseBB(Nucleus::createBasicBlock())
			, endBB(Nucleus::createBasicBlock())
			, doLoop(true)
		{
			branch(cond, trueBB, falseBB);
		}
		~IfHandler()
		{
			Nucleus::createBr(endBB);
			Nucleus::setInsertBlock(falseBB);
			Nucleus::createBr(endBB);
			Nucleus::setInsertBlock(endBB);
		}
		bool loop() const { return doLoop; }
		void setLoopDone() { doLoop = false; }
	private:
		llvm::BasicBlock *trueBB;
		llvm::BasicBlock *falseBB;
		llvm::BasicBlock *endBB;
		bool doLoop;
	};

	#define If(cond) \
	for (IfHandler ic(cond); ic.loop(); ic.setLoopDone())

	class ElseHandler
	{
	public:
		ElseHandler()
			: endBB(Nucleus::getInsertBlock())
			, falseBB(Nucleus::createBasicBlock())
			, doLoop(true)
		{
			elseBlock(falseBB);
		}
		~ElseHandler()
		{
			Nucleus::createBr(endBB);
			Nucleus::setInsertBlock(endBB);
		}
		bool loop() const { return doLoop; }
		void setLoopDone() { doLoop = false; }
	private:
		llvm::BasicBlock *endBB;
		llvm::BasicBlock *falseBB;
		bool doLoop;
	};

	#define Else \
	for (ElseHandler ec; ec.loop(); ec.setLoopDone())
}

#endif // sw_Reactor_hpp