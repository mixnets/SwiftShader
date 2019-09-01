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

#define YARN_REG_R1  0x00
#define YARN_REG_R2  0x08
#define YARN_REG_R13 0x10
#define YARN_REG_R14 0x18
#define YARN_REG_R15 0x20
#define YARN_REG_R16 0x28
#define YARN_REG_R17 0x30
#define YARN_REG_R18 0x38
#define YARN_REG_R19 0x40
#define YARN_REG_R20 0x48
#define YARN_REG_R21 0x50
#define YARN_REG_R22 0x58
#define YARN_REG_R23 0x60
#define YARN_REG_R24 0x68
#define YARN_REG_R25 0x70
#define YARN_REG_R26 0x78
#define YARN_REG_R27 0x80
#define YARN_REG_R28 0x88
#define YARN_REG_R29 0x90
#define YARN_REG_R30 0x98
#define YARN_REG_R31 0xa0

#define YARN_REG_R3  0xa8
#define YARN_REG_R4  0xb0

#define YARN_REG_LR  0xb8
#define YARN_REG_CCR 0xc0

#define YARN_REG_FPRS   0xc8
#define YARN_REG_VRSAVE 0x158
#define YARN_REG_VMX    0x160

#ifndef YARN_BUILD_ASM

#include <stdint.h>

struct yarn_fiber_context
{
    // non-volatile registers
    uintptr_t r1;
    uintptr_t r2;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;
    uintptr_t r16;
    uintptr_t r17;
    uintptr_t r18;
    uintptr_t r19;
    uintptr_t r20;
    uintptr_t r21;
    uintptr_t r22;
    uintptr_t r23;
    uintptr_t r24;
    uintptr_t r25;
    uintptr_t r26;
    uintptr_t r27;
    uintptr_t r28;
    uintptr_t r29;
    uintptr_t r30;
    uintptr_t r31;

    // first two parameter registers (r3, r4)
    uintptr_t r3;
    uintptr_t r4;

    // special registers
    uintptr_t lr;
    uintptr_t ccr;

    // non-volatile floating-point registers (f14-f31)
    uintptr_t fprs[18];

    // non-volatile altivec registers
    uint32_t vrsave;
    uintptr_t vmx[12 * 2];
};

// Only the ELFv2 ABI is supported for now.
#if !defined(_CALL_ELF) || (_CALL_ELF != 2)
#error "Only the ppc64 ELFv2 ABI is supported."
#endif

#ifdef __cplusplus
#include <cstddef>
static_assert(offsetof(yarn_fiber_context, r1) == YARN_REG_R1, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r2) == YARN_REG_R2, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r13) == YARN_REG_R13, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r15) == YARN_REG_R15, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r16) == YARN_REG_R16, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r17) == YARN_REG_R17, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r18) == YARN_REG_R18, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r19) == YARN_REG_R19, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r20) == YARN_REG_R20, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r21) == YARN_REG_R21, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r22) == YARN_REG_R22, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r23) == YARN_REG_R23, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r24) == YARN_REG_R24, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r25) == YARN_REG_R25, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r26) == YARN_REG_R26, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r27) == YARN_REG_R27, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r28) == YARN_REG_R28, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r29) == YARN_REG_R29, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r30) == YARN_REG_R30, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r31) == YARN_REG_R31, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, r14) == YARN_REG_R14, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, lr) == YARN_REG_LR, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, ccr) == YARN_REG_CCR, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, fprs) == YARN_REG_FPRS, "Bad register offset");
static_assert((offsetof(yarn_fiber_context, vmx) % 16) == 0);
static_assert(offsetof(yarn_fiber_context, vmx) == YARN_REG_VMX, "Bad register offset");
static_assert(offsetof(yarn_fiber_context, vrsave) == YARN_REG_VRSAVE, "Bad register offset");
#endif // __cplusplus

#endif // YARN_BUILD_ASM
