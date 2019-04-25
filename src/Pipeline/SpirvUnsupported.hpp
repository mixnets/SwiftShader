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

// This file contains a list of SPIR-V feature descriptions that are currently
// unsupported by SwiftShader.
// The #defines in this file should be used as the first parameter of the
// UNSUPPORTED() macro.

#define SPIRV_FEATURE_EXTENSION "SPIR-V Extension"
#define SPIRV_FEATURE_FLOAT_64 "SPIR-V Float64 Capability"
#define SPIRV_FEATURE_GENERIC_POINTER "SPIR-V GenericPointer Capability"
#define SPIRV_FEATURE_IMAGE_CUBE_ARRAY "SPIR-V ImageCubeArray Capability"
#define SPIRV_FEATURE_NON_FLOAT_32 "SPIR-V Float16 or Float64 Capability"
#define SPIRV_FEATURE_NON_INT_32 "SPIR-V Int16 or Int64 Capability"
#define SPIRV_FEATURE_OPENCL "SPIR-V OpenCL Execution Model"
#define SPIRV_FEATURE_SAMPLE_RATE_SHADING "SPIR-V SampleRateShading Capability"
#define SPIRV_FEATURE_SAMPLED_IMAGE_ARRAY_DYNAMIC_INDEXING "SPIR-V SampledImageArrayDynamicIndexing Capability"