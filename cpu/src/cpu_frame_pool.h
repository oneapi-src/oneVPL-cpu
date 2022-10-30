/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef CPU_SRC_CPU_FRAME_POOL_H_
#define CPU_SRC_CPU_FRAME_POOL_H_

#include <memory>
#include <vector>
#include "src/cpu_common.h"
#include "src/cpu_frame.h"

class CpuFramePool {
public:
    CpuFramePool() : m_surfaces(), m_info({}), m_framePoolInterface() {
        // pass handle to this pool for use in external interface functions
        m_framePoolInterface.SetParentPool(this);
    }

    mfxStatus Init(mfxU32 nPoolSize);
    mfxStatus Init(mfxFrameInfo info, mfxU32 nPoolSize);
    mfxStatus GetFreeSurface(mfxFrameSurface1 **surface);

    mfxU32 GetCurrentPoolSize() {
        return (mfxU32)m_surfaces.size();
    }

private:
    std::vector<std::unique_ptr<CpuFrame>> m_surfaces;
    mfxFrameInfo m_info;

    CpuFramePoolInterface m_framePoolInterface;
};

#endif // CPU_SRC_CPU_FRAME_POOL_H_
