/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_frame_pool.h"
#include <memory>
#include <utility>

mfxStatus CpuFramePool::Init(mfxU32 nPoolSize) {
    for (mfxU32 i = 0; i < nPoolSize; i++) {
        auto cpu_frame = std::make_unique<CpuFrame>(&m_framePoolInterface);
        RET_IF_FALSE(cpu_frame->GetAVFrame(), MFX_ERR_MEMORY_ALLOC);
        m_surfaces.push_back(std::move(cpu_frame));
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuFramePool::Init(mfxU32 FourCC, mfxU32 width, mfxU32 height, mfxU32 nPoolSize) {
    for (mfxU32 i = 0; i < nPoolSize; i++) {
        auto cpu_frame = std::make_unique<CpuFrame>(&m_framePoolInterface);
        RET_ERROR(cpu_frame->Allocate(FourCC, width, height));
        m_surfaces.push_back(std::move(cpu_frame));
    }

    m_info.FourCC = FourCC;
    m_info.Width  = width;
    m_info.Height = height;

    return MFX_ERR_NONE;
}

// return free surface and set refCount to 1
mfxStatus CpuFramePool::GetFreeSurface(mfxFrameSurface1 **surface) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);
    *surface = nullptr;

    for (std::unique_ptr<CpuFrame> &surf : m_surfaces) {
        mfxU32 counter = 0xFFFFFFFF;
        surf->FrameInterface->GetRefCounter(surf.get(), &counter);

        if (!counter && !surf->Data.Locked) {
            *surface = surf.get();
            (*surface)->FrameInterface->AddRef(*surface);
            return MFX_ERR_NONE;
        }
    }

    // no free surface found in pool, create new one
    auto cpu_frame = std::make_unique<CpuFrame>(&m_framePoolInterface);
    RET_IF_FALSE(cpu_frame && cpu_frame->GetAVFrame(), MFX_ERR_MEMORY_ALLOC);
    if (m_info.FourCC) {
        RET_ERROR(cpu_frame->Allocate(m_info.FourCC, m_info.Width, m_info.Height));
    }
    *surface = cpu_frame.get();
    (*surface)->FrameInterface->AddRef(*surface);
    m_surfaces.push_back(std::move(cpu_frame));

    return MFX_ERR_NONE;
}

mfxStatus CpuFramePoolInterface::AddRef(struct mfxSurfacePoolInterface *pool) {
    RET_IF_FALSE(pool, MFX_ERR_NULL_PTR);

    CpuFramePoolInterface *framePoolInterface = (CpuFramePoolInterface *)(pool->Context);
    RET_IF_FALSE(framePoolInterface, MFX_ERR_INVALID_HANDLE);

    framePoolInterface->m_refCount++;

    return MFX_ERR_NONE;
}

mfxStatus CpuFramePoolInterface::Release(struct mfxSurfacePoolInterface *pool) {
    RET_IF_FALSE(pool, MFX_ERR_NULL_PTR);

    CpuFramePoolInterface *framePoolInterface = (CpuFramePoolInterface *)(pool->Context);
    RET_IF_FALSE(framePoolInterface, MFX_ERR_INVALID_HANDLE);

    if (framePoolInterface->m_refCount == 0)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    framePoolInterface->m_refCount--;

    // when refCount goes to 0, "destroy" the interface by setting Context to NULL
    //   (m_surfacePoolInterface will not actually be freed until the CpuFramePoolInterface dtor)
    // attempting to call mfxSurfacePoolInterface::FUNCTION after this point will return MFX_ERR_INVALID_HANDLE
    if (framePoolInterface->m_refCount == 0)
        pool->Context = nullptr;

    return MFX_ERR_NONE;
}

mfxStatus CpuFramePoolInterface::GetRefCounter(struct mfxSurfacePoolInterface *pool,
                                               mfxU32 *counter) {
    RET_IF_FALSE(pool, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(counter, MFX_ERR_NULL_PTR);

    CpuFramePoolInterface *framePoolInterface = (CpuFramePoolInterface *)(pool->Context);
    RET_IF_FALSE(framePoolInterface, MFX_ERR_INVALID_HANDLE);

    *counter = framePoolInterface->m_refCount;

    return MFX_ERR_NONE;
}

mfxStatus CpuFramePoolInterface::SetNumSurfaces(struct mfxSurfacePoolInterface *pool,
                                                mfxU32 num_surfaces) {
    RET_IF_FALSE(pool, MFX_ERR_NULL_PTR);

    CpuFramePoolInterface *framePoolInterface = (CpuFramePoolInterface *)(pool->Context);
    RET_IF_FALSE(framePoolInterface, MFX_ERR_INVALID_HANDLE);

    // CpuFramePool only supports MFX_ALLOCATION_UNLIMITED policy
    return MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;
}

mfxStatus CpuFramePoolInterface::RevokeSurfaces(struct mfxSurfacePoolInterface *pool,
                                                mfxU32 num_surfaces) {
    RET_IF_FALSE(pool, MFX_ERR_NULL_PTR);

    CpuFramePoolInterface *framePoolInterface = (CpuFramePoolInterface *)(pool->Context);
    RET_IF_FALSE(framePoolInterface, MFX_ERR_INVALID_HANDLE);

    // CpuFramePool only supports MFX_ALLOCATION_UNLIMITED policy
    return MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;
}

mfxStatus CpuFramePoolInterface::GetAllocationPolicy(struct mfxSurfacePoolInterface *pool,
                                                     mfxPoolAllocationPolicy *policy) {
    RET_IF_FALSE(pool, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(policy, MFX_ERR_NULL_PTR);

    CpuFramePoolInterface *framePoolInterface = (CpuFramePoolInterface *)(pool->Context);
    RET_IF_FALSE(framePoolInterface, MFX_ERR_INVALID_HANDLE);

    // CpuFramePool only supports MFX_ALLOCATION_UNLIMITED policy
    *policy = MFX_ALLOCATION_UNLIMITED;

    return MFX_ERR_NONE;
}

mfxStatus CpuFramePoolInterface::GetMaximumPoolSize(struct mfxSurfacePoolInterface *pool,
                                                    mfxU32 *size) {
    RET_IF_FALSE(pool, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(size, MFX_ERR_NULL_PTR);

    CpuFramePoolInterface *framePoolInterface = (CpuFramePoolInterface *)(pool->Context);
    RET_IF_FALSE(framePoolInterface, MFX_ERR_INVALID_HANDLE);

    // CpuFramePool only supports MFX_ALLOCATION_UNLIMITED policy
    *size = 0xFFFFFFFF;

    return MFX_ERR_NONE;
}

mfxStatus CpuFramePoolInterface::GetCurrentPoolSize(struct mfxSurfacePoolInterface *pool,
                                                    mfxU32 *size) {
    RET_IF_FALSE(pool, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(size, MFX_ERR_NULL_PTR);

    CpuFramePoolInterface *framePoolInterface = (CpuFramePoolInterface *)(pool->Context);
    RET_IF_FALSE(framePoolInterface, MFX_ERR_INVALID_HANDLE);

    CpuFramePool *framePool = (CpuFramePool *)framePoolInterface->GetParentPool();
    RET_IF_FALSE(framePoolInterface, MFX_ERR_INVALID_HANDLE);

    *size = framePool->GetCurrentPoolSize();

    return MFX_ERR_NONE;
}
