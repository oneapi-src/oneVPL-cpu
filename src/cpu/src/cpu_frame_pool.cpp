/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_frame_pool.h"
#include "src/cpu_frames.h"

// calculate space required for surface given CS/width/height
void CpuFramePool::GetSurfaceSizes(mfxU32 FourCC,
                                   mfxU32 width,
                                   mfxU32 height,
                                   mfxU32 planeBytes[MAX_NUM_PLANES]) {
    mfxU32 nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_I420:
            planeBytes[0] = width * height;
            planeBytes[1] = (width >> 1) * (height >> 1);
            planeBytes[2] = (width >> 1) * (height >> 1);
            planeBytes[3] = 0;
            break;
        case MFX_FOURCC_I010:
            planeBytes[0] = 2 * width * height;
            planeBytes[1] = 2 * (width >> 1) * (height >> 1);
            planeBytes[2] = 2 * (width >> 1) * (height >> 1);
            planeBytes[3] = 0;
            break;
        default:
            planeBytes[0] = 0;
            planeBytes[1] = 0;
            planeBytes[2] = 0;
            planeBytes[3] = 0;
            break;
    }
}

// allocated buffer of size nBytes with alignment nAlign
// nAlign must be power of 2
mfxU8* CpuFramePool::AllocAlignedBuffer(mfxU32 nBytes, mfxU32 nAlign) {
    RET_IF_FALSE(nBytes, nullptr);

    if (nAlign < sizeof(mfxU8*))
        nAlign = sizeof(mfxU8*);

    // alloc enough space for buffer, alignment padding, and base ptr
    mfxU8* basePtr = new mfxU8[nBytes + 2 * nAlign];
    if (!basePtr)
        return nullptr;

    // pointer to aligned buffer which we return to caller
    mfxU64 nAlignM1 = (mfxU64)nAlign - 1;
    mfxU8* alignedPtr =
        (mfxU8*)(((mfxU64)basePtr + nAlign + nAlignM1) & (~nAlignM1));

    // save pointer to actual base - needed for freeing buffer
    *(mfxU64*)(alignedPtr - sizeof(mfxU8*)) = (mfxU64)basePtr;

    return alignedPtr;
}

// free buffer allocated with AllocAlignBuffer
void CpuFramePool::FreeAlignedBuffer(mfxU8* alignedPtr) {
    if (alignedPtr) {
        mfxU8* basePtr = (mfxU8*)(*(mfxU64*)(alignedPtr - sizeof(mfxU8*)));
        if (basePtr)
            delete[] basePtr;
    }
}

// init pool of numSurfaces mfxFrameSurface1 types
// includes corresponding FrameSurfaceInterface alloc/init (2.0)
mfxStatus CpuFramePool::Init(mfxU32 FourCC,
                             mfxU32 width,
                             mfxU32 height,
                             mfxU32 nPoolSize) {
    surf = new mfxFrameSurface1[nPoolSize];
    RET_IF_FALSE(surf, MFX_ERR_MEMORY_ALLOC);
    this->nPoolSize = nPoolSize;

    mfxU32 decPlaneBytes[MAX_NUM_PLANES];
    GetSurfaceSizes(FourCC, width, height, decPlaneBytes);

    for (mfxU32 i = 0; i < nPoolSize; i++) {
        surf[i] = { 0 };

        // allocate and init a mfxFrameSurfaceInterface for this surface
        surf[i].FrameInterface =
            FrameSurfaceInterface::AllocFrameSurfaceInterface();
        RET_IF_FALSE(surf[i].FrameInterface, MFX_ERR_MEMORY_ALLOC)

        surf[i].Version.Version = MFX_FRAMESURFACE1_VERSION;

        surf[i].Info.Width  = width;
        surf[i].Info.Height = height;
        surf[i].Info.FourCC = FourCC;

        // TO DO - work out alignment for ffmpeg internal buffers - (align each plane to linesize?)
        mfxU8* Y = AllocAlignedBuffer(decPlaneBytes[0], 32);
        mfxU8* U = AllocAlignedBuffer(decPlaneBytes[1], 32);
        mfxU8* V = AllocAlignedBuffer(decPlaneBytes[2], 32);

        RET_IF_FALSE(Y && U && V, MFX_ERR_MEMORY_ALLOC)

        surf[i].Data.Y = Y;
        surf[i].Data.U = U;
        surf[i].Data.V = V;

        surf[i].Data.Pitch = width;
    }

    return MFX_ERR_NONE;
}

// free surfaces allocated with InitSurfacePool
CpuFramePool::~CpuFramePool() {
    for (mfxU32 i = 0; i < nPoolSize; i++) {
        FreeAlignedBuffer(surf[i].Data.Y);
        FreeAlignedBuffer(surf[i].Data.U);
        FreeAlignedBuffer(surf[i].Data.V);

        FrameSurfaceInterface::FreeFrameSurfaceInterface(
            surf[i].FrameInterface);
    }

    delete[] surf;
    surf = nullptr;
}

// return free surface and set refCount to 1
mfxStatus CpuFramePool::GetFreeSurface(mfxFrameSurface1** surface) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);
    *surface = nullptr;

    for (mfxU32 i = 0; i < nPoolSize; i++) {
        mfxU32 counter = 0xFFFFFFFF;
        surf[i].FrameInterface->GetRefCounter(&surf[i], &counter);

        if (!counter && !surf[i].Data.Locked) {
            *surface = &surf[i];
            break;
        }
    }

    // no free surface found in pool
    // TO DO - clarify expected behavior here - do we just alloc a new
    //   surface and add to the pool? (up to some max?)
    // need to avoid getting in infinite loop due to application
    //   using the API wrong, e.g. never releasing surfaces
    RET_IF_FALSE(*surface, MFX_ERR_NOT_FOUND);

    (*surface)->FrameInterface->AddRef(*surface);

    return MFX_ERR_NONE;
}
