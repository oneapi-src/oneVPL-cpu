/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_CPU_WORKSTREAM_H_
#define SRC_CPU_SRC_CPU_WORKSTREAM_H_

#include <chrono>
#include <future>
#include <string>
#include <vector>

#include "vpl/mfxstructures.h"

#include "vpl/mfxjpeg.h"

#define ENABLE_LIBAV_AUTO_THREADS

#if !defined(WIN32) && !defined(memcpy_s)
    #define memcpy_s(dest, destsz, src, count) memcpy(dest, src, count)
#endif

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libswscale/swscale.h"
}

#define ERR_EXIT(ws)                    \
    { /* optional logging, etc. here */ \
        return MFX_ERR_UNKNOWN;         \
    }

typedef enum {
    VPL_VPP_CSC       = 1,
    VPL_VPP_SCALE     = 2,
    VPL_VPP_CROP      = 4,
    VPL_VPP_COMPOSITE = 8,
    VPL_VPP_SHARP     = 16,
    VPL_VPP_BLUR      = 32
} eVPPfunction;

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
} Rect;

typedef struct {
    uint32_t vpp_func;

    uint32_t src_width; // buffersrc
    uint32_t src_height; // buffersrc
    AVPixelFormat src_pixel_format; // buffersrc
    uint32_t src_fr_num;
    uint32_t src_fr_den;
    uint32_t src_shift;

    uint32_t dst_width; // scale
    uint32_t dst_height; // scale
    AVPixelFormat dst_pixel_format; // csc
    uint32_t dst_fr_num;
    uint32_t dst_fr_den;
    uint32_t dst_shift;

    Rect src_rc; // crop, composite
    Rect dst_rc; // scale, composite
    double value; // sharp, blur
} VPPBaseConfig;

class CpuWorkstream {
public:
    CpuWorkstream();
    ~CpuWorkstream();

    // decode
    mfxStatus InitDecode(mfxU32 FourCC);
    mfxStatus DecodeHeader(mfxBitstream* bs, mfxVideoParam* par);
    mfxStatus DecodeFrame(mfxBitstream* bs,
                          mfxFrameSurface1* surface_work,
                          mfxFrameSurface1** surface_out);
    void FreeDecode(void);

    // VPP
    mfxStatus Query(mfxVideoParam* in, mfxVideoParam* out);
    mfxStatus QueryIOSurf(mfxVideoParam* par, mfxFrameAllocRequest* request);
    mfxStatus InitVPP(mfxVideoParam* par);
    mfxStatus ProcessFrame(mfxFrameSurface1* surface_in,
                           mfxFrameSurface1* surface_out,
                           mfxExtVppAuxData* aux);
    void FreeVPP(void);

    // encode
    mfxStatus InitEncode(mfxVideoParam* par);
    mfxStatus EncodeFrame(mfxFrameSurface1* surface, mfxBitstream* bs);
    void FreeEncode(void);

    mfxStatus Sync(mfxU32 wait);

    bool getDecInit() {
        return m_decInit;
    }
    bool getVppInit() {
        return m_vppInit;
    }
    bool getEncInit() {
        return m_encInit;
    }

private:
    CpuWorkstream(const CpuWorkstream&) { /* copy not allowed */
    }
    CpuWorkstream& operator=(const CpuWorkstream&) {
        return *this; /* copy not allowed */
    }

    void AVFrame2mfxFrameSurface(mfxFrameSurface1* surface, AVFrame* frame);

    mfxStatus InitHEVCParams(mfxVideoParam* par);
    mfxStatus InitAV1Params(mfxVideoParam* par);
    mfxStatus InitJPEGParams(mfxVideoParam* par);

    // libav objects - Decode
    const AVCodec* m_avDecCodec;
    AVCodecContext* m_avDecContext;
    AVCodecParserContext* m_avDecParser;
    AVPacket* m_avDecPacket;

    // bitstream buffer - Decode
    uint8_t* m_bsDecData;
    uint32_t m_bsDecValidBytes;
    uint32_t m_bsDecMaxBytes;

    // libav objects - Encode
    const AVCodec* m_avEncCodec;
    AVCodecContext* m_avEncContext;
    AVPacket* m_avEncPacket;

    // libav frames
    AVFrame* m_avDecFrameOut;
    AVFrame* m_avVppFrameIn;
    AVFrame* m_avVppFrameOut;
    AVFrame* m_avEncFrameIn;

    bool m_decInit;
    bool m_vppInit;
    bool m_vppBypass;
    bool m_encInit;

    // other internal state
    mfxU32 m_encCodecId;

    // VPP
    bool m_vpp_use_graph;
    VPPBaseConfig m_vpp_base;
    char m_vpp_filter_desc[1024];
    AVFilterGraph* m_vpp_graph;
    AVFilterContext* m_buffersrc_ctx;
    AVFilterContext* m_buffersink_ctx;
    AVFrame* m_av_vpp_in;
    AVFrame* m_av_vpp_out;

    bool InitFilters(void);
    void CloseFilterPads(AVFilterInOut* src_out, AVFilterInOut* sink_in);
    mfxStatus CheckIOPattern_AndSetIOMemTypes(mfxU16 IOPattern,
                                              mfxU16* pInMemType,
                                              mfxU16* pOutMemType);
    bool IsConfigurable(mfxU32 filterId);
    bool IsFilterFound(const mfxU32* pList, mfxU32 len, mfxU32 filterName);
    size_t GetConfigSize(mfxU32 filterId);
    mfxStatus ExtendedQuery(mfxU32 filterName, mfxExtBuffer* pHint);
    mfxU32 GetFilterIndex(mfxU32* pList, mfxU32 len, mfxU32 filterName);
    bool CheckDoUseCompatibility(mfxU32 filterName);
    void GetDoUseFilterList(mfxVideoParam* par, mfxU32** ppList, mfxU32* pLen);
    void GetConfigurableFilterList(mfxVideoParam* par,
                                   mfxU32* pList,
                                   mfxU32* pLen);
    double CalculateUMCFramerate(mfxU32 FrameRateExtN, mfxU32 FrameRateExtD);
    void ReorderPipelineListForQuality(std::vector<mfxU32>& pipelineList);
    void ReorderPipelineListForSpeed(mfxVideoParam* videoParam,
                                     std::vector<mfxU32>& pipelineList);
    void ShowPipeline(std::vector<mfxU32> pipelineList);
    mfxStatus GetPipelineList(mfxVideoParam* videoParam,
                              std::vector<mfxU32>& pipelineList,
                              bool bExtended);
    mfxStatus CheckIOPattern(mfxVideoParam* par);
    mfxStatus CheckFrameInfo(mfxFrameInfo* info, mfxU32 request);
    bool GetExtParamList(mfxVideoParam* par, mfxU32* pList, mfxU32* pLen);
    mfxStatus GetFilterParam(mfxVideoParam* par,
                             mfxU32 filterName,
                             mfxExtBuffer** ppHint);
    void GetDoNotUseFilterList(mfxVideoParam* par,
                               mfxU32** ppList,
                               mfxU32* pLen);
    bool CheckFilterList(mfxU32* pList, mfxU32 count, bool bDoUseTable);
    mfxStatus CheckExtParam(mfxExtBuffer** ppExtParam, mfxU16 count);
    AVPixelFormat MFXFourCC2AVPixelFormat(uint32_t fourcc);
};

#endif // SRC_CPU_SRC_CPU_WORKSTREAM_H_
