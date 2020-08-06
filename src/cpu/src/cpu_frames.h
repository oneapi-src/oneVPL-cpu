/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_CPU_FRAMES_H_
#define SRC_CPU_SRC_CPU_FRAMES_H_

#include "src/cpu_common.h"

// classes for 2.0 memory API
class FrameSurfaceInterface {
public:
    // callbacks exposed to application
    static mfxStatus AddRef(mfxFrameSurface1* surface);
    static mfxStatus Release(mfxFrameSurface1* surface);
    static mfxStatus GetRefCounter(mfxFrameSurface1* surface, mfxU32* counter);
    static mfxStatus Map(mfxFrameSurface1* surface, mfxU32 flags);
    static mfxStatus Unmap(mfxFrameSurface1* surface);
    static mfxStatus GetNativeHandle(mfxFrameSurface1* surface,
                                     mfxHDL* resource,
                                     mfxResourceType* resource_type);
    static mfxStatus GetDeviceHandle(mfxFrameSurface1* surface,
                                     mfxHDL* device_handle,
                                     mfxHandleType* device_type);
    static mfxStatus Synchronize(mfxFrameSurface1* surface, mfxU32 wait);

    // internal helper functions (init, free)
    static mfxFrameSurfaceInterface* AllocFrameSurfaceInterface();
    static void FreeFrameSurfaceInterface(
        mfxFrameSurfaceInterface* frameInterface);
};

// use as handle for mfxFrameSurfaceInterface
class FrameInterfaceContext : public FrameSurfaceInterface {
public:
    FrameInterfaceContext() {
        m_refCount = 0;
    };
    ~FrameInterfaceContext(){};

    std::atomic<mfxU32> m_refCount;
};

#endif // SRC_CPU_SRC_CPU_FRAMES_H_
