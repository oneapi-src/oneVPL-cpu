/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"

#ifndef DISABLE_LIBAV

CpuWorkstream::CpuWorkstream()
        : m_decInit(false),
          m_decDrain(false),
          m_vppInit(false),
          m_vppBypass(false),
          m_encInit(false),

          m_avDecCodec(nullptr),
          m_avDecContext(nullptr),
          m_avDecParser(nullptr),
          m_avDecPacket(nullptr),

          m_bsDecData(nullptr),
          m_bsDecValidBytes(),
          m_bsDecMaxBytes(),

          m_avVppContext(nullptr),

          m_avEncCodec(nullptr),
          m_avEncContext(nullptr),
          m_avEncPacket(nullptr),

          m_avDecFrameOut(nullptr),
          m_avVppFrameIn(nullptr),
          m_avVppFrameOut(nullptr),
          m_avEncFrameIn(nullptr) {
    av_log_set_level(AV_LOG_QUIET);
}

CpuWorkstream::~CpuWorkstream() {}

mfxStatus CpuWorkstream::Sync(mfxU32 wait) {
    if (decode_future.valid()) {
        std::future_status dec_future_sts =
            decode_future.wait_for(std::chrono::milliseconds(wait));
    }
    return MFX_ERR_NONE;
}

#else

CpuWorkstream::CpuWorkstream(){};
CpuWorkstream::~CpuWorkstream(){};

#endif