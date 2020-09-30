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
          m_vppSurfaces() {
    m_vpp_base = { 0 };
    memset(&m_vpp_base, 0, sizeof(m_vpp_base));
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
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d", //:pixel_aspect=1/1",
             m_vpp_base.src_width,
             m_vpp_base.src_height,
             m_vpp_base.src_pixel_format,
             m_vpp_base.src_fr_num,
             m_vpp_base.src_fr_den);

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
    if (m_vpp_base.vpp_func & VPL_VPP_SCALE) {
        snprintf(m_vpp_filter_desc,
                 sizeof(m_vpp_filter_desc),
                 "scale=%d:%d",
                 m_vpp_base.dst_width,
                 m_vpp_base.dst_height);
    }

    // crop - do crop and scale to match msdk feature
    if (m_vpp_base.vpp_func & VPL_VPP_CROP) {
        // no need background
        if (m_vpp_base.dst_width == m_vpp_base.dst_rc.w &&
            m_vpp_base.dst_height == m_vpp_base.dst_rc.h) {
            snprintf(m_vpp_filter_desc,
                     sizeof(m_vpp_filter_desc),
                     "crop=%d:%d:%d:%d,scale=%d:%d",
                     m_vpp_base.src_rc.w,
                     m_vpp_base.src_rc.h,
                     m_vpp_base.src_rc.x,
                     m_vpp_base.src_rc.y,
                     m_vpp_base.dst_rc.w,
                     m_vpp_base.dst_rc.h);
        }
        else {
            std::string f_split     = "split=2[bg][main];";
            std::string f_scale_dst = "[bg]scale=" + std::to_string(m_vpp_base.dst_width) + ":" +
                                      std::to_string(m_vpp_base.dst_height) + ",";
            std::string f_bg = "drawbox=x=0:y=0:w=" + std::to_string(m_vpp_base.dst_width) +
                               ":h=" + std::to_string(m_vpp_base.dst_height) + ":t=fill[bg2];";
            std::string f_crop_src = "[main]crop=" + std::to_string(m_vpp_base.src_rc.w) + ":" +
                                     std::to_string(m_vpp_base.src_rc.h) + ":" +
                                     std::to_string(m_vpp_base.src_rc.x) + ":" +
                                     std::to_string(m_vpp_base.src_rc.y) +
                                     ",scale=" + std::to_string(m_vpp_base.dst_rc.w) + ":" +
                                     std::to_string(m_vpp_base.dst_rc.h) + "[ovr];";
            std::string f_ovr = "[bg2][ovr]overlay=" + std::to_string(m_vpp_base.dst_rc.x) + ":" +
                                std::to_string(m_vpp_base.dst_rc.y);

            snprintf(m_vpp_filter_desc,
                     sizeof(m_vpp_filter_desc),
                     "%s%s%s%s%s",
                     f_split.c_str(),
                     f_scale_dst.c_str(),
                     f_bg.c_str(),
                     f_crop_src.c_str(),
                     f_ovr.c_str());
        }

        m_vpp_base.vpp_func |= VPL_VPP_CSC;
    }

    // csc - set pixel format of buffersink
    if (m_vpp_base.vpp_func & VPL_VPP_CSC) {
        AVPixelFormat csc_dst_fmt     = m_vpp_base.dst_pixel_format;
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

        if (m_vpp_base.vpp_func == VPL_VPP_CSC) // there's no filter assigned
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

mfxStatus CpuVPP::InitVPP(mfxVideoParam* par) {
    int ret       = 0;
    mfxStatus sts = MFX_ERR_INVALID_VIDEO_PARAM;

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

    sts = CheckFrameInfo(&(par->vpp.In), VPP_IN);
    RET_ERROR(sts);

    sts = CheckFrameInfo(&(par->vpp.Out), VPP_OUT);
    RET_ERROR(sts);

    m_vpp_base.src_pixel_format = MFXFourCC2AVPixelFormat(par->vpp.In.FourCC);
    m_vpp_base.src_shift        = par->vpp.In.Shift;
    m_vpp_base.src_fr_num       = par->vpp.In.FrameRateExtN;
    m_vpp_base.src_fr_den       = par->vpp.In.FrameRateExtD;
    m_vpp_base.src_rc.x         = par->vpp.In.CropX;
    m_vpp_base.src_rc.y         = par->vpp.In.CropY;
    m_vpp_base.src_rc.w         = par->vpp.In.CropW;
    m_vpp_base.src_rc.h         = par->vpp.In.CropH;
    m_vpp_base.src_width        = par->vpp.In.Width;
    m_vpp_base.src_height       = par->vpp.In.Height;

    m_vpp_base.dst_rc.x         = par->vpp.Out.CropX;
    m_vpp_base.dst_rc.y         = par->vpp.Out.CropY;
    m_vpp_base.dst_rc.w         = par->vpp.Out.CropW;
    m_vpp_base.dst_rc.h         = par->vpp.Out.CropH;
    m_vpp_base.dst_pixel_format = MFXFourCC2AVPixelFormat(par->vpp.Out.FourCC);
    m_vpp_base.dst_shift        = par->vpp.Out.Shift;
    m_vpp_base.dst_fr_num       = par->vpp.Out.FrameRateExtN;
    m_vpp_base.dst_fr_den       = par->vpp.Out.FrameRateExtD;
    m_vpp_base.dst_width        = par->vpp.Out.Width;
    m_vpp_base.dst_height       = par->vpp.Out.Height;

    if (m_vpp_base.src_pixel_format != m_vpp_base.dst_pixel_format) {
        m_vpp_base.vpp_func |= VPL_VPP_CSC;
    }

    if (m_vpp_base.src_rc.x != 0 || m_vpp_base.src_rc.y != 0 || m_vpp_base.dst_rc.x != 0 ||
        m_vpp_base.dst_rc.y != 0) {
        m_vpp_base.vpp_func |= VPL_VPP_CROP;
    }

    if (!(m_vpp_base.vpp_func & VPL_VPP_CROP) && (m_vpp_base.src_rc.w != m_vpp_base.src_width ||
                                                  m_vpp_base.src_rc.h != m_vpp_base.src_height ||
                                                  m_vpp_base.dst_rc.w != m_vpp_base.dst_width ||
                                                  m_vpp_base.dst_rc.h != m_vpp_base.dst_height)) {
        m_vpp_base.vpp_func |= VPL_VPP_CROP;
    }

    if (!(m_vpp_base.vpp_func & VPL_VPP_CROP) && (m_vpp_base.src_width != m_vpp_base.dst_width ||
                                                  m_vpp_base.src_height != m_vpp_base.dst_height)) {
        m_vpp_base.vpp_func |= VPL_VPP_SCALE;
    }

    if (InitFilters() == false)
        return MFX_ERR_NOT_INITIALIZED;

    m_avVppFrameOut = av_frame_alloc();
    if (!m_avVppFrameOut)
        return MFX_ERR_NOT_INITIALIZED;

    ret = av_image_alloc(m_avVppFrameOut->data,
                         m_avVppFrameOut->linesize,
                         m_vpp_base.dst_width,
                         m_vpp_base.dst_height,
                         m_vpp_base.dst_pixel_format,
                         16);
    if (ret < 0)
        return MFX_ERR_NOT_INITIALIZED;

    m_vppInFormat = par->vpp.In.FourCC;
    m_vppWidth    = par->vpp.In.Width;
    m_vppHeight   = par->vpp.In.Height;

    return MFX_ERR_NONE;
}

CpuVPP::~CpuVPP() {
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
        out->vpp.In.FourCC        = 0xFFFFFFFF;
        out->vpp.In.Height        = 0xFFFF;
        out->vpp.In.Width         = 0xFFFF;
        out->vpp.In.CropH         = 0xFFFF;
        out->vpp.In.CropW         = 0xFFFF;
        out->vpp.In.PicStruct     = 0xFFFF;
        out->vpp.In.FrameRateExtN = 0xFFFFFFFF;
        out->vpp.In.FrameRateExtD = 0xFFFFFFFF;

        /* vppOut */
        out->vpp.Out.FourCC        = 0xFFFFFFFF;
        out->vpp.Out.Height        = 0xFFFF;
        out->vpp.Out.Width         = 0xFFFF;
        out->vpp.Out.CropH         = 0xFFFF;
        out->vpp.Out.CropW         = 0xFFFF;
        out->vpp.Out.PicStruct     = 0xFFFF;
        out->vpp.Out.FrameRateExtN = 0xFFFFFFFF;
        out->vpp.Out.FrameRateExtD = 0xFFFFFFFF;

        out->IOPattern = 0xFFFF;

        return MFX_ERR_NONE;
    }
    else {
        *out = *in;

        if (!out->vpp.Out.Width)
            out->vpp.Out.Width = out->vpp.In.Width;

        if (!out->vpp.Out.Height)
            out->vpp.Out.Height = out->vpp.In.Height;

        if (!out->vpp.Out.FourCC)
            out->vpp.Out.FourCC = out->vpp.In.FourCC;

        if (out->Protected) {
            return MFX_ERR_INVALID_VIDEO_PARAM;
        }

        //query, always correct
        out->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
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
mfxStatus CpuVPP::CheckFrameInfo(mfxFrameInfo* info, mfxU32 request) {
    mfxStatus mfxSts = MFX_ERR_NONE;

    /* FourCC */
    switch (info->FourCC) {
        case MFX_FOURCC_NV12:
        case MFX_FOURCC_RGB4:
        case MFX_FOURCC_P210:
        case MFX_FOURCC_NV16:
        case MFX_FOURCC_YUY2:
        case MFX_FOURCC_I420:
        case MFX_FOURCC_I010:
            break;
        case MFX_FOURCC_P010:
            if (info->Shift == 0)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    /* Picture Size */
    if (0 == info->Width || 0 == info->Height) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    /* Frame Rate */
    if (0 == info->FrameRateExtN || 0 == info->FrameRateExtD) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return mfxSts;
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

    return m_vppSurfaces->GetFreeSurface(surface);
}
