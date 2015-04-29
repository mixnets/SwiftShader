/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef GRALLOC_PRIV_H_
#define GRALLOC_PRIV_H_

#include <stdint.h>
#include <limits.h>
#include <sys/cdefs.h>
#include <hardware/gralloc.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include <cutils/native_handle.h>

#include <linux/fb.h>

/*****************************************************************************/

struct private_module_t;
struct private_handle_t;

inline size_t roundUpToPageSize(size_t x) {
    return (x + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);
}

int mapUserspaceFrameBufferLocked(struct private_module_t* module);
int terminateBuffer(gralloc_module_t const* module, private_handle_t* hnd);
int mapBuffer(gralloc_module_t const* module, private_handle_t* hnd);


struct private_module_t {
    gralloc_module_t base;

    private_handle_t* framebuffer;
    int remoter_socket;
    uint32_t flags;
    uint32_t numBuffers;
    uint32_t bufferMask;
    pthread_mutex_t lock;
    buffer_handle_t currentBuffer;
    int pmem_master;
    void* pmem_master_base;

    struct fb_var_screeninfo info;
    struct fb_fix_screeninfo finfo;
    float xdpi;
    float ydpi;
    float fps;
};

/*****************************************************************************/

struct gralloc_buffer_control_t {
  enum gce_buffer_t {
    PRIMARY,
    SECONDARY
  };
  gce_buffer_t last_locked;
};

struct private_handle_t : public native_handle {
    enum {
        PRIV_FLAGS_FRAMEBUFFER = 0x00000001
    };

    // file-descriptors
    int     fd;
    // ints
    int     magic;
    int     flags;
    int     format;
    int     x_res;
    int     y_res;
    // Use to indicate which frame we're using.
    int     frame_offset;
    int     allocating_pid;
    // Flags the position of the primary buffer. 0 for framebuffers,
    // sizeof(gralloc_buffer_control_t) for gralloc buffers.
    int     primary_offset;
    // Used to derive the position of the 4 bytes-per-pixel buffer for
    // conversions.
    int     secondary_offset;
    int     total_size;

    // FIXME: the attributes below should be out-of-line
    void* base __attribute__((aligned(8)));

    static inline int sNumInts() {
        return (((sizeof(private_handle_t) - sizeof(native_handle_t))/sizeof(int)) - sNumFds);
    }
    static const int sNumFds = 1;
    static const int sMagic = 0x3141592;

  private_handle_t(int fd, int size, int format, int x_res, int y_res,
                   int flags) :
        fd(fd), magic(sMagic), flags(flags),
        format(format), x_res(x_res), y_res(y_res), frame_offset(0),
        allocating_pid(getpid()),
        primary_offset(0), secondary_offset(0), total_size(size),
        base(NULL)
    {
        version = sizeof(native_handle);
        numInts = sNumInts();
        numFds = sNumFds;
    }
    ~private_handle_t() {
        magic = 0;
    }

    static int validate(const native_handle* h) {
        const private_handle_t* hnd = (const private_handle_t*)h;
        if (!h || h->version != sizeof(native_handle) ||
                h->numInts != sNumInts() || h->numFds != sNumFds ||
                hnd->magic != sMagic)
        {
            ALOGE("invalid gralloc handle (at %p)", h);
            return -EINVAL;
        }
        return 0;
    }
};

static inline int formatToBytesPerPixel(int format) {
  switch (format) {
    case HAL_PIXEL_FORMAT_RGBA_8888:
    case HAL_PIXEL_FORMAT_RGBX_8888:
    case HAL_PIXEL_FORMAT_BGRA_8888:
      return 4;
    case HAL_PIXEL_FORMAT_RGB_888:
      return 3;
    case HAL_PIXEL_FORMAT_RGB_565:
      return 2;
    default:
      ALOGE("%s: unknown format=%d", __FUNCTION__, format);
      return 4;
  }
}


int fb_device_open(
    const hw_module_t* module, const char* name, hw_device_t** device);

int gralloc_lock(
    gralloc_module_t const* module,
    buffer_handle_t handle, int usage,
    int l, int t, int w, int h,
    void** vaddr);

int gralloc_converting_lock(
    gralloc_module_t const* module,
    buffer_handle_t handle, int usage,
    int l, int t, int w, int h,
    void** vaddr);

int gralloc_unlock(
    gralloc_module_t const* module, buffer_handle_t handle);

int gralloc_converting_unlock(
    gralloc_module_t const* module, buffer_handle_t handle);

int gralloc_register_buffer(
    gralloc_module_t const* module, buffer_handle_t handle);

int gralloc_unregister_buffer(
    gralloc_module_t const* module, buffer_handle_t handle);

#endif /* GRALLOC_PRIV_H_ */
