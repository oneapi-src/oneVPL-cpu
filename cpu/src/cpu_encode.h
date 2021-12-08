/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef CPU_SRC_CPU_ENCODE_H_
#define CPU_SRC_CPU_ENCODE_H_

#include <memory>
#include <string>
#include <utility>
#include "src/cpu_common.h"
#include "src/cpu_frame_pool.h"
#include "src/frame_lock.h"

class CpuWorkstream;

class CpuEncode {
public:
    explicit CpuEncode(CpuWorkstream *session);
    ~CpuEncode();

    static mfxStatus EncodeQuery(mfxVideoParam *in, mfxVideoParam *out);
    static mfxStatus EncodeQueryIOSurf(mfxVideoParam *par, mfxFrameAllocRequest *request);

    mfxStatus InitEncode(mfxVideoParam *par);
    mfxStatus EncodeFrame(mfxFrameSurface1 *surface, mfxEncodeCtrl *ctrl, mfxBitstream *bs);
    mfxStatus GetVideoParam(mfxVideoParam *par);
    mfxStatus GetEncodeSurface(mfxFrameSurface1 **surface);
    mfxStatus IsSameVideoParam(mfxVideoParam *newPar, mfxVideoParam *oldPar);

private:
    static mfxStatus ValidateEncodeParams(mfxVideoParam *par, bool canCorrect);
    int convertTargetUsageVal(int val, int minIn, int maxIn, int minOut, int maxOut);
    mfxStatus InitHEVCParams(mfxVideoParam *par);
    mfxStatus GetHEVCParams(mfxVideoParam *par);
    mfxStatus InitAV1Params(mfxVideoParam *par);
    mfxStatus GetAV1Params(mfxVideoParam *par);
    mfxStatus InitAVCParams(mfxVideoParam *par);
    mfxStatus GetAVCParams(mfxVideoParam *par);
    mfxStatus InitJPEGParams(mfxVideoParam *par);
    mfxStatus GetJPEGParams(mfxVideoParam *par);

    AVFrame *CreateAVFrame(mfxFrameSurface1 *surface);

    const AVCodec *m_avEncCodec;
    AVCodecContext *m_avEncContext;
    AVPacket *m_avEncPacket;
    FrameLock m_input_locker;

    mfxVideoParam m_param;
    bool m_bFrameEncoded;

    CpuWorkstream *m_session;

    std::unique_ptr<CpuFramePool> m_encSurfaces;

    void InitExtBuffers();
    void CleanUpExtBuffers();
    mfxStatus CheckExtBuffers(mfxExtBuffer **extParam, int32_t numExtParam);
    void CopyExtParam(mfxVideoParam& dst, mfxVideoParam& src);
    inline mfxExtBuffer *GetExtBufferById(mfxExtBuffer **extBuffer, int32_t numExtBuffer, uint32_t id) {
        if (extBuffer)
            for (int32_t i = 0; i < numExtBuffer; i++)
                if (extBuffer[i]->BufferId == id)
                    return extBuffer[i];
        return NULL;
    }

    mfxExtAV1BitstreamParam m_extAV1BSParam;
    mfxExtBuffer *m_extParamAll[1];

    size_t m_numExtSupported;

    /* copy not allowed */
    CpuEncode(const CpuEncode &);
    CpuEncode &operator=(const CpuEncode &);
};

#endif // CPU_SRC_CPU_ENCODE_H_
