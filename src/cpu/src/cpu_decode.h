/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_CPU_DECODE_H_
#define SRC_CPU_SRC_CPU_DECODE_H_

#include <memory>
#include "src/cpu_common.h"
#include "src/cpu_frame_pool.h"

class CpuWorkstream;

class CpuDecode {
public:
    explicit CpuDecode(CpuWorkstream* session);
    ~CpuDecode();

    static mfxStatus DecodeQuery(mfxVideoParam* in, mfxVideoParam* out);
    static mfxStatus DecodeQueryIOSurf(mfxVideoParam* par,
                                       mfxFrameAllocRequest* request);

    mfxStatus InitDecode(mfxVideoParam* par);
    mfxStatus DecodeFrame(mfxBitstream* bs,
                          mfxFrameSurface1* surface_work,
                          mfxFrameSurface1** surface_out);
    mfxStatus GetVideoParam(mfxVideoParam* par);
    mfxStatus GetDecodeSurface(mfxFrameSurface1** surface);

private:
    const AVCodec* m_avDecCodec;
    AVCodecContext* m_avDecContext;
    AVCodecParserContext* m_avDecParser;
    AVPacket* m_avDecPacket;
    AVFrame* m_avDecFrameOut;

    mfxVideoParam m_param;
    std::unique_ptr<CpuFramePool> m_decSurfaces;

    CpuWorkstream* m_session;

    /* copy not allowed */
    CpuDecode(const CpuDecode&);
    CpuDecode& operator=(const CpuDecode&);
};

#endif // SRC_CPU_SRC_CPU_DECODE_H_