// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#ifndef DESCRIPTOR_VIEW_HPP_
#define DESCRIPTOR_VIEW_HPP_

namespace vk {

class Device;

class DescriptorView
{
public:
	enum AccessType {
		READ_ACCESS = 0x1,
		WRITE_BUFFER_ACCESS = 0x2,
		WRITE_IMAGE_ACCESS = 0x4,
		WRITE_ANY_ACCESS = WRITE_BUFFER_ACCESS | WRITE_IMAGE_ACCESS,
	};

	virtual ~DescriptorView() { /* FIXME: unregister here */ }
	virtual void notify(AccessType accessType) = 0;
};

}

#endif // DESCRIPTOR_VIEW_HPP_