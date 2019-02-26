// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#ifndef sw_DescriptorSetsLayout_hpp
#define sw_DescriptorSetsLayout_hpp

#include <stddef.h>

namespace sw
{
	// DescriptorSetsLayout is a pure virtual interface that provides layout
	// information for descriptor sets.
	class DescriptorSetsLayout
	{
	public:
		// getNumDescriptorSets returns the total number of descriptor sets in
		// the layout.
		virtual size_t getNumDescriptorSets() const = 0;

		// getBindingOffset returns the offset in bytes from the descriptor set
		// pointer to the specified binding.
		virtual size_t getBindingOffset(size_t descriptorSet, size_t binding) const = 0;
	};
}

#endif  // sw_DescriptorSetsLayout_hpp
