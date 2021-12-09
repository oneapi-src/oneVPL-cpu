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

// AV1: for adding IVF header
typedef struct EbConfig {
    uint32_t input_padded_width;
    uint32_t input_padded_height;

    uint64_t frame_count;
    uint64_t ivf_count;
    uint32_t frame_rate;
    /* Default is 0. */
    uint32_t frame_rate_numerator;
    /* Frame rate denominator. When zero, the encoder will use -fps if
     * FrameRateNumerator is also zero, otherwise an error is returned.
     *
     * Default is 0. */
    uint32_t frame_rate_denominator;
} EbConfig;

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

    inline void mem_put_le32(void *vmem, int32_t val) {
        uint8_t *mem = (uint8_t *)vmem;

        mem[0] = (uint8_t)((val >> 0) & 0xff);
        mem[1] = (uint8_t)((val >> 8) & 0xff);
        mem[2] = (uint8_t)((val >> 16) & 0xff);
        mem[3] = (uint8_t)((val >> 24) & 0xff);
    }

    inline void mem_put_le16(void *vmem, int32_t val) {
        uint8_t *mem = (uint8_t *)vmem;

        mem[0] = (uint8_t)((val >> 0) & 0xff);
        mem[1] = (uint8_t)((val >> 8) & 0xff);
    }

    void WriteIVFStreamHeader(EbConfig *config, unsigned char *bs, uint32_t bs_size);
    void WriteIVFFrameHeader(EbConfig *config,
                             unsigned char *bs,
                             uint32_t bs_size,
                             uint32_t frame_size);

    EbConfig m_cfgIVF;
    bool m_bWriteIVFHeaders;

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
    void CopyExtParam(mfxVideoParam &dst, mfxVideoParam &src);
    inline mfxExtBuffer *GetExtBufferById(mfxExtBuffer **extBuffer,
                                          int32_t numExtBuffer,
                                          uint32_t id) {
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
