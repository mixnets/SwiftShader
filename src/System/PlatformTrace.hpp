// Copyright 2023 The SwiftShader Authors. All Rights Reserved.
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

#if defined(SWIFTSHADER_DISABLE_PLATFORM_TRACE) || !defined(__ANDROID__)

#	define SCOPED_PLATFORM_TRACE(const char *format, ...) (void(0))

#else

#	include <stdarg.h>

#	define ATRACE_TAG (ATRACE_TAG_GRAPHICS | ATRACE_TAG_HAL)
#	include <utils/Trace.h>

namespace swiftshader_internal {

class ScopedPlatformTrace
{
public:
	ScopedPlatformTrace(const char *fmt, ...)
	{
		if(!ATRACE_ENABLED())
		{
			return;
		}

		const int BUFFER_SIZE = 256;
		va_list ap;
		char buf[BUFFER_SIZE];

		va_start(ap, fmt);
		vsnprintf(buf, BUFFER_SIZE, fmt, ap);
		va_end(ap);

		ATRACE_BEGIN(buf);
	}

	~ScopedPlatformTrace()
	{
		if(!ATRACE_ENABLED())
		{
			return;
		}

		ATRACE_END();
	}
};

}  // namespace swiftshader_internal

#	define SCOPED_PLATFORM_TRACE(format, ...) swiftshader_internal::ScopedPlatformTrace scoped_platform_trace(format, ##__VA_ARGS__);

#endif