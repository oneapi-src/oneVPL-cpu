/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_vpp.h"
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include "src/cpu_workstream.h"

// vpp in/out type
enum { VPP_IN = 0x00, VPP_OUT = 0x01 };

CpuVPP::CpuVPP(CpuWorkstream* session)
        : m_session(session),
          m_avVppFrameOut(nullptr),
          m_vpp_graph(nullptr),
          m_buffersrc_ctx(nullptr),
          m_buffersink_ctx(nullptr),
          m_vppInFormat(MFX_FOURCC_I420),
          m_vppWidth(0),
          m_vppHeight(0),
          m_vppFunc(0),
          m_param(),
          m_vppSurfaces() {
    memset(m_vpp_filter_desc, 0, sizeof(m_vpp_filter_desc));
}

// buffersrc --> execution filters from filter description --> buffersink
bool CpuVPP::InitFilters(void) {
    int ret                          = 0;
    char buffersrc_fmt[512]          = { 0 };
    const AVFilter* buffersrc        = avfilter_get_by_name("buffer");
    const AVFilter* buffersink       = avfilter_get_by_name("buffersink");
    AVFilterInOut* buffersrc_out_pad = avfilter_inout_alloc();
    AVFilterInOut* buffersink_in_pad = avfilter_inout_alloc();

    m_vpp_graph = avfilter_graph_alloc();

    if (!buffersrc_out_pad || !buffersink_in_pad || !m_vpp_graph) {
        printf("cannot alloc filter graph\n");
        CloseFilterPads(buffersrc_out_pad, buffersink_in_pad);
        return false;
    }

    snprintf(buffersrc_fmt,
             sizeof(buffersrc_fmt),
             "video_size=%ux%u:pix_fmt=%d:time_base=%u/%u", //:pixel_aspect=1/1",
             (unsigned int)m_param.vpp.In.Width,
             (unsigned int)m_param.vpp.In.Height,
             (int)MFXFourCC2AVPixelFormat(m_param.vpp.In.FourCC),
             (unsigned int)m_param.vpp.In.FrameRateExtN,
             (unsigned int)m_param.vpp.In.FrameRateExtD);

    ret = avfilter_graph_create_filter(&m_buffersrc_ctx,
                                       buffersrc,
                                       "video-in",
                                       buffersrc_fmt,
                                       NULL,
                                       m_vpp_graph);
    if (ret < 0) {
        printf("cannot create buffer source\n");
        CloseFilterPads(buffersrc_out_pad, buffersink_in_pad);
        return false;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&m_buffersink_ctx,
                                       buffersink,
                                       "video-out",
                                       NULL,
                                       NULL,
                                       m_vpp_graph);
    if (ret < 0) {
        printf("cannot create buffer sink\n");
        CloseFilterPads(buffersrc_out_pad, buffersink_in_pad);
        return false;
    }

    // scale
    if (m_vppFunc & VPL_VPP_SCALE) {
        snprintf(m_vpp_filter_desc,
                 sizeof(m_vpp_filter_desc),
                 "scale=%d:%d",
                 m_param.vpp.Out.Width,
                 m_param.vpp.Out.Height);
    }

    // crop - do crop and scale to match msdk feature
    if (m_vppFunc & VPL_VPP_CROP) {
        // no need background
        if (m_param.vpp.Out.Width == m_param.vpp.Out.CropW &&
            m_param.vpp.Out.Height == m_param.vpp.Out.CropH) {
            if (m_param.vpp.In.CropW == m_param.vpp.Out.CropW &&
                m_param.vpp.In.CropH == m_param.vpp.Out.CropH) {
                snprintf(m_vpp_filter_desc,
                         sizeof(m_vpp_filter_desc),
                         "crop=%d:%d:%d:%d",
                         m_param.vpp.In.CropW,
                         m_param.vpp.In.CropH,
                         m_param.vpp.In.CropX,
                         m_param.vpp.In.CropY);
            }
            else {
                snprintf(m_vpp_filter_desc,
                         sizeof(m_vpp_filter_desc),
                         "crop=%d:%d:%d:%d,scale=%d:%d",
                         m_param.vpp.In.CropW,
                         m_param.vpp.In.CropH,
                         m_param.vpp.In.CropX,
                         m_param.vpp.In.CropY,
                         m_param.vpp.Out.CropW,
                         m_param.vpp.Out.CropH);
            }
        }
        else {
            std::string f_split     = "split=2[bg][main];";
            std::string f_scale_dst = "[bg]scale=" + std::to_string(m_param.vpp.Out.Width) + ":" +
                                      std::to_string(m_param.vpp.Out.Height) + ",";
            std::string f_bg = "drawbox=x=0:y=0:w=" + std::to_string(m_param.vpp.Out.Width) +
                               ":h=" + std::to_string(m_param.vpp.Out.Height) + ":t=fill[bg2];";

            std::string f_crop_src;
            if (m_param.vpp.In.CropW == m_param.vpp.Out.CropW &&
                m_param.vpp.In.CropH == m_param.vpp.Out.CropH) {
                f_crop_src = "[main]crop=" + std::to_string(m_param.vpp.In.CropW) + ":" +
                             std::to_string(m_param.vpp.In.CropH) + ":" +
                             std::to_string(m_param.vpp.In.CropX) + ":" +
                             std::to_string(m_param.vpp.In.CropY) + "[ovr];";
            }
            else {
                f_crop_src = "[main]crop=" + std::to_string(m_param.vpp.In.CropW) + ":" +
                             std::to_string(m_param.vpp.In.CropH) + ":" +
                             std::to_string(m_param.vpp.In.CropX) + ":" +
                             std::to_string(m_param.vpp.In.CropY) +
                             ",scale=" + std::to_string(m_param.vpp.Out.CropW) + ":" +
                             std::to_string(m_param.vpp.Out.CropH) + "[ovr];";
            }
            std::string f_ovr = "[bg2][ovr]overlay=" + std::to_string(m_param.vpp.Out.CropX) + ":" +
                                std::to_string(m_param.vpp.Out.CropY);

            snprintf(m_vpp_filter_desc,
                     sizeof(m_vpp_filter_desc),
                     "%s%s%s%s%s",
                     f_split.c_str(),
                     f_scale_dst.c_str(),
                     f_bg.c_str(),
                     f_crop_src.c_str(),
                     f_ovr.c_str());
        }

        m_vppFunc |= VPL_VPP_CSC;
    }

    // csc - set pixel format of buffersink
    if (m_vppFunc & VPL_VPP_CSC) {
        AVPixelFormat csc_dst_fmt     = MFXFourCC2AVPixelFormat(m_param.vpp.Out.FourCC);
        enum AVPixelFormat pix_fmts[] = { csc_dst_fmt, AV_PIX_FMT_NONE };

        ret = av_opt_set_int_list(m_buffersink_ctx,
                                  "pix_fmts",
                                  pix_fmts,
                                  AV_PIX_FMT_NONE,
                                  AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            printf("cannot set output pixel format\n");
            CloseFilterPads(buffersrc_out_pad, buffersink_in_pad);
            return false;
        }

        char pixel_format[50] = { 0 };
        if (csc_dst_fmt == AV_PIX_FMT_YUV420P)
            snprintf(pixel_format, sizeof(pixel_format), "format=pix_fmts=yuv420p");
        else if (csc_dst_fmt == AV_PIX_FMT_YUV420P10LE)
            snprintf(pixel_format, sizeof(pixel_format), "format=pix_fmts=yuv420p10le");
        else if (csc_dst_fmt == AV_PIX_FMT_BGRA)
            snprintf(pixel_format, sizeof(pixel_format), "format=pix_fmts=bgra");

        if (m_vppFunc == VPL_VPP_CSC) // there's no filter assigned
            snprintf(m_vpp_filter_desc, sizeof(m_vpp_filter_desc), "%s", pixel_format);
        else {
            std::string curr_desc = m_vpp_filter_desc;
            snprintf(m_vpp_filter_desc,
                     sizeof(m_vpp_filter_desc),
                     "%s,%s",
                     curr_desc.c_str(),
                     pixel_format);
        }
    }

    buffersrc_out_pad->name       = av_strdup("in");
    buffersrc_out_pad->filter_ctx = m_buffersrc_ctx;
    buffersrc_out_pad->pad_idx    = 0;
    buffersrc_out_pad->next       = NULL;

    buffersink_in_pad->name       = av_strdup("out");
    buffersink_in_pad->filter_ctx = m_buffersink_ctx;
    buffersink_in_pad->pad_idx    = 0;
    buffersink_in_pad->next       = NULL;

    // this prevents from failing by non filter description
    if (m_vpp_filter_desc[0] == '\0') {
        snprintf(m_vpp_filter_desc, sizeof(m_vpp_filter_desc), "null");
    }

    ret = avfilter_graph_parse_ptr(m_vpp_graph,
                                   (const char*)m_vpp_filter_desc,
                                   &buffersink_in_pad,
                                   &buffersrc_out_pad,
                                   NULL);
    if (ret < 0) {
        printf("cannot setup graph with filter description\n");
    }
    else {
        ret = avfilter_graph_config(m_vpp_graph, NULL);
    }

    CloseFilterPads(buffersrc_out_pad, buffersink_in_pad);

    if (ret < 0) {
        printf("vpp filter initialization fail\n");
        return false;
    }
    else {
        return true;
    }
}

void CpuVPP::CloseFilterPads(AVFilterInOut* src_out, AVFilterInOut* sink_in) {
    if (src_out)
        avfilter_inout_free(&src_out);
    if (sink_in)
        avfilter_inout_free(&sink_in);
    return;
}

mfxStatus CpuVPP::ValidateVPPParams(mfxVideoParam* par, bool canCorrect) {
    if (canCorrect == true) {
        if (par->AsyncDepth > 16)
            par->AsyncDepth = 16;

        if (!par->AsyncDepth)
            par->AsyncDepth = 1;

        if (par->Protected)
            par->Protected = 0;

        if (par->NumExtParam)
            par->NumExtParam = 0;

        if (!par->vpp.Out.Width)
            par->vpp.Out.Width = par->vpp.In.Width;

        if (!par->vpp.Out.Height)
            par->vpp.Out.Height = par->vpp.In.Height;

        if (!par->vpp.In.FourCC) {
            par->vpp.In.FourCC = MFX_FOURCC_I420;
        }

        if (!par->vpp.In.ChromaFormat) {
            if (par->vpp.In.FourCC == MFX_FOURCC_BGRA)
                par->vpp.In.ChromaFormat = MFX_CHROMAFORMAT_YUV444;
            else
                par->vpp.In.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
        }

        if (!par->vpp.Out.FourCC)
            par->vpp.Out.FourCC = par->vpp.In.FourCC;

        if (!par->vpp.Out.ChromaFormat) {
            par->vpp.Out.ChromaFormat = par->vpp.In.ChromaFormat;
        }

        if (par->Protected)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        if (par->mfx.FrameInfo.PicStruct == MFX_PICSTRUCT_UNKNOWN) {
            par->mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
        }

        par->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    }

    if (0 == par->IOPattern) // IOPattern is mandatory parameter
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (!(par->IOPattern & MFX_IOPATTERN_IN_SYSTEM_MEMORY) ||
        !(par->IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY))
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->AsyncDepth > 16) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->Protected)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->NumExtParam)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.NumThread)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    mfxStatus sts = CheckFrameInfo(&par->vpp.In);
    RET_ERROR(sts);

    sts = CheckFrameInfo(&par->vpp.Out);
    RET_ERROR(sts);

    return sts;
}

mfxStatus CpuVPP::InitVPP(mfxVideoParam* par) {
    mfxStatus sts = ValidateVPPParams(par, false);
    if (sts != MFX_ERR_NONE)
        return sts;

    m_param = *par;

    m_param.vpp.In.CropW =
        (m_param.vpp.In.CropW > m_param.vpp.In.Width) ? m_param.vpp.In.Width : m_param.vpp.In.CropW;
    m_param.vpp.In.CropH = (m_param.vpp.In.CropH > m_param.vpp.In.Height) ? m_param.vpp.In.Height
                                                                          : m_param.vpp.In.CropH;
    m_param.vpp.Out.CropW = (m_param.vpp.Out.CropW > m_param.vpp.Out.Width) ? m_param.vpp.Out.Width
                                                                            : m_param.vpp.Out.CropW;
    m_param.vpp.Out.CropH = (m_param.vpp.Out.CropH > m_param.vpp.Out.Height)
                                ? m_param.vpp.Out.Height
                                : m_param.vpp.Out.CropH;

    if (m_param.vpp.In.FourCC != m_param.vpp.Out.FourCC) {
        m_vppFunc |= VPL_VPP_CSC;
    }

    if (m_param.vpp.In.CropX != 0 || m_param.vpp.In.CropY != 0 || m_param.vpp.Out.CropX != 0 ||
        m_param.vpp.Out.CropY != 0) {
        m_vppFunc |= VPL_VPP_CROP;
    }

    if (!(m_vppFunc & VPL_VPP_CROP) && (m_param.vpp.In.CropW != m_param.vpp.In.Width ||
                                        m_param.vpp.In.CropH != m_param.vpp.In.Height ||
                                        m_param.vpp.Out.CropW != m_param.vpp.Out.Width ||
                                        m_param.vpp.Out.CropH != m_param.vpp.Out.Height)) {
        m_vppFunc |= VPL_VPP_CROP;
    }

    if (!(m_vppFunc & VPL_VPP_CROP) && (m_param.vpp.In.Width != m_param.vpp.Out.Width ||
                                        m_param.vpp.In.Height != m_param.vpp.Out.Height)) {
        m_vppFunc |= VPL_VPP_SCALE;
    }

    if (InitFilters() == false)
        return MFX_ERR_NOT_INITIALIZED;

    m_avVppFrameOut = av_frame_alloc();
    if (!m_avVppFrameOut)
        return MFX_ERR_NOT_INITIALIZED;

    m_vppInFormat = m_param.vpp.In.FourCC;
    m_vppWidth    = m_param.vpp.In.Width;
    m_vppHeight   = m_param.vpp.In.Height;

    return MFX_ERR_NONE;
}

CpuVPP::~CpuVPP() {
    if (m_avVppFrameOut) {
        av_frame_free(&m_avVppFrameOut);
    }

    if (m_vpp_graph) {
        avfilter_graph_free(&m_vpp_graph);
        m_vpp_graph = nullptr;
    }
}

mfxStatus CpuVPP::ProcessFrame(mfxFrameSurface1* surface_in,
                               mfxFrameSurface1* surface_out,
                               mfxExtVppAuxData* aux) {
    // Try get AVFrame from surface_out
    AVFrame* dst_avframe = nullptr;
    CpuFrame* dst_frame  = CpuFrame::TryCast(surface_out);
    if (dst_frame) {
        dst_avframe = dst_frame->GetAVFrame();
    }
    if (!dst_avframe) { // Otherwise use AVFrame allocated in this class
        dst_avframe = m_avVppFrameOut;
    }

    if (surface_in) {
        AVFrame* av_frame =
            m_input_locker.GetAVFrame(surface_in, MFX_MAP_READ, m_session->GetFrameAllocator());
        RET_IF_FALSE(av_frame, MFX_ERR_ABORTED);

        int ret =
            av_buffersrc_add_frame_flags(m_buffersrc_ctx, av_frame, AV_BUFFERSRC_FLAG_KEEP_REF);
        m_input_locker.Unlock();
        RET_IF_FALSE(ret >= 0, MFX_ERR_ABORTED);
    }

    // av_buffersink_get_frame
    int ret = av_buffersink_get_frame(m_buffersink_ctx, dst_avframe);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return MFX_ERR_MORE_DATA;
    }
    RET_IF_FALSE(ret >= 0, MFX_ERR_ABORTED);

    if (dst_avframe == m_avVppFrameOut) { // copy image data
        RET_ERROR(
            AVFrame2mfxFrameSurface(surface_out, m_avVppFrameOut, m_session->GetFrameAllocator()));
        av_frame_unref(m_avVppFrameOut);
    }
    else if (dst_frame) { // update MFXFrameSurface from AVFrame
        dst_frame->Update();
    }

    if (surface_in) {
        if (surface_in->Data.TimeStamp) {
            surface_out->Data.TimeStamp = surface_in->Data.TimeStamp;
            surface_out->Data.DataFlag  = MFX_FRAMEDATA_ORIGINAL_TIMESTAMP;
        }
    }
    return MFX_ERR_NONE;
}

mfxStatus CpuVPP::VPPQuery(mfxVideoParam* in, mfxVideoParam* out) {
    if (out == 0)
        return MFX_ERR_NULL_PTR;

    if (in != 0 && in->Protected != 0)
        return MFX_ERR_UNSUPPORTED;

    if (in == NULL) {
        out->mfx = { 0 };
        out->vpp = { 0 };

        // We have to set FourCC and FrameRate below to
        // pass requirements of CheckPlatformLimitation for frame interpolation

        /* vppIn */
        out->vpp.In.FourCC        = 1;
        out->vpp.In.Height        = 1;
        out->vpp.In.Width         = 1;
        out->vpp.In.CropX         = 1;
        out->vpp.In.CropY         = 1;
        out->vpp.In.CropH         = 1;
        out->vpp.In.CropW         = 1;
        out->vpp.In.PicStruct     = 1;
        out->vpp.In.FrameRateExtN = 1;
        out->vpp.In.FrameRateExtD = 1;

        /* vppOut */
        out->vpp.Out.FourCC        = 1;
        out->vpp.Out.Height        = 1;
        out->vpp.Out.CropX         = 1;
        out->vpp.Out.CropY         = 1;
        out->vpp.Out.Width         = 1;
        out->vpp.Out.CropH         = 1;
        out->vpp.Out.CropW         = 1;
        out->vpp.Out.PicStruct     = 1;
        out->vpp.Out.FrameRateExtN = 1;
        out->vpp.Out.FrameRateExtD = 1;

        out->IOPattern = 1;

        return MFX_ERR_NONE;
    }
    else {
        mfxStatus sts = MFX_ERR_NONE;
        *out          = *in;

        // Query() returns MFX_ERR_UNSUPPORTED for uncorrectable parameter combination
        sts = ValidateVPPParams(out, true);
        if (sts < 0)
            return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuVPP::VPPQueryIOSurf(mfxVideoParam* par, mfxFrameAllocRequest request[2]) {
    mfxStatus sts;

    // VPP_IN
    request[VPP_IN].NumFrameMin       = 1;
    request[VPP_IN].NumFrameSuggested = 1;

    //VPP_OUT
    request[VPP_OUT].NumFrameMin       = 1;
    request[VPP_OUT].NumFrameSuggested = 1;

    // may be null for internal use
    if (par) {
        request[VPP_IN].Info  = par->vpp.In;
        request[VPP_OUT].Info = par->vpp.Out;

        sts = CheckIOPattern_AndSetIOMemTypes(par->IOPattern,
                                              &request[VPP_IN].Type,
                                              &request[VPP_OUT].Type);
    }
    else {
        request[VPP_IN].Info  = { 0 };
        request[VPP_OUT].Info = { 0 };

        sts = MFX_ERR_NONE;
    }

    return sts;
}

mfxStatus CpuVPP::CheckIOPattern_AndSetIOMemTypes(mfxU16 IOPattern,
                                                  mfxU16* pInMemType,
                                                  mfxU16* pOutMemType) {
    if (IOPattern & MFX_IOPATTERN_IN_VIDEO_MEMORY || IOPattern & MFX_IOPATTERN_OUT_VIDEO_MEMORY)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (IOPattern & MFX_IOPATTERN_IN_SYSTEM_MEMORY)
        *pInMemType =
            MFX_MEMTYPE_FROM_VPPIN | MFX_MEMTYPE_EXTERNAL_FRAME | MFX_MEMTYPE_SYSTEM_MEMORY;
    else
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY)
        *pOutMemType =
            MFX_MEMTYPE_FROM_VPPOUT | MFX_MEMTYPE_EXTERNAL_FRAME | MFX_MEMTYPE_SYSTEM_MEMORY;
    else
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

// check each field of FrameInfo excluding PicStruct
mfxStatus CpuVPP::CheckFrameInfo(mfxFrameInfo* info) {
    /* FourCC */
    switch (info->FourCC) {
        case MFX_FOURCC_BGRA:
        case MFX_FOURCC_I420:
        case MFX_FOURCC_I010:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    /* Picture Size */
    if (0 == info->Width || 0 == info->Height) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    /* Picture structure */
    if (info->PicStruct != MFX_PICSTRUCT_PROGRESSIVE && info->PicStruct != MFX_PICSTRUCT_UNKNOWN) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    /* ChromaFormat */
    switch (info->ChromaFormat) {
        case MFX_CHROMAFORMAT_YUV420:
            break;
        case MFX_CHROMAFORMAT_YUV444:
            if (info->FourCC != MFX_FOURCC_BGRA)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuVPP::GetVideoParam(mfxVideoParam* par) {
    *par = m_param;

    return MFX_ERR_NONE;
}

mfxStatus CpuVPP::GetVPPSurface(mfxFrameSurface1** surface) {
    if (!m_vppSurfaces) {
        mfxFrameAllocRequest VPPRequest[2] = { 0 };
        VPPQueryIOSurf(nullptr, VPPRequest);

        auto pool = std::make_unique<CpuFramePool>();
        RET_ERROR(
            pool->Init(m_vppInFormat, m_vppWidth, m_vppHeight, VPPRequest[0].NumFrameSuggested));
        m_vppSurfaces = std::move(pool);
    }

    mfxStatus sts = m_vppSurfaces->GetFreeSurface(surface);
    (*surface)->Data.MemType |=
        MFX_MEMTYPE_FROM_VPPIN | MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_INTERNAL_FRAME;

    return sts;
}

mfxStatus CpuVPP::IsSameVideoParam(mfxVideoParam* newPar, mfxVideoParam* oldPar) {
    if (!(newPar->IOPattern & MFX_IOPATTERN_IN_SYSTEM_MEMORY) ||
        !(newPar->IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY)) {
        return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }

    if (newPar->AsyncDepth > oldPar->AsyncDepth) {
        return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }

    if (newPar->mfx.FrameInfo.Width > oldPar->mfx.FrameInfo.Width) {
        return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }

    if (newPar->mfx.FrameInfo.Height > oldPar->mfx.FrameInfo.Height) {
        return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }

    if (newPar->vpp.In.Width > oldPar->vpp.In.Width) {
        return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }

    if (newPar->vpp.Out.Height > oldPar->vpp.Out.Height) {
        return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }

    if (newPar->mfx.FrameInfo.FourCC != oldPar->mfx.FrameInfo.FourCC) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (newPar->mfx.FrameInfo.ChromaFormat != oldPar->mfx.FrameInfo.ChromaFormat) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    mfxFrameAllocRequest requestOld[2] = { 0 };
    mfxStatus mfxSts                   = VPPQueryIOSurf(oldPar, requestOld);
    if (mfxSts != MFX_ERR_NONE)
        return mfxSts;

    mfxFrameAllocRequest requestNew[2] = { 0 };
    mfxSts                             = VPPQueryIOSurf(newPar, requestNew);
    if (mfxSts != MFX_ERR_NONE)
        return mfxSts;

    if (requestNew[0].NumFrameMin > requestOld[0].NumFrameMin ||
        requestNew[0].Type != requestOld[0].Type) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (requestNew[1].NumFrameMin > requestOld[1].NumFrameMin ||
        requestNew[1].Type != requestOld[1].Type) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return MFX_ERR_NONE;
}
