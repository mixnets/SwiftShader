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

// Singleton implementation as there is at most 1 instance at any given time
class ThreadAnalyzer
{
public:	
	static ThreadAnalyzer* Instance();
	bool getThreadAnalysisActive();
	void setThreadAnalysisActive(bool active);
	bool getResourceAnalysisActive();
	void setResourceAnalysisActive(bool active);
	void ConflictWhenLocking(LockResourceId id);
	void UnlockedConflictingResource(LockResourceId id);
	void startTask(int threadId, int task);
	void endTask(int threadId, int task);
	LockInfo* getLockInfo();
	ThreadTask* getThreadTasks();
	static const char * getLockName(LockResourceId id);
private:
	ThreadAnalyzer(){}
	static ThreadAnalyzer * instance;

	// In order not to get accidental copies of it
	ThreadAnalyzer(ThreadAnalyzer const&);
	void operator=(ThreadAnalyzer const&);

	LockInfo Locks[Last];
	ThreadTask Tasks[THREAD_AMOUNT];
	bool threadAnalysisActive;
	bool resourceAnalysisActive;
};

#endif