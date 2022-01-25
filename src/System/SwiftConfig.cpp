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
#include "Debug.hpp"

#include <algorithm>

namespace sw {

std::string toLowerStr(const std::string &str)
{
	std::string lower = str;
	std::transform(lower.begin(), lower.end(), lower.begin(),
	               [](unsigned char c) { return std::tolower(c); });
	return lower;
}

Configuration readConfigurationFromFile()
{
	Configurator ini("SwiftShader.ini");
	Configuration config{};
	config.threadCount = ini.getInteger<uint32_t>("Processor", "ThreadCount", 0);
	config.affinityMask = ini.getInteger<uint64_t>("Processor", "AffinityMask", 0xffffffffffffffff);
	if(config.affinityMask == 0)
	{
		warn("Affinity mask is empty, using all-cores affinity\n");
		config.affinityMask = 0xffffffffffffffff;
	}
	std::string affinityPolicy = toLowerStr(ini.getValue("Processor", "AffinityPolicy", "any"));
	if(affinityPolicy == "one")
	{
		config.affinityPolicy = Configuration::AffinityPolicy::OneOf;
	}
	else
	{
		// Default.
		config.affinityPolicy = Configuration::AffinityPolicy::AnyOf;
	}

	return config;
}

const Configuration &getConfiguration()
{
	static Configuration config = readConfigurationFromFile();
	return config;
}
}  // namespace sw