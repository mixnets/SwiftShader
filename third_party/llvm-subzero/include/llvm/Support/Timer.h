//===-- llvm/Support/Timer.h - Interval Timing Support ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_TIMER_H
#define LLVM_SUPPORT_TIMER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/DataTypes.h"
#include <cassert>
#include <string>
#include <utility>
#include <vector>

namespace llvm {

class Timer;
class TimerGroup;
class raw_ostream;

class TimeRecord {
  double WallTime;       // Wall clock time elapsed in seconds
  double UserTime;       // User time elapsed
  double SystemTime;     // System time elapsed
  ssize_t MemUsed;       // Memory allocated (in bytes)
public:
  TimeRecord() : WallTime(0), UserTime(0), SystemTime(0), MemUsed(0) {}

  /// getCurrentTime - Get the current time and memory usage.  If Start is true
  /// we get the memory usage before the time, otherwise we get time before
  /// memory usage.  This matters if the time to get the memory usage is
  /// significant and shouldn't be counted as part of a duration.
  static TimeRecord getCurrentTime(bool Start = true);

  double getProcessTime() const { return UserTime + SystemTime; }
  double getUserTime() const { return UserTime; }
  double getSystemTime() const { return SystemTime; }
  double getWallTime() const { return WallTime; }
  ssize_t getMemUsed() const { return MemUsed; }

  // operator< - Allow sorting.
  bool operator<(const TimeRecord &T) const {
    // Sort by Wall Time elapsed, as it is the only thing really accurate
    return WallTime < T.WallTime;
  }

  void operator+=(const TimeRecord &RHS) {
    WallTime   += RHS.WallTime;
    UserTime   += RHS.UserTime;
    SystemTime += RHS.SystemTime;
    MemUsed    += RHS.MemUsed;
  }
  void operator-=(const TimeRecord &RHS) {
    WallTime   -= RHS.WallTime;
    UserTime   -= RHS.UserTime;
    SystemTime -= RHS.SystemTime;
    MemUsed    -= RHS.MemUsed;
  }

  /// Print the current time record to \p OS, with a breakdown showing
  /// contributions to the \p Total time record.
  void print(const TimeRecord &Total, raw_ostream &OS) const;
};

} // End llvm namespace

#endif
