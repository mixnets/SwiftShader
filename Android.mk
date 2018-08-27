#
# Copyright 2015 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

SWIFTSHADER_LLVM_VERSION := 3

# Use Subzero as the Reactor JIT back-end on ARM, else LLVM.
ifeq ($(TARGET_ARCH),$(filter $(TARGET_ARCH),arm))
use_subzero := true
endif

# Subzero requires full C++11 support, which is available from Marshmallow and up.
ifdef use_subzero
ifeq ($(shell test $(PLATFORM_SDK_VERSION) -lt 23 && echo PreMarshmallow),PreMarshmallow)
unsupported_build := true
endif
endif

ifeq ($(SWIFTSHADER_LLVM_VERSION),3)
ifneq ($(TARGET_ARCH),$(filter $(TARGET_ARCH),x86 x86_64 arm))
unsupported_build := true
endif
endif

ifeq ($(SWIFTSHADER_LLVM_VERSION),7)
ifneq ($(TARGET_ARCH),$(filter $(TARGET_ARCH),x86 x86_64 arm arm64))
unsupported_build := true
endif
use_subzero :=
endif

ifndef unsupported_build
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
