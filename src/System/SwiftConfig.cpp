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

#include <memory>
#include <mutex>

namespace sw {
std::unique_ptr<Configuration> config = nullptr;
std::mutex readConfigMutex;

void readConfigurationFromFile()
{
	const std::lock_guard<std::mutex> lock(readConfigMutex);

	// If the config was already parsed by the time we acquired the lock, do nothing.
	if(config != nullptr)
	{
		return;
	}

	Configurator ini("SwiftShader.ini");

	Configuration *configPtr = new Configuration();
	configPtr->threadCount = ini.getInteger("Processor", "ThreadCount", 0);

	config.reset(configPtr);
}

Configuration *getConfiguration()
{
	if(config == nullptr)
	{
		readConfigurationFromFile();
	}
	return config.get();
}
}  // namespace sw