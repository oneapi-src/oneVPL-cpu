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
    FrameLock()
            : m_data(nullptr),
              m_surface(nullptr),
              m_allocator(nullptr),
              m_newapi(false) {}
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
        RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);
        m_surface   = surface;
        m_allocator = allocator;
        m_newapi    = surface->Version.Version >= MFX_FRAMESURFACE1_VERSION &&
                   surface->FrameInterface;
        if (allocator && allocator->pthis) {
            mem_id = surface->Data.MemId;
            RET_ERROR(
                allocator->Lock(allocator->pthis, mem_id, &m_locked_data));
            m_data = &m_locked_data;
        }
        else {
            if (m_newapi) {
                RET_ERROR(surface->FrameInterface->Map(surface, flags));
                surface->FrameInterface->AddRef(surface);
            }
            else {
                surface->Data.Locked++; // TODO(make atomic)
            }
            m_data = &surface->Data;
        }
        return MFX_ERR_NONE;
    }
    void Unlock() {
        VPL_TRACE_FUNC;
        if (m_data) {
            if (m_allocator && m_allocator->pthis) {
                m_allocator->Unlock(m_allocator->pthis, mem_id, m_data);
            }
            else if (m_surface) {
                if (m_newapi) {
                    m_surface->FrameInterface->Unmap(m_surface);
                    m_surface->FrameInterface->Release(m_surface);
                }
                else {
                    m_surface->Data.Locked--; // TODO(make atomic)
                }
            }
            m_data = nullptr;
        }
    }
    mfxFrameData *GetData() {
        return m_data;
    }

private:
    mfxFrameSurface1 *m_surface;
    mfxFrameAllocator *m_allocator;
    bool m_newapi;
    mfxFrameData *m_data;
    mfxFrameData m_locked_data;
    mfxMemId mem_id;
};

#endif // SRC_CPU_SRC_FRAME_LOCK_H_
