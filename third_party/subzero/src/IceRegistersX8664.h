//===- subzero/src/IceRegistersX8664.h - Register information ---*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the registers and their encodings for x86-64.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEREGISTERSX8664_H
#define SUBZERO_SRC_ICEREGISTERSX8664_H

#include "IceDefs.h"
#include "IceInstX8664.def"
#include "IceTypes.h"

namespace Ice {

class RegX8664 {
public:
  /// An enum of every register. The enum value may not match the encoding used
  /// to binary encode register operands in instructions.
  enum AllRegisters {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  val,
    REGX8664_TABLE
#undef X
        Reg_NUM
  };

  /// An enum of GPR Registers. The enum value does match the encoding used to
  /// binary encode register operands in instructions.
  enum GPRRegister {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  Encoded_##val = encode,
    REGX8664_GPR_TABLE
#undef X
        Encoded_Not_GPR = -1
  };

  /// An enum of XMM Registers. The enum value does match the encoding used to
  /// binary encode register operands in instructions.
  enum XmmRegister {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  Encoded_##val = encode,
    REGX8664_XMM_TABLE
#undef X
        Encoded_Not_Xmm = -1
  };

  /// An enum of Byte Registers. The enum value does match the encoding used to
  /// binary encode register operands in instructions.
  enum ByteRegister {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  Encoded_8_##val = encode,
    REGX8664_BYTEREG_TABLE
#undef X
        Encoded_Not_ByteReg = -1
  };

  static inline const char *getRegName(RegNumT RegNum) {
    static const char *const RegNames[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  name,
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    return RegNames[RegNum];
  }

  static inline GPRRegister getEncodedGPR(RegNumT RegNum) {
    static const GPRRegister GPRRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  GPRRegister(isGPR ? encode : GPRRegister::Encoded_Not_GPR),
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    assert(GPRRegs[RegNum] != GPRRegister::Encoded_Not_GPR);
    return GPRRegs[RegNum];
  }

  static inline ByteRegister getEncodedByteReg(RegNumT RegNum) {
    static const ByteRegister ByteRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  ByteRegister(is8 ? encode : ByteRegister::Encoded_Not_ByteReg),
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    assert(ByteRegs[RegNum] != ByteRegister::Encoded_Not_ByteReg);
    return ByteRegs[RegNum];
  }

  static inline bool isXmm(RegNumT RegNum) {
    static const bool IsXmm[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  isXmm,
        REGX8664_TABLE
#undef X
    };
    return IsXmm[RegNum];
  }

  static inline XmmRegister getEncodedXmm(RegNumT RegNum) {
    static const XmmRegister XmmRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  XmmRegister(isXmm ? encode : XmmRegister::Encoded_Not_Xmm),
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    assert(XmmRegs[RegNum] != XmmRegister::Encoded_Not_Xmm);
    return XmmRegs[RegNum];
  }

  static inline uint32_t getEncoding(RegNumT RegNum) {
    static const uint32_t Encoding[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  encode,
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    return Encoding[RegNum];
  }

  static inline RegNumT getBaseReg(RegNumT RegNum) {
    static const RegNumT BaseRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  base,
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    return BaseRegs[RegNum];
  }

private:
  static inline RegNumT getFirstGprForType(Type Ty) {
    switch (Ty) {
    default:
      llvm_unreachable("Invalid type for GPR.");
    case IceType_i1:
    case IceType_i8:
      return Reg_al;
    case IceType_i16:
      return Reg_ax;
    case IceType_i32:
      return Reg_eax;
    case IceType_i64:
      return Reg_rax;
    }
  }

public:
  static inline RegNumT getGprForType(Type Ty, RegNumT RegNum) {
    assert(RegNum.hasValue());

    if (!isScalarIntegerType(Ty)) {
      return RegNum;
    }

    assert(Ty == IceType_i1 || Ty == IceType_i8 || Ty == IceType_i16 ||
           Ty == IceType_i32 || Ty == IceType_i64);

    if (RegNum == Reg_ah) {
      assert(Ty == IceType_i8);
      return RegNum;
    }

    assert(RegNum != Reg_bh);
    assert(RegNum != Reg_ch);
    assert(RegNum != Reg_dh);

    const RegNumT FirstGprForType = getFirstGprForType(Ty);

    switch (RegNum) {
    default:
      llvm::report_fatal_error("Unknown register.");
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  case val: {                                                     \
    if (!isGPR)                                                                \
      return val;                                                 \
    assert((is64) || (is32) || (is16) || (is8) ||                              \
           getBaseReg(val) == Reg_rsp);              \
    constexpr AllRegisters FirstGprWithRegNumSize =               \
        ((is64) || val == Reg_rsp)                   \
            ? Reg_rax                                             \
            : (((is32) || val == Reg_esp)            \
                   ? Reg_eax                                      \
                   : (((is16) || val == Reg_sp)      \
                          ? Reg_ax                                \
                          : Reg_al));                             \
    const auto NewRegNum =                                                     \
        RegNumT::fixme(RegNum - FirstGprWithRegNumSize + FirstGprForType);     \
    assert(getBaseReg(RegNum) == getBaseReg(NewRegNum) &&                      \
           "Error involving " #val);                                           \
    return NewRegNum;                                                          \
  }
      REGX8664_TABLE
#undef X
    }
  }
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEREGISTERSX8664_H
