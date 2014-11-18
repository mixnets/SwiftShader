// ExceptionsSymbols.cpp
//
// Android source toolchain only has minimal C++ runtime support (libstdc++).
// In particular, exception is not available. The symbols listed here are
// missing when building for Android and since they are all related to
// exception, they are defined in this file merely to make linker happy. This
// should be safe as long as SwiftShader does not try to throw any exception.
// DO NOT use this file when building for any platform other than Android.
//
// TODO(pinghao): Get rid of them.

#if defined(__ANDROID__) || defined(ANDROID)

void *__cxa_begin_catch;

namespace std {
  void terminate() {};
}

void *__gxx_personality_v0;

#endif
