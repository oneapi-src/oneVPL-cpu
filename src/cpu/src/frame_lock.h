/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_FRAME_LOCK_H_
#define SRC_CPU_SRC_FRAME_LOCK_H_

#include "src/cpu_common.h"

class FrameLock {
public:
    FrameLock() : m_data(nullptr), m_surface(nullptr), m_allocator(nullptr) {}
    FrameLock(mfxFrameSurface1 *surface,
              mfxU32 flags                 = 0,
              mfxFrameAllocator *allocator = nullptr) {
        Lock(surface, flags, allocator);
    }
    ~FrameLock() {
        Unlock();
    }
    mfxStatus Lock(mfxFrameSurface1 *surface,
                   mfxU32 flags                 = 0,
                   mfxFrameAllocator *allocator = nullptr) {
        VPL_TRACE_FUNC;
        if (allocator && allocator->pthis) {
            mem_id = surface->Data.MemId;
            RET_ERROR(
                allocator->Lock(allocator->pthis, mem_id, &m_locked_data));
            m_data = &m_locked_data;
        }
        else {
            if (surface->Version.Version >= MFX_FRAMESURFACE1_VERSION &&
                surface->FrameInterface) {
                RET_ERROR(surface->FrameInterface->Map(surface, flags));
            }
            m_data = &surface->Data;
        }
        m_surface   = surface;
        m_allocator = allocator;
        return MFX_ERR_NONE;
    }
    mfxStatus Unlock() {
        VPL_TRACE_FUNC;
        if (m_allocator && m_allocator->pthis) {
            m_allocator->Unlock(m_allocator->pthis, mem_id, &m_locked_data);
        }
        else if (m_surface &&
                 m_surface->Version.Version >= MFX_FRAMESURFACE1_VERSION &&
                 m_surface->FrameInterface) {
            RET_ERROR(m_surface->FrameInterface->Unmap(m_surface));
        }
        m_data = nullptr;
        return MFX_ERR_NONE;
    }
    mfxFrameData *GetData() {
        return m_data;
    }

private:
    mfxFrameSurface1 *m_surface;
    mfxFrameAllocator *m_allocator;
    mfxFrameData *m_data;
    mfxFrameData m_locked_data;
    mfxMemId mem_id;
};

#endif // SRC_CPU_SRC_FRAME_LOCK_H_
