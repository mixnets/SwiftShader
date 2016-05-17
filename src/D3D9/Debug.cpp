// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#include "Debug.hpp"

#include "Timer.hpp"

FILE *Trace::trace = 0;
int Trace::indent = 0;
bool Trace::comma = false;

int Trace::frame = 0;
double Trace::frequency = 1000000.0 / sw::Timer::frequency();

Trace::Trace(const char *function, const void *object, const char *format, ...) : tracing(trace)
{
	if(false)
	{
		FILE *file = fopen("debug.txt", "a");

		if(file)
		{
			for(int i = 0; i < indent; i++) fprintf(file, " ");

			fprintf(file, "%s[0x%0.8X](", function, object);

			va_list vararg;
			va_start(vararg, format);
			vfprintf(file, format, vararg);
			va_end(vararg);

			fprintf(file, ")\n");

			fclose(file);
		}
	}

	if(tracing)
	{
		char arguments[4096];

 		va_list vararg;
 		va_start(vararg, format);
		vsprintf(arguments, format, vararg);
 		va_end(vararg);
 
		log('B', function, object, arguments);
	}

	indent++;
}

Trace::~Trace()
{
	indent--;

	if(tracing)
	{
		log('E', "", 0, "");
	}
}

void Trace::log(char type, const char *function, const void *object, const char *arguments)
{
	if(trace)
	{
		double time = (double)sw::Timer::counter() * frequency;
		
		int pid = (int)GetCurrentProcessId();
		int tid = (int)GetCurrentThreadId();

		if(object)
		{
			fprintf(trace, "%s{\"ph\": \"%c\", \"ts\": %f, \"pid\": %d, \"tid\": %d, \"name\": \"%s\", \"args\":{\"this 0x%0.8X\" : 0, \"%s\": 0}}",
					comma ? ",\n" : "", type, time, pid, tid, function, object, arguments);
		}
		else
		{
			fprintf(trace, "%s{\"ph\": \"%c\", \"ts\": %f, \"pid\": %d, \"tid\": %d, \"name\": \"%s\", \"args\":{\"%s\": 0}}",
					comma ? ",\n" : "", type, time, pid, tid, function, arguments);
		}

		comma = true;
	}
}

void Trace::nextFrame()
{
	if(GetKeyState(VK_CAPITAL) & 0x0001)   // Caps Lock
	{
		if(!trace)
		{
			trace = fopen("trace.json", "wt");
			setvbuf(trace, 0, _IOFBF, 64 * 4096);

			fprintf(trace, "{\"traceEvents\":[\n");
			comma = false;

			char frameNum[32];
			sprintf(frameNum, "Frame %d", frame);
			log('B', frameNum, 0, "");
		}
		else
		{
			char frameNum[32];
			sprintf(frameNum, "Frame %d", frame - 1);
			log('E', frameNum, 0, "");

			sprintf(frameNum, "Frame %d", frame);
			log('B', frameNum, 0, "");
		}
	}
	else if(trace)
	{
		char frameNum[32];
		sprintf(frameNum, "Frame %d", frame - 1);
		log('E', frameNum, 0, "");

		fprintf(trace, "\n]}\n");
		fclose(trace);
		trace = 0;
	}

	frame++;
}
