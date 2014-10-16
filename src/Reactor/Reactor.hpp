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

#ifndef BRANCHING_DEFINITIONS
#define BRANCHING_DEFINITIONS

#include "Nucleus.hpp"
#include "Routine.hpp"

namespace sw
{
	class ForContainer {
	public:
		ForContainer(RValue<Bool> cond)
		  : loopBB(beginLoop())
		  , bodyBB(Nucleus::createBasicBlock())
		  , endBB(Nucleus::createBasicBlock())
		  , doIncrement(true)
		{
			branch(cond, bodyBB, endBB);
		}
		bool loop() const { return doIncrement; }
		void increment() {
			Nucleus::createBr(loopBB);
			Nucleus::setInsertBlock(endBB);
			doIncrement = false;
		}
	private:
		llvm::BasicBlock* loopBB;
		llvm::BasicBlock* bodyBB;
		llvm::BasicBlock* endBB;
		bool doIncrement;
	};

	#define For(init, cond, inc) \
	init;                        \
	for (ForContainer fc(cond); fc.loop(); inc, fc.increment())

	#define While(cond) For(((void*)0), cond, ((void*)0))

	#define Do \
	{ \
		llvm::BasicBlock *body = Nucleus::createBasicBlock(); \
		Nucleus::createBr(body); \
		Nucleus::setInsertBlock(body);

	#define Until(cond) \
		llvm::BasicBlock *end = Nucleus::createBasicBlock(); \
		Nucleus::createCondBr((cond).value, end, body); \
		Nucleus::setInsertBlock(end); \
	}

	class IfContainer {
	public:
		IfContainer(RValue<Bool> cond)
		  : trueBB(Nucleus::createBasicBlock())
		  , falseBB(Nucleus::createBasicBlock())
		  , endBB(Nucleus::createBasicBlock())
		  , doIncrement(true)
		{
			branch(cond, trueBB, falseBB);
		}
		bool loop() const { return doIncrement; }
		void inc() {
			Nucleus::createBr(endBB);
			Nucleus::setInsertBlock(falseBB);
			Nucleus::createBr(endBB);
			Nucleus::setInsertBlock(endBB);
			doIncrement = false;
		}
	private:
		llvm::BasicBlock* trueBB;
		llvm::BasicBlock* falseBB;
		llvm::BasicBlock* endBB;
		bool doIncrement;
	};

	#define If(cond) \
	for (IfContainer ic(cond); ic.loop(); ic.inc())

	class ElseContainer {
	public:
		ElseContainer()
		  : endBB(Nucleus::getInsertBlock())
		  , falseBB(Nucleus::createBasicBlock())
		  , doIncrement(true)
		{
			elseBlock(falseBB);
		}
		bool loop() const { return doIncrement; }
		void inc() {
			Nucleus::createBr(endBB);
			Nucleus::setInsertBlock(endBB);
			doIncrement = false;
		}
	private:
		llvm::BasicBlock* endBB;
		llvm::BasicBlock* falseBB;
		bool doIncrement;
	};

	#define Else \
	for (ElseContainer ec; ec.loop(); ec.inc())
}

#endif