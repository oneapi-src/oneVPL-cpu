/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"

void CpuWorkstream::GetSurfaceSizes(mfxU32 FourCC,
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

mfxI32 CpuWorkstream::GetFreeSurfaceIndex(mfxFrameSurface1* SurfacesPool,
                                          mfxU32 nPoolSize) {
    for (mfxU32 i = 0; i < nPoolSize; i++) {
        if (0 == SurfacesPool[i].Data.Locked)
            return (mfxI32)i;
    }
    return -1;
}

mfxU8* CpuWorkstream::AllocAlignedBuffer(mfxU32 nBytes, mfxU32 nAlign) {
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

void CpuWorkstream::FreeAlignedBuffer(mfxU8* alignedPtr) {
    if (alignedPtr) {
        mfxU8* basePtr = (mfxU8*)(*(mfxU64*)(alignedPtr - sizeof(mfxU8*)));
        if (basePtr)
            delete[] basePtr;
    }
}

mfxStatus CpuWorkstream::InitDecodeSurfacePool() {
    m_decMemMgmtType = VPL_MEM_MGMT_INTERNAL;

    GetSurfaceSizes(m_decOutFormat, m_decWidth, m_decHeight, m_decPlaneBytes);

    mfxFrameAllocRequest DecRequest = { 0 };
    DecodeQueryIOSurf(nullptr, &DecRequest);

    m_decPoolSize = DecRequest.NumFrameSuggested;
    if (!m_decPoolSize)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    m_decSurfaces = new mfxFrameSurface1[m_decPoolSize];
    if (!m_decSurfaces)
        return MFX_ERR_MEMORY_ALLOC;

    for (mfxU32 i = 0; i < m_decPoolSize; i++) {
        m_decSurfaces[i] = { 0 };

        // allocate and init a mfxFrameSurfaceInterface for this surface
        m_decSurfaces[i].FrameInterface =
            FrameSurfaceInterface::AllocFrameSurfaceInterface();
        if (!m_decSurfaces[i].FrameInterface)
            return MFX_ERR_MEMORY_ALLOC;

        m_decSurfaces[i].Version.Version = MFX_FRAMESURFACE1_VERSION;

        // might be easier just to cache FrameInfo from when DecodeHeader() was called
        m_decSurfaces[i].Info.Width  = m_decWidth;
        m_decSurfaces[i].Info.Height = m_decHeight;
        m_decSurfaces[i].Info.FourCC = m_decOutFormat;

        // TO DO - work out alignment for ffmpeg internal buffers - (align each plane to linesize?)
        mfxU8* Y = AllocAlignedBuffer(m_decPlaneBytes[0], 32);
        mfxU8* U = AllocAlignedBuffer(m_decPlaneBytes[1], 32);
        mfxU8* V = AllocAlignedBuffer(m_decPlaneBytes[2], 32);

        if (!Y || !U || !V)
            return MFX_ERR_MEMORY_ALLOC;

        m_decSurfaces[i].Data.Y = Y;
        m_decSurfaces[i].Data.U = U;
        m_decSurfaces[i].Data.V = V;

        m_decSurfaces[i].Data.Pitch = m_decWidth;
    }

    return MFX_ERR_NONE;
}

void CpuWorkstream::FreeDecodeSurfacePool() {
    for (mfxU32 i = 0; i < m_decPoolSize; i++) {
        FreeAlignedBuffer(m_decSurfaces[i].Data.Y);
        FreeAlignedBuffer(m_decSurfaces[i].Data.U);
        FreeAlignedBuffer(m_decSurfaces[i].Data.V);

        FrameSurfaceInterface::FreeFrameSurfaceInterface(
            m_decSurfaces[i].FrameInterface);
    }

    delete[] m_decSurfaces;
    m_decSurfaces = nullptr;
}

// return free surface and set refCount to 1
mfxStatus CpuWorkstream::GetDecodeSurface(mfxFrameSurface1** surface) {
    if (m_decMemMgmtType != VPL_MEM_MGMT_INTERNAL)
        return MFX_ERR_NOT_INITIALIZED;

    mfxFrameSurface1* foundSurf = nullptr;

    for (mfxU32 i = 0; i < m_decPoolSize; i++) {
        mfxU32 counter = 0xFFFFFFFF;
        m_decSurfaces[i].FrameInterface->GetRefCounter(&m_decSurfaces[i],
                                                       &counter);

        if (counter == 0) {
            foundSurf = &m_decSurfaces[i];
            break;
        }
    }

    // TO DO - clarify expected behavior here - do we just alloc a new
    //   surface and add to the pool? (up to some max?)
    // need to avoid getting in infinite loop due to application
    //   using the API wrong, e.g. never releasing surfaces
    if (!foundSurf)
        return MFX_ERR_MEMORY_ALLOC;

    foundSurf->FrameInterface->AddRef(foundSurf);

    *surface = foundSurf;
    return MFX_ERR_NONE;
}
