/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_workstream.h"
#include "src/cpu_common.h"

CpuWorkstream::CpuWorkstream()
        : m_decode(),
          m_encode(),
          m_vpp(),
          m_decvpp(),
          m_allocator(),
          m_handles() {
    av_log_set_level(AV_LOG_QUIET);
}

CpuWorkstream::~CpuWorkstream() {}

mfxStatus CpuWorkstream::Sync(mfxSyncPoint &syncp, mfxU32 wait) {
    return MFX_ERR_NONE;
}
