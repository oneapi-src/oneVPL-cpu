/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_CPU_FRAME_POOL_H_
#define SRC_CPU_SRC_CPU_FRAME_POOL_H_

#include "src/cpu_common.h"

#define MAX_NUM_PLANES 4

class CpuFramePool {
public:
    CpuFramePool() : surf(nullptr), nPoolSize(0){};
    ~CpuFramePool();
    mfxStatus Init(mfxU32 FourCC,
                   mfxU32 width,
                   mfxU32 height,
                   mfxU32 nPoolSize);
    mfxStatus GetFreeSurface(mfxFrameSurface1** surface);

private:
    mfxFrameSurface1* surf;
    mfxU32 nPoolSize;

    static void GetSurfaceSizes(mfxU32 FourCC,
                                mfxU32 width,
                                mfxU32 height,
                                mfxU32 planeBytes[MAX_NUM_PLANES]);

    static mfxU8* AllocAlignedBuffer(mfxU32 nBytes, mfxU32 nAlign);

    static void FreeAlignedBuffer(mfxU8* alignedPtr);
};

#endif // SRC_CPU_SRC_CPU_FRAME_POOL_H_
