#ifndef rad_Object_hpp
#define rad_Object_hpp

#include "Common/debug.h"
#include "Common/Thread.hpp"

namespace es2
{
	class Object
	{
	public:
		Object()
		{
			referenceCount = 1;
		}

		void reference()
		{
			sw::atomicIncrement(&referenceCount);
		}

		void release()
		{
			ASSERT(referenceCount > 0);

			if(referenceCount > 0)
			{
				sw::atomicDecrement(&referenceCount);
			}

			if(referenceCount == 0)
			{
				delete this;
			}
		}

	protected:
		virtual ~Object()
		{
			ASSERT(referenceCount == 0);
		}

	private:
		volatile int referenceCount;
	};
}

#endif   // rad_Object_hpp
