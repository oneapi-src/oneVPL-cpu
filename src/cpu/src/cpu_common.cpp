/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"

CpuWorkstream::CpuWorkstream()
        : m_avDecCodec(nullptr),
          m_avDecContext(nullptr),
          m_avDecParser(nullptr),
          m_avDecPacket(nullptr),

          m_bsDecData(nullptr),
          m_bsDecValidBytes(),
          m_bsDecMaxBytes(),

          m_avEncCodec(nullptr),
          m_avEncContext(nullptr),
          m_avEncPacket(nullptr),

          m_avDecFrameOut(nullptr),
          m_avVppFrameIn(nullptr),
          m_avVppFrameOut(nullptr),
          m_avEncFrameIn(nullptr),

          m_decInit(false),
          m_vppInit(false),
          m_vppBypass(false),
          m_encInit(false),

          m_decMemMgmtType(VPL_MEM_MGMT_EXTERNAL),
          m_vppMemMgmtType(VPL_MEM_MGMT_EXTERNAL),
          m_encMemMgmtType(VPL_MEM_MGMT_EXTERNAL),

          m_decCodecId(MFX_CODEC_HEVC),
          m_decOutFormat(MFX_FOURCC_I420),
          m_decWidth(0),
          m_decHeight(0),
          m_decPoolSize(),
          m_decFrameInterface(),
          m_decSurfaces(),

          m_encCodecId(MFX_CODEC_HEVC),
          m_encInFormat(MFX_FOURCC_I420),
          m_encWidth(0),
          m_encHeight(0),
          m_encPoolSize(),
          m_encFrameInterface(),
          m_encSurfaces(),

          m_vpp_use_graph(false),
          m_vpp_graph(nullptr),
          m_buffersrc_ctx(nullptr),
          m_buffersink_ctx(nullptr),
          m_av_vpp_in(nullptr),
          m_av_vpp_out(nullptr) {
    av_log_set_level(AV_LOG_QUIET);
    m_handles.clear();
    memset(&m_vpp_base, 0, sizeof(m_vpp_base));
    memset(m_vpp_filter_desc, 0, sizeof(m_vpp_filter_desc));
}

CpuWorkstream::~CpuWorkstream() {
    FreeDecode();
    FreeVPP();
    FreeEncode();
}

// Sync operation implementation
mfxStatus CpuWorkstream::Sync(mfxSyncPoint &syncp, mfxU32 wait) {
    return MFX_ERR_NONE;
}
