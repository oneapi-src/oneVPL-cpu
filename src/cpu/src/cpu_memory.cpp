/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"

// calculate space required for surface given CS/width/height
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

// TO DO - clarify expected behavior here - what is the relationship
//   between Data.Locked and the 2.0 refCount API? do we even need this?
mfxI32 CpuWorkstream::GetFreeSurfaceIndex(mfxFrameSurface1* SurfacesPool,
                                          mfxU32 nPoolSize) {
    for (mfxU32 i = 0; i < nPoolSize; i++) {
        if (0 == SurfacesPool[i].Data.Locked)
            return (mfxI32)i;
    }
    return -1;
}

// allocated buffer of size nBytes with alignment nAlign
// nAlign must be power of 2
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

// free buffer allocated with AllocAlignBuffer
void CpuWorkstream::FreeAlignedBuffer(mfxU8* alignedPtr) {
    if (alignedPtr) {
        mfxU8* basePtr = (mfxU8*)(*(mfxU64*)(alignedPtr - sizeof(mfxU8*)));
        if (basePtr)
            delete[] basePtr;
    }
}

// init pool of numSurfaces mfxFrameSurface1 types
// includes corresponding FrameSurfaceInterface alloc/init (2.0)
mfxFrameSurface1* CpuWorkstream::InitSurfacePool(mfxU32 FourCC,
                                                 mfxU32 width,
                                                 mfxU32 height,
                                                 mfxU32 nPoolSize) {
    mfxFrameSurface1* surf = new mfxFrameSurface1[nPoolSize];
    if (!surf)
        return nullptr;

    mfxU32 decPlaneBytes[MAX_NUM_PLANES];
    GetSurfaceSizes(FourCC, width, height, decPlaneBytes);

    for (mfxU32 i = 0; i < nPoolSize; i++) {
        surf[i] = { 0 };

        // allocate and init a mfxFrameSurfaceInterface for this surface
        surf[i].FrameInterface =
            FrameSurfaceInterface::AllocFrameSurfaceInterface();
        if (!surf[i].FrameInterface)
            return nullptr;

        surf[i].Version.Version = MFX_FRAMESURFACE1_VERSION;

        surf[i].Info.Width  = width;
        surf[i].Info.Height = height;
        surf[i].Info.FourCC = FourCC;

        // TO DO - work out alignment for ffmpeg internal buffers - (align each plane to linesize?)
        mfxU8* Y = AllocAlignedBuffer(decPlaneBytes[0], 32);
        mfxU8* U = AllocAlignedBuffer(decPlaneBytes[1], 32);
        mfxU8* V = AllocAlignedBuffer(decPlaneBytes[2], 32);

        if (!Y || !U || !V)
            return nullptr;

        surf[i].Data.Y = Y;
        surf[i].Data.U = U;
        surf[i].Data.V = V;

        surf[i].Data.Pitch = width;
    }

    return surf;
}

// free surfaces allocated with InitSurfacePool
void CpuWorkstream::FreeSurfacePool(mfxFrameSurface1* surf, mfxU32 nPoolSize) {
    for (mfxU32 i = 0; i < nPoolSize; i++) {
        FreeAlignedBuffer(surf[i].Data.Y);
        FreeAlignedBuffer(surf[i].Data.U);
        FreeAlignedBuffer(surf[i].Data.V);

        FrameSurfaceInterface::FreeFrameSurfaceInterface(
            surf[i].FrameInterface);
    }

    delete[] surf;
}

// return free surface and set refCount to 1
mfxFrameSurface1* CpuWorkstream::GetFreeSurface(mfxFrameSurface1* surf,
                                                mfxU32 nPoolSize) {
    mfxFrameSurface1* foundSurf = nullptr;

    for (mfxU32 i = 0; i < nPoolSize; i++) {
        mfxU32 counter = 0xFFFFFFFF;
        surf[i].FrameInterface->GetRefCounter(&surf[i], &counter);

        if (counter == 0) {
            foundSurf = &surf[i];
            break;
        }
    }

    // no free surface found in pool
    // let caller decide how to handle this case
    //   (error out, alloc new surfaces, etc.)
    if (!foundSurf)
        return nullptr;

    foundSurf->FrameInterface->AddRef(foundSurf);

    return foundSurf;
}

// Decode surface pool management
mfxStatus CpuWorkstream::InitDecodeSurfacePool() {
    m_decMemMgmtType = VPL_MEM_MGMT_INTERNAL;

    mfxFrameAllocRequest DecRequest = { 0 };
    DecodeQueryIOSurf(nullptr, &DecRequest);

    m_decPoolSize = DecRequest.NumFrameSuggested;
    if (!m_decPoolSize)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    m_decSurfaces =
        InitSurfacePool(m_decOutFormat, m_decWidth, m_decHeight, m_decPoolSize);

    if (!m_decSurfaces)
        return MFX_ERR_MEMORY_ALLOC;

    return MFX_ERR_NONE;
}

void CpuWorkstream::FreeDecodeSurfacePool() {
    FreeSurfacePool(m_decSurfaces, m_decPoolSize);

    m_decSurfaces = nullptr;
}

// return free surface and set refCount to 1
mfxStatus CpuWorkstream::GetDecodeSurface(mfxFrameSurface1** surface) {
    if (m_decMemMgmtType != VPL_MEM_MGMT_INTERNAL)
        return MFX_ERR_NOT_INITIALIZED;

    mfxFrameSurface1* foundSurf = GetFreeSurface(m_decSurfaces, m_decPoolSize);

    // TO DO - clarify expected behavior here - do we just alloc a new
    //   surface and add to the pool? (up to some max?)
    // need to avoid getting in infinite loop due to application
    //   using the API wrong, e.g. never releasing surfaces
    if (!foundSurf)
        return MFX_ERR_MEMORY_ALLOC;

    *surface = foundSurf;

    return MFX_ERR_NONE;
}

// Encode surface pool management
mfxStatus CpuWorkstream::InitEncodeSurfacePool() {
    m_encMemMgmtType = VPL_MEM_MGMT_INTERNAL;

    mfxFrameAllocRequest EncRequest = { 0 };
    EncodeQueryIOSurf(nullptr, &EncRequest);

    m_encPoolSize = EncRequest.NumFrameSuggested;
    if (!m_encPoolSize)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    m_encSurfaces =
        InitSurfacePool(m_encInFormat, m_encWidth, m_encHeight, m_encPoolSize);

    if (!m_encSurfaces)
        return MFX_ERR_MEMORY_ALLOC;

    return MFX_ERR_NONE;
}

void CpuWorkstream::FreeEncodeSurfacePool() {
    FreeSurfacePool(m_encSurfaces, m_encPoolSize);

    m_encSurfaces = nullptr;
}

mfxStatus CpuWorkstream::GetEncodeSurface(mfxFrameSurface1** surface) {
    if (m_encMemMgmtType != VPL_MEM_MGMT_INTERNAL)
        return MFX_ERR_NOT_INITIALIZED;

    mfxFrameSurface1* foundSurf = GetFreeSurface(m_encSurfaces, m_encPoolSize);

    if (!foundSurf)
        return MFX_ERR_MEMORY_ALLOC;

    *surface = foundSurf;

    return MFX_ERR_NONE;
}

// VPP surface pool management
mfxStatus CpuWorkstream::InitVPPSurfacePool() {
    m_vppMemMgmtType = VPL_MEM_MGMT_INTERNAL;

    mfxFrameAllocRequest VPPRequest[2] = { 0 };
    VPPQueryIOSurf(nullptr, VPPRequest);

    m_vppPoolSize = VPPRequest[0].NumFrameSuggested;
    if (!m_vppPoolSize)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    m_vppSurfaces =
        InitSurfacePool(m_vppInFormat, m_vppWidth, m_vppHeight, m_vppPoolSize);

    if (!m_vppSurfaces)
        return MFX_ERR_MEMORY_ALLOC;

    return MFX_ERR_NONE;
}

void CpuWorkstream::FreeVPPSurfacePool() {
    FreeSurfacePool(m_vppSurfaces, m_vppPoolSize);

    m_vppSurfaces = nullptr;
}

mfxStatus CpuWorkstream::GetVPPSurface(mfxFrameSurface1** surface) {
    if (m_vppMemMgmtType != VPL_MEM_MGMT_INTERNAL)
        return MFX_ERR_NOT_INITIALIZED;

    mfxFrameSurface1* foundSurf = GetFreeSurface(m_vppSurfaces, m_vppPoolSize);

    if (!foundSurf)
        return MFX_ERR_MEMORY_ALLOC;

    *surface = foundSurf;

    return MFX_ERR_NONE;
}
