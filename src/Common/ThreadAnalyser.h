#pragma once

#include <ctime>
#include <stdio.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <stdio.h>
#include <windows.h>
#include <intrin.h>

#ifndef _THREAD_ANALYZER
#define _THREAD_ANALYZER

#define _ANALYZER_ON 1

#define PRIMITIVES_TASK 0
#define PIXELS_TASK 1
#define SLEEP_TASK 2

#define THREAD_AMOUNT 16

enum LockResourceId
{
	RendererVertexStream,
	RendererSync,
	RendererIndexBufferLock,
	RendererTextureLock,
	RendererSchedulerLock,
	NucleusCodegenMutexLock,
	BlitterCriticalSection,

	Mipmaps, // all of them, for now
	Texture2d, // all of them, for now
	Texture2dProxy, // all of them, for now
	TextureCubeMap, // all of them, for now

	ContextVertexDataManager,
	ContextIndexDataManager,

	DeviceDepthStencil,
	DeviceRenderTarget,

	ResourceManagerBuffer,

	SurfaceBackBuffer,
	SurfaceDepthStencil,

	Logo,

	SwiftConfig,
	Last = SwiftConfig
};


struct ThreadTask
{
	int primitivesCalls = 0;
	int pixelsCalls = 0;
	int sleepCalls = 0;
	bool primitivesStarted = false;
	clock_t timePrimitivesStart = 0;
	clock_t timePrimitives = 0;
	bool pixelsStarted = false;
	clock_t timePixelsStart = 0;
	clock_t timePixels = 0;
	bool sleepStarted = false;
	clock_t timeSleepStart = 0;
	clock_t timeSleep = 0;
};

struct LockInfo
{
	int conflictingCalls = 0;
	bool isInConflict = false;
	clock_t lockingAttemptTime = 0;
	clock_t totalConflictingTime = 0;
};

class ThreadAnalyzer
{
public:
	ThreadAnalyzer()
	{

	}
	
	LockInfo Locks[Last];
	ThreadTask Tasks[THREAD_AMOUNT]; //16 threads
	bool threadAnalysisActive;
};

static ThreadAnalyzer * threadAnalyzer;
static std::mutex mtx;
static void initThreadAnalyzer()
{
	mtx.lock();
	threadAnalyzer = new ThreadAnalyzer();
	mtx.unlock();
}

static void deleteThreadAnalalyzer()
{
	mtx.lock();
	delete threadAnalyzer;
	mtx.unlock();
}

static void setAnalysisActive(bool isActive)
{
#ifndef _ANALYZER_ON
	return;
#endif

	static bool initialized = false;

	if(!initialized)
	{
		mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return;
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}
	}

	/*std::lock_guard<std::mutex> lock(mtx);
	std::lock_guard<std::mut> guard(mtx);*/
	for(int i = 0; i <= LockResourceId::Last; i++)
	{
		threadAnalyzer->Locks->conflictingCalls = 0;
		threadAnalyzer->Locks->isInConflict = false;
		threadAnalyzer->Locks->lockingAttemptTime = 0;
		threadAnalyzer->Locks->totalConflictingTime = 0;
	}

	threadAnalyzer->threadAnalysisActive = isActive;
}

static bool getAnalysisActive()
{
#ifndef _ANALYZER_ON
	return;
#endif

	static bool initialized = false;

	if(!initialized)
	{
		mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return false;
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}
	}

	//std::lock_guard<std::mutex> lock(mtx);
	//std::shared_lock<std::shared_timed_mutex> guard(mtx);
	return threadAnalyzer->threadAnalysisActive;
}

static void ConflictWhenLocking(LockResourceId id)
{
#ifndef _ANALYZER_ON
	return;
#endif

	static bool initialized = false;

	if(threadAnalyzer == NULL/*!initialized*/)
	{
		return;
		/*mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return;
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}*/
	}

	//std::lock_guard<std::mutex> guard(mtx);
	if(!threadAnalyzer->threadAnalysisActive)
	{
		return;
	}

	if(id < 0 || id > Last - 1 || threadAnalyzer->Locks[id].isInConflict)
	{
		return; // error
	}

	threadAnalyzer->Locks[id].isInConflict = true;
	threadAnalyzer->Locks[id].lockingAttemptTime = std::clock();
	threadAnalyzer->Locks[id].conflictingCalls++;
}

static void UnlockedConflictingResource(LockResourceId id)
{
#ifndef _ANALYZER_ON
	return;
#endif

	static bool initialized = false;

	if(threadAnalyzer == NULL/*!initialized*/)
	{
		return;
		/*mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return;
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}*/
	}

	//std::lock_guard<std::mutex> guard(mtx);
	if(!threadAnalyzer->threadAnalysisActive)
	{
		return;
	}

	if(id < 0 || id > Last - 1 || !threadAnalyzer->Locks[id].isInConflict)
	{
		return; // error
	}

	threadAnalyzer->Locks[id].isInConflict = false;
	threadAnalyzer->Locks[id].totalConflictingTime += std::clock() - threadAnalyzer->Locks[id].lockingAttemptTime;
}

static void startTask(int threadId, int task)
{
#ifndef _ANALYZER_ON
	return;
#endif

	static bool initialized = false;

	if(threadAnalyzer == NULL/*!initialized*/)
	{
		return;
		/*mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return;
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}*/
	}

	if(!threadAnalyzer->threadAnalysisActive)
	{
		return;
	}

	switch(task)
	{
	case PRIMITIVES_TASK:
		if(threadAnalyzer->Tasks[threadId].primitivesStarted)
		{
			return;
		}
		threadAnalyzer->Tasks[threadId].primitivesCalls++;
		threadAnalyzer->Tasks[threadId].primitivesStarted = true;
		threadAnalyzer->Tasks[threadId].timePrimitivesStart = std::clock();
		break;
	case PIXELS_TASK:
		if(threadAnalyzer->Tasks[threadId].pixelsStarted)
		{
			return;
		}
		threadAnalyzer->Tasks[threadId].pixelsCalls++;
		threadAnalyzer->Tasks[threadId].pixelsStarted = true;
		threadAnalyzer->Tasks[threadId].timePixelsStart = std::clock();
		break;
	case SLEEP_TASK:
		if(threadAnalyzer->Tasks[threadId].sleepStarted)
		{
			return;
		}
		threadAnalyzer->Tasks[threadId].sleepCalls++;
		threadAnalyzer->Tasks[threadId].sleepStarted = true;
		threadAnalyzer->Tasks[threadId].timeSleepStart = std::clock();
		break;
	default:
		break;
	}
}

static void endTask(int threadId, int task)
{
#ifndef _ANALYZER_ON
	return;
#endif

	static bool initialized = false;

	if(threadAnalyzer == NULL/*!initialized*/)
	{
		return;
		/*mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return;
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}*/
	}

	if(!threadAnalyzer->threadAnalysisActive)
	{
		return;
	}

	switch(task)
	{
	case PRIMITIVES_TASK:
		if(!threadAnalyzer->Tasks[threadId].primitivesStarted)
		{
			return;
		}
		threadAnalyzer->Tasks[threadId].primitivesStarted = false;
		threadAnalyzer->Tasks[threadId].timePrimitives += std::clock() - threadAnalyzer->Tasks[threadId].timePrimitivesStart;
		break;
	case PIXELS_TASK:
		if(!threadAnalyzer->Tasks[threadId].pixelsStarted)
		{
			return;
		}
		threadAnalyzer->Tasks[threadId].pixelsStarted = false;
		threadAnalyzer->Tasks[threadId].timePixels += std::clock() - threadAnalyzer->Tasks[threadId].timePixelsStart;
		break;
	case SLEEP_TASK:
		if(!threadAnalyzer->Tasks[threadId].sleepStarted)
		{
			return;
		}
		threadAnalyzer->Tasks[threadId].sleepStarted = false;
		threadAnalyzer->Tasks[threadId].timeSleep += std::clock() - threadAnalyzer->Tasks[threadId].timeSleepStart;
		break;
	default:
		break;
	}
}

static const char * getLockName(LockResourceId id)
{
	switch(id)
	{
	case RendererVertexStream: return "Renderer Vertex Stream"; break;
	case RendererSync: return "Renderer Sync"; break;
	case RendererIndexBufferLock: return "Renderer Index Buffer Lock"; break;
	case RendererTextureLock: return "Renderer Texture Lock"; break;
	case RendererSchedulerLock: return "Renderer Scheduler Lock"; break;
	case NucleusCodegenMutexLock: return "Nucleus Codegen Mutex Lock"; break;
	case BlitterCriticalSection: return "Blitter Critical Section"; break;
	case Mipmaps: return "Mipmaps"; break;
	case Texture2d: return "Texture2d"; break;
	case Texture2dProxy: return "Texture2d Proxy"; break;
	case TextureCubeMap: return "Texture Cube Map"; break;
	case ContextVertexDataManager: return "Context Vertex Data Manager"; break;
	case ContextIndexDataManager: return "Context Index Data Manager"; break;
	case DeviceDepthStencil: return "Device Depth Stencil"; break;
	case DeviceRenderTarget: return "Device Render Target"; break;
	case ResourceManagerBuffer: return "Resource Manager Buffer"; break;
	case SurfaceBackBuffer: return "Surface Back Buffer"; break;
	case SurfaceDepthStencil: return "Surface Depth Stencil"; break;
	case Logo: return "Logo"; break;
	case SwiftConfig: return "SwiftConfig"; break;
	default: return "Unidentified"; break;
	}
}

static const char * getLongestLock()
{
	static bool initialized = false;

	if(!initialized)
	{
		mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return "";
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}
	}

	LockResourceId longest = LockResourceId::Logo;
	for(int i = 0; i < LockResourceId::Last; i++)
	{
		if(threadAnalyzer->Locks[i].totalConflictingTime >= threadAnalyzer->Locks[longest].totalConflictingTime)
		{
			longest = (LockResourceId)i;
		}
	}

	return getLockName(longest);
}

static int getLongestPrimitivesThread()
{
	static bool initialized = false;

	if(!initialized)
	{
		mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return -1;
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}
	}

	int longest = 0;
	for(int i = 0; i < THREAD_AMOUNT; i++)
	{
		if(threadAnalyzer->Tasks[i].timePrimitives >= threadAnalyzer->Tasks[longest].timePrimitives)
		{
			longest = i;
		}
	}

	return longest;
}

static LockInfo* getLockInfo()
{
	static bool initialized = false;
	if(!initialized)
	{
		mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return NULL;
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}
	}

	return threadAnalyzer->Locks;
}

static ThreadTask* getThreadTasks()
{
	static bool initialized = false;
	if(!initialized)
	{
		mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return NULL;
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}
	}

	return threadAnalyzer->Tasks;
}

static int getLongestPixelsThread()
{
	static bool initialized = false;

	if(!initialized)
	{
		mtx.lock();

		if(threadAnalyzer == NULL)
		{
			mtx.unlock();
			return -1;
		}
		else
		{
			mtx.unlock();
			initialized = true;
		}
	}

	int longest = 0;
	for(int i = 0; i < THREAD_AMOUNT; i++)
	{
		if(threadAnalyzer->Tasks[i].timePixels >= threadAnalyzer->Tasks[longest].timePixels)
		{
			longest = i;
		}
	}

	return longest;
}
//private:
	//struct LockInfo
	//{
	//	int conflictingCalls = 0;
	//	bool isInConflict = false;
	//	clock_t lockingAttemptTime = 0;
	//	clock_t totalConflictingTime = 0;
	//};
	//LockInfo Locks[Last];
//};

//#ifndef _ANALYZER_CREATED
//#define _ANALYZER_CREATED
//
//static bool threadAnalysisActive = false;
//std::mutex mtx;
////static ThreadAnalyzer * threadAnalyzer = new ThreadAnalyzer();
//#endif
#endif