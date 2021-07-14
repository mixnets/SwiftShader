//===- subzero/src/IceTargetLoweringX8664Traits.h - x86-64 traits -*- C++ -*-=//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the X8664 Target Lowering Traits.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETARGETLOWERINGX8664TRAITS_H
#define SUBZERO_SRC_ICETARGETLOWERINGX8664TRAITS_H

#include "IceAssembler.h"
#include "IceConditionCodesX86.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceRegistersX8664.h"
#include "IceTargetLowering.h"
#include "IceTargetLoweringX8664.def"
#include "IceTargetLoweringX86RegClass.h"

#include <array>
#include <initializer_list>

namespace Ice {
namespace X8664 {
using namespace ::Ice::X86;

class AssemblerX8664;
struct Insts;
class TargetX8664;
class TargetX8664;

struct TargetX8664Traits {
  using RegisterSet = ::Ice::RegX8664;
  using GPRRegister = RegisterSet::GPRRegister;
  using ByteRegister = RegisterSet::ByteRegister;
  using XmmRegister = RegisterSet::XmmRegister;

  //----------------------------------------------------------------------------
  //     __      ______  __     __  ______  ______  __  __   __  ______
  //    /\ \    /\  __ \/\ \  _ \ \/\  ___\/\  == \/\ \/\ "-.\ \/\  ___\
  //    \ \ \___\ \ \/\ \ \ \/ ".\ \ \  __\\ \  __<\ \ \ \ \-.  \ \ \__ \
  //     \ \_____\ \_____\ \__/".~\_\ \_____\ \_\ \_\ \_\ \_\\"\_\ \_____\
  //      \/_____/\/_____/\/_/   \/_/\/_____/\/_/ /_/\/_/\/_/ \/_/\/_____/
  //
  //----------------------------------------------------------------------------

public:
  /// The number of bits in a byte
  static constexpr uint32_t X86_CHAR_BIT = 8;
  /// Stack alignment. This is defined in IceTargetLoweringX8664.cpp because it
  /// is used as an argument to std::max(), and the default std::less<T> has an
  /// operator(T const&, T const&) which requires this member to have an
  /// address.
  static const uint32_t X86_STACK_ALIGNMENT_BYTES;
  /// Size of the return address on the stack
  static constexpr uint32_t X86_RET_IP_SIZE_BYTES = 8;
  /// The number of different NOP instructions
  static constexpr uint32_t X86_NUM_NOP_VARIANTS = 5;

  /// \name Limits for unrolling memory intrinsics.
  /// @{
  static constexpr uint32_t MEMCPY_UNROLL_LIMIT = 8;
  static constexpr uint32_t MEMMOVE_UNROLL_LIMIT = 8;
  static constexpr uint32_t MEMSET_UNROLL_LIMIT = 8;
  /// @}

  /// Value is in bytes. Return Value adjusted to the next highest multiple of
  /// the stack alignment.
  static uint32_t applyStackAlignment(uint32_t Value) {
    return Utils::applyAlignment(Value, X86_STACK_ALIGNMENT_BYTES);
  }

  /// Return the type which the elements of the vector have in the X86
  /// representation of the vector.
  static Type getInVectorElementType(Type Ty) {
    assert(isVectorType(Ty));
    assert(Ty < TableTypeX8664AttributesSize);
    return TableTypeX8664Attributes[Ty].InVectorElementType;
  }

  /// The following table summarizes the logic for lowering the fcmp
  /// instruction. There is one table entry for each of the 16 conditions.
  ///
  /// The first four columns describe the case when the operands are floating
  /// point scalar values. A comment in lowerFcmp() describes the lowering
  /// template. In the most general case, there is a compare followed by two
  /// conditional branches, because some fcmp conditions don't map to a single
  /// x86 conditional branch. However, in many cases it is possible to swap the
  /// operands in the comparison and have a single conditional branch. Since
  /// it's quite tedious to validate the table by hand, good execution tests are
  /// helpful.
  ///
  /// The last two columns describe the case when the operands are vectors of
  /// floating point values. For most fcmp conditions, there is a clear mapping
  /// to a single x86 cmpps instruction variant. Some fcmp conditions require
  /// special code to handle and these are marked in the table with a
  /// Cmpps_Invalid predicate.
  /// {@
  static const struct TableFcmpType {
    uint32_t Default;
    bool SwapScalarOperands;
    CondX86::BrCond C1, C2;
    bool SwapVectorOperands;
    CondX86::CmppsCond Predicate;
  } TableFcmp[];
  static const size_t TableFcmpSize;
  /// @}

  /// The following table summarizes the logic for lowering the icmp instruction
  /// for i32 and narrower types. Each icmp condition has a clear mapping to an
  /// x86 conditional branch instruction.
  /// {@
  static const struct TableIcmp32Type {
    CondX86::BrCond Mapping;
  } TableIcmp32[];
  static const size_t TableIcmp32Size;
  /// @}

  /// The following table summarizes the logic for lowering the icmp instruction
  /// for the i64 type. For Eq and Ne, two separate 32-bit comparisons and
  /// conditional branches are needed. For the other conditions, three separate
  /// conditional branches are needed.
  /// {@
  static const struct TableIcmp64Type {
    CondX86::BrCond C1, C2, C3;
  } TableIcmp64[];
  static const size_t TableIcmp64Size;
  /// @}

  static CondX86::BrCond getIcmp32Mapping(InstIcmp::ICond Cond) {
    assert(Cond < TableIcmp32Size);
    return TableIcmp32[Cond].Mapping;
  }

  static const struct TableTypeX8664AttributesType {
    Type InVectorElementType;
  } TableTypeX8664Attributes[];
  static const size_t TableTypeX8664AttributesSize;
};

} // end of namespace X8664
} // end of namespace Ice

#endif // SUBZERO_SRC_ICETARGETLOWERINGX8664TRAITS_H
