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

#ifndef sw_MutexLock_hpp
#define sw_MutexLock_hpp

#include "Thread.hpp"
#include "ThreadAnalyser.h"

namespace sw
{
	class BackoffLock
	{
	public:
		BackoffLock()
		{
			mutex = 0;
		}

		bool attemptLock()
		{
			if(!isLocked())
			{
				if(atomicExchange(&mutex, 1) == 0)
				{
					return true;
				}
			}

			return false;
		}
		
		void lock(LockResourceId lockId, ThreadAnalyzer * ta)
		{
			threadAnalyzer = ta;
			int backoff = 1;
			bool logged = false;

			while(!attemptLock())
			{
				if(!logged)
				{
					// Locked by another thread, log the conflict!
					ConflictWhenLocking(lockId);
					logged = true;
				}

				if(backoff <= 64)
				{
					for(int i = 0; i < backoff; i++)
					{
						nop();
						nop();
						nop();
						nop();
						nop();

						nop();
						nop();
						nop();
						nop();
						nop();

						nop();
						nop();
						nop();
						nop();
						nop();

						nop();
						nop();
						nop();
						nop();
						nop();

						nop();
						nop();
						nop();
						nop();
						nop();

						nop();
						nop();
						nop();
						nop();
						nop();

						nop();
						nop();
						nop();
						nop();
						nop();
					}

					backoff *= 2;
				}
				else
				{
					Thread::yield();

					backoff = 1;
				}
			};

			// Succesfuly locked, no (more) conflicts!
			UnlockedConflictingResource(lockId);
		}

		void unlock()
		{
			mutex = 0;
		}

		bool isLocked()
		{
			return mutex != 0;
		}

	private:
		// Ensure that the mutex variable is on its own 64-byte cache line to avoid false sharing
		volatile int a[16];
		volatile int mutex;
		volatile int b[15];
	};

	class SleeplessLock
	{
	public:
		SleeplessLock()
		{
			mutex = 0;
		}

		bool attemptLock()
		{
			if(!isLocked())
			{
				if(atomicExchange(&mutex, 1) == 0)
				{
					return true;
				}
			}

			return false;
		}

		void lock(LockResourceId lockId, ThreadAnalyzer * ta)
		{
			threadAnalyzer = ta;
			int backoff = 1;
			bool logged = false;
			while(!attemptLock())
			{
				if(!logged)
				{
					// Locked by another thread, log the conflict!
					ConflictWhenLocking(lockId);
					logged = true;
				}

				nop();
				nop();
				nop();
				nop();
				nop();

				nop();
				nop();
				nop();
				nop();
				nop();

				nop();
				nop();
				nop();
				nop();
				nop();

				nop();
				nop();
				nop();
				nop();
				nop();

				nop();
				nop();
				nop();
				nop();
				nop();

				nop();
				nop();
				nop();
				nop();
				nop();

				nop();
				nop();
				nop();
				nop();
				nop();
			};

			// Succesfuly locked, no (more) conflicts!
			UnlockedConflictingResource(lockId);
		}

		void unlock()
		{
			mutex = 0;
		}

		bool isLocked()
		{
			return mutex != 0;
		}

	private:
		// Ensure that the mutex variable is on its own 64-byte cache line to avoid false sharing
		volatile int a[16];
		volatile int mutex;
		volatile int b[15];
	};
}

#endif   // sw_MutexLock_hpp
