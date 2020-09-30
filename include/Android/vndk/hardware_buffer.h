/*
 * Copyright 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

enum {
    AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM           = 1,
    AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM           = 2,
    AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM             = 3,
    AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM             = 4,
    AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM           = 5,
    AHARDWAREBUFFER_FORMAT_YCbCr_422_SP             = 0x10,
    AHARDWAREBUFFER_FORMAT_YCrCb_420_SP             = 0x11,
    AHARDWAREBUFFER_FORMAT_YCbCr_422_I              = 0x14,
    AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT       = 0x16,
    AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM        = 0x2b,
    AHARDWAREBUFFER_FORMAT_RAW16                    = 0x20,
    AHARDWAREBUFFER_FORMAT_BLOB                     = 0x21,
    AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED   = 0x22,
    AHARDWAREBUFFER_FORMAT_RAW_OPAQUE               = 0x24,
    AHARDWAREBUFFER_FORMAT_RAW10                    = 0x25,
    AHARDWAREBUFFER_FORMAT_RAW12                    = 0x26,
    AHARDWAREBUFFER_FORMAT_D16_UNORM                = 0x30,
    AHARDWAREBUFFER_FORMAT_D24_UNORM                = 0x31,
    AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT        = 0x32,
    AHARDWAREBUFFER_FORMAT_D32_FLOAT                = 0x33,
    AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT        = 0x34,
    AHARDWAREBUFFER_FORMAT_S8_UINT                  = 0x35,
    AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420             = 0x23,
    AHARDWAREBUFFER_FORMAT_Y8                       = 0x20203859,
    AHARDWAREBUFFER_FORMAT_Y16                      = 0x20363159,
    AHARDWAREBUFFER_FORMAT_YV12                     = 0x32315659,
};
