#include "DebugAndroid.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <cutils/properties.h>

void AndroidEnterDebugger()
{
#ifndef NDEBUG
    static volatile int * const makefault = nullptr;
    char value[PROPERTY_VALUE_MAX];
    property_get("debug.db.uid", value, "-1");
    int debug_uid = atoi(value);
    if (geteuid() < static_cast<uid_t>(debug_uid))
    {
        ALOGE("Entering debugger...");
        // Android is picky about what causes a crash. Null pointer dereferences
        // work well.
        (void)*makefault;
    }
#endif
}
