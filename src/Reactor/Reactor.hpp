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

namespace sw
{
	class ForHandlerImpl;
	class ForHandler
	{
	public:
		ForHandler(RValue<Bool> cond);
		~ForHandler();
		operator bool() const;
		void operator++();
	private:
		ForHandlerImpl* impl;
	};

	#define For(init, cond, inc) \
	init;                        \
	for (ForHandler fc(cond); fc; inc, ++fc)

	#define While(cond) \
	for (ForHandler fc(cond); fc; ++fc)

	class DoUntilHandlerImpl;
	class DoUntilHandler
	{
	public:
		DoUntilHandler();
		~DoUntilHandler();
		void until(RValue<Bool> cond);
	private:
		DoUntilHandlerImpl* impl;
	};

	#define Do \
	do { \
	DoUntilHandler duh;

	#define Until(cond) \
	duh.until(cond); \
	} while (false)

	class IfHandlerImpl;
	class IfHandler
	{
	public:
		IfHandler(RValue<Bool> cond);
		~IfHandler();
		operator bool() const;
		void operator++();
	private:
		IfHandlerImpl* impl;
	};

	#define If(cond) \
	for (IfHandler ic(cond); ic; ++ic)

	class ElseHandlerImpl;
	class ElseHandler
	{
	public:
		ElseHandler();
		~ElseHandler();
		operator bool() const;
		void operator++();
	private:
		ElseHandlerImpl* impl;
	};

	#define Else \
	for (ElseHandler ec; ec; ++ec)
}

#endif // sw_Reactor_hpp