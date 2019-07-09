#include "mt.hpp"

namespace mt {

thread_local Fiber* Fiber::current = nullptr;

} // namespace mt