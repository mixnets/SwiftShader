#include "ThreadAnalyser.h"

// Global static pointer used to ensure a single instance of the class.
ThreadAnalyzer* ThreadAnalyzer::instance = NULL;

ThreadAnalyzer* ThreadAnalyzer::Instance()
{
	// Lazy loaded on first access
	if(!instance)
	{
		instance = new ThreadAnalyzer;
	}

	return instance;
}

bool ThreadAnalyzer::getThreadAnalysisActive()
{		
	return threadAnalysisActive;
}

void ThreadAnalyzer::setThreadAnalysisActive(bool active)
{
	threadAnalysisActive = active;
}

bool ThreadAnalyzer::getResourceAnalysisActive()
{
	return resourceAnalysisActive;
}

void ThreadAnalyzer::setResourceAnalysisActive(bool active)
{
	resourceAnalysisActive = active;
}

void ThreadAnalyzer::ConflictWhenLocking(LockResourceId id)
{		
	if(!resourceAnalysisActive)
	{
		return;
	}

	if(id < 0 || id > Last - 1 || Locks[id].isInConflict)
	{
		return; // error
	}

	Locks[id].isInConflict = true;
	Locks[id].lockingAttemptTime = std::clock();
	Locks[id].conflictingCalls++;
}

void ThreadAnalyzer::UnlockedConflictingResource(LockResourceId id)
{
	if(!resourceAnalysisActive)
	{
		return;
	}

	if(id < 0 || id > Last - 1 || !Locks[id].isInConflict)
	{
		return; // error
	}

	Locks[id].isInConflict = false;
	Locks[id].totalConflictingTime += std::clock() - Locks[id].lockingAttemptTime;
}

void ThreadAnalyzer::startTask(int threadId, int task)
{	
	if(!threadAnalysisActive)
	{
		return;
	}

	switch(task)
	{
	case PRIMITIVES_TASK:
		if(Tasks[threadId].primitivesStarted)
		{
			return;
		}
		Tasks[threadId].primitivesCalls++;
		Tasks[threadId].primitivesStarted = true;
		Tasks[threadId].timePrimitivesStart = std::clock();
		break;
	case PIXELS_TASK:
		if(Tasks[threadId].pixelsStarted)
		{
			return;
		}
		Tasks[threadId].pixelsCalls++;
		Tasks[threadId].pixelsStarted = true;
		Tasks[threadId].timePixelsStart = std::clock();
		break;
	case SLEEP_TASK:
		if(Tasks[threadId].sleepStarted)
		{
			return;
		}
		Tasks[threadId].sleepCalls++;
		Tasks[threadId].sleepStarted = true;
		Tasks[threadId].timeSleepStart = std::clock();
		break;
	default:
		//TODO: UNREACHABLE
		break;
	}
}

void ThreadAnalyzer::endTask(int threadId, int task)
{
	if(!threadAnalysisActive)
	{
		return;
	}

	switch(task)
	{
	case PRIMITIVES_TASK:
		if(!Tasks[threadId].primitivesStarted)
		{
			return;
		}
		Tasks[threadId].primitivesStarted = false;
		Tasks[threadId].timePrimitives += std::clock() - Tasks[threadId].timePrimitivesStart;
		break;
	case PIXELS_TASK:
		if(!Tasks[threadId].pixelsStarted)
		{
			return;
		}
		Tasks[threadId].pixelsStarted = false;
		Tasks[threadId].timePixels += std::clock() - Tasks[threadId].timePixelsStart;
		break;
	case SLEEP_TASK:
		if(!Tasks[threadId].sleepStarted)
		{
			return;
		}
		Tasks[threadId].sleepStarted = false;
		Tasks[threadId].timeSleep += std::clock() - Tasks[threadId].timeSleepStart;
		break;
	default:
		//TODO: unreachable
		break;
	}
}

LockInfo* ThreadAnalyzer::getLockInfo()
{
	return Locks;
}

ThreadTask* ThreadAnalyzer::getThreadTasks()
{
	return Tasks;
}

const char * ThreadAnalyzer::getLockName(LockResourceId id)
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