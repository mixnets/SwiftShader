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

#if defined(__powerpc64__)

#define YARN_BUILD_ASM 1
#include "OSFiber_asm_ppc64.h"

// void yarn_fiber_swap(yarn_fiber_context* from, const yarn_fiber_context* to)
// r3: from
// r4: to
.text
.global yarn_fiber_swap
.align 4
.type yarn_fiber_swap @function
yarn_fiber_swap:

    // Store non-volatile registers
    std 1, YARN_REG_R1(3)
    std 2, YARN_REG_R2(3)
    std 13, YARN_REG_R13(3)
    std 14, YARN_REG_R14(3)
    std 15, YARN_REG_R15(3)
    std 16, YARN_REG_R16(3)
    std 17, YARN_REG_R17(3)
    std 18, YARN_REG_R18(3)
    std 19, YARN_REG_R19(3)
    std 20, YARN_REG_R20(3)
    std 21, YARN_REG_R21(3)
    std 22, YARN_REG_R22(3)
    std 23, YARN_REG_R23(3)
    std 24, YARN_REG_R24(3)
    std 25, YARN_REG_R25(3)
    std 26, YARN_REG_R26(3)
    std 27, YARN_REG_R27(3)
    std 28, YARN_REG_R28(3)
    std 29, YARN_REG_R29(3)
    std 30, YARN_REG_R30(3)
    std 31, YARN_REG_R31(3)

    // Store special registers
    mflr 5
    std 5, YARN_REG_LR(3)
    mfcr 5
    std 5, YARN_REG_CCR(3)

    // Store non-volatile floating point registers
    stfd 14, YARN_REG_FPRS+0x10(3)
    stfd 15, YARN_REG_FPRS+0x18(3)
    stfd 16, YARN_REG_FPRS+0x20(3)
    stfd 17, YARN_REG_FPRS+0x28(3)
    stfd 18, YARN_REG_FPRS+0x30(3)
    stfd 19, YARN_REG_FPRS+0x38(3)
    stfd 20, YARN_REG_FPRS+0x40(3)
    stfd 21, YARN_REG_FPRS+0x48(3)
    stfd 22, YARN_REG_FPRS+0x50(3)
    stfd 23, YARN_REG_FPRS+0x58(3)
    stfd 24, YARN_REG_FPRS+0x60(3)
    stfd 25, YARN_REG_FPRS+0x68(3)
    stfd 26, YARN_REG_FPRS+0x70(3)
    stfd 27, YARN_REG_FPRS+0x78(3)
    stfd 28, YARN_REG_FPRS+0x80(3)
    stfd 29, YARN_REG_FPRS+0x88(3)
    stfd 30, YARN_REG_FPRS+0x90(3)
    stfd 31, YARN_REG_FPRS+0x98(3)

    // Store non-volatile altivec registers
#ifdef __ALTIVEC__
    li 5, YARN_REG_VMX
    stvxl 20, 3, 5
    addi 5, 5, 16
    stvxl 21, 3, 5
    addi 5, 5, 16
    stvxl 22, 3, 5
    addi 5, 5, 16
    stvxl 23, 3, 5
    addi 5, 5, 16
    stvxl 24, 3, 5
    addi 5, 5, 16
    stvxl 25, 3, 5
    addi 5, 5, 16
    stvxl 26, 3, 5
    addi 5, 5, 16
    stvxl 27, 3, 5
    addi 5, 5, 16
    stvxl 28, 3, 5
    addi 5, 5, 16
    stvxl 29, 3, 5
    addi 5, 5, 16
    stvxl 30, 3, 5
    addi 5, 5, 16
    stvxl 31, 3, 5
    addi 5, 5, 16

    mfvrsave 5
    stw 5, YARN_REG_VRSAVE(3)
#endif // __ALTIVEC__

    // Load non-volatile registers
    ld 1, YARN_REG_R1(4)
    ld 2, YARN_REG_R2(4)
    ld 13, YARN_REG_R13(4)
    ld 14, YARN_REG_R14(4)
    ld 15, YARN_REG_R15(4)
    ld 16, YARN_REG_R16(4)
    ld 17, YARN_REG_R17(4)
    ld 18, YARN_REG_R18(4)
    ld 19, YARN_REG_R19(4)
    ld 20, YARN_REG_R20(4)
    ld 21, YARN_REG_R21(4)
    ld 22, YARN_REG_R22(4)
    ld 23, YARN_REG_R23(4)
    ld 24, YARN_REG_R24(4)
    ld 25, YARN_REG_R25(4)
    ld 26, YARN_REG_R26(4)
    ld 27, YARN_REG_R27(4)
    ld 28, YARN_REG_R28(4)
    ld 29, YARN_REG_R29(4)
    ld 30, YARN_REG_R30(4)
    ld 31, YARN_REG_R31(4)

    // Load non-volatile floating point registers
    lfd 14, YARN_REG_FPRS+0x10(4)
    lfd 15, YARN_REG_FPRS+0x18(4)
    lfd 16, YARN_REG_FPRS+0x20(4)
    lfd 17, YARN_REG_FPRS+0x28(4)
    lfd 18, YARN_REG_FPRS+0x30(4)
    lfd 19, YARN_REG_FPRS+0x38(4)
    lfd 20, YARN_REG_FPRS+0x40(4)
    lfd 21, YARN_REG_FPRS+0x48(4)
    lfd 22, YARN_REG_FPRS+0x50(4)
    lfd 23, YARN_REG_FPRS+0x58(4)
    lfd 24, YARN_REG_FPRS+0x60(4)
    lfd 25, YARN_REG_FPRS+0x68(4)
    lfd 26, YARN_REG_FPRS+0x70(4)
    lfd 27, YARN_REG_FPRS+0x78(4)
    lfd 28, YARN_REG_FPRS+0x80(4)
    lfd 29, YARN_REG_FPRS+0x88(4)
    lfd 30, YARN_REG_FPRS+0x90(4)
    lfd 31, YARN_REG_FPRS+0x98(4)

    // Load non-volatile altivec registers
#ifdef __ALTIVEC__
    li 5, YARN_REG_VMX
    lvxl 20, 4, 5
    addi 5, 5, 16
    lvxl 21, 4, 5
    addi 5, 5, 16
    lvxl 22, 4, 5
    addi 5, 5, 16
    lvxl 23, 4, 5
    addi 5, 5, 16
    lvxl 24, 4, 5
    addi 5, 5, 16
    lvxl 25, 4, 5
    addi 5, 5, 16
    lvxl 26, 4, 5
    addi 5, 5, 16
    lvxl 27, 4, 5
    addi 5, 5, 16
    lvxl 28, 4, 5
    addi 5, 5, 16
    lvxl 29, 4, 5
    addi 5, 5, 16
    lvxl 30, 4, 5
    addi 5, 5, 16
    lvxl 31, 4, 5
    addi 5, 5, 16

    lwz 5, YARN_REG_VRSAVE(4)
    mtvrsave 5
#endif // __ALTIVEC__

    // Load parameters and entrypoint
    ld 12, YARN_REG_LR(4)
    ld 3, YARN_REG_R3(4)
    ld 4, YARN_REG_R4(4)
    mtlr 12

    // Branch to entrypoint
    blr

#endif // defined(__powerpc64__)
