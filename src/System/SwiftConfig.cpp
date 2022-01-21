// Copyright 2022 The SwiftShader Authors. All Rights Reserved.
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

#include "SwiftConfig.hpp"
#include "Configurator.hpp"

#include <atomic>
#include <mutex>

namespace sw {
Configuration globalConfig;
std::atomic<bool> configParsed;

void readConfigurationFromFile()
{
	static std::mutex configParseMutex;
	const std::lock_guard lock{ configParseMutex };

	// If the configuration has been already parsed by the time
	// we acquire the lock, do nothing.
	if(configParsed.load(std::memory_order_acquire))
	{
		return;
	}

	Configurator ini("SwiftShader.ini");

	Configuration config{};
	config.threadCount = ini.getInteger("Processor", "ThreadCount", 0);

	globalConfig = config;
	configParsed.store(true, std::memory_order_release);
}

const Configuration &getConfiguration()
{
	if(!configParsed.load(std::memory_order_acquire))
	{
		readConfigurationFromFile();
	}
	return globalConfig;
}
}  // namespace sw