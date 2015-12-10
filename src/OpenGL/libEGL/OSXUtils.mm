#include "OSXUtils.hpp"

#import <Cocoa/Cocoa.h>

#include "common/debug.h"

namespace sw {
namespace OSX {

    bool IsValidWindow(EGLNativeWindowType window) {
        NSObject* obj = reinterpret_cast<NSObject*>(window);
        return (window != nullptr) &&
               ([obj isKindOfClass: [NSView class]] || [obj isKindOfClass: [CALayer class]]);
    }

    void GetNativeWindowSize(EGLNativeWindowType window, int* width, int* height) {
        NSObject* obj = reinterpret_cast<NSObject*>(window);

        if([obj isKindOfClass: [NSView class]]) {
            NSView* view = reinterpret_cast<NSView*>(obj);
            *width = [view bounds].size.width;
            *height = [view bounds].size.height;

        } else if([obj isKindOfClass: [CALayer class]]) {
            CALayer* layer = reinterpret_cast<CALayer*>(obj);
            *width = CGRectGetWidth([layer frame]);
            *height = CGRectGetHeight([layer frame]);

        } else {
            UNREACHABLE(0);
        }
    }

}
}
