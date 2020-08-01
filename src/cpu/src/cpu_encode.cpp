/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <sstream>
#include "./cpu_workstream.h"

mfxStatus CpuWorkstream::ValidateEncodeParams(mfxVideoParam *par) {
    if (par->mfx.FrameInfo.FourCC) {
        if (par->mfx.FrameInfo.FourCC != MFX_FOURCC_I420 &&
            par->mfx.FrameInfo.FourCC != MFX_FOURCC_I010)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        par->mfx.FrameInfo.FourCC = MFX_FOURCC_I420;
    }

    // validate fields in the input param struct
    if (par->mfx.CodecId != MFX_CODEC_HEVC &&
        par->mfx.CodecId != MFX_CODEC_AVC &&
        par->mfx.CodecId != MFX_CODEC_JPEG && par->mfx.CodecId != MFX_CODEC_AV1)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    // check codec id and the values
    switch (par->mfx.CodecId) {
        case MFX_CODEC_AVC: // leave this for later
            // default: vbr, profile high, level 3.1
            if (par->mfx.FrameInfo.Width < 64 ||
                par->mfx.FrameInfo.Width > 4096)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.FrameInfo.Height < 64 ||
                par->mfx.FrameInfo.Height > 2304)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.RateControlMethod) {
                if (par->mfx.RateControlMethod != MFX_RATECONTROL_CQP &&
                    par->mfx.RateControlMethod != MFX_RATECONTROL_CBR &&
                    par->mfx.RateControlMethod != MFX_RATECONTROL_VBR)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
            }

            if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP)
                if (par->mfx.QPI > 51 || par->mfx.QPP > 51 || par->mfx.QPB > 51)
                    return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.CodecProfile) {
                if (par->mfx.CodecProfile != MFX_PROFILE_AVC_BASELINE &&
                    par->mfx.CodecProfile != MFX_PROFILE_AVC_MAIN &&
                    par->mfx.CodecProfile != MFX_PROFILE_AVC_HIGH)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.CodecProfile = MFX_PROFILE_AVC_HIGH;
            }

            if (par->mfx.CodecLevel) {
                if (par->mfx.CodecLevel != MFX_LEVEL_AVC_1 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_1b &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_11 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_12 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_13 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_2 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_21 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_22 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_3 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_31 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_32 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_4 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_41 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_42 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_5 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_51 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_52)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.CodecLevel = MFX_LEVEL_AVC_31;
            }

            break;
        case MFX_CODEC_HEVC:
            // default: vbr, profile main, level 3.1
            if (par->mfx.FrameInfo.Width < 64 ||
                par->mfx.FrameInfo.Width > 8192)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.FrameInfo.Height < 64 ||
                par->mfx.FrameInfo.Height > 4320)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.RateControlMethod) {
                if (par->mfx.RateControlMethod != MFX_RATECONTROL_CQP &&
                    par->mfx.RateControlMethod != MFX_RATECONTROL_VBR)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
            }

            if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP)
                if (par->mfx.QPI > 51 || par->mfx.QPP > 51 || par->mfx.QPB > 51)
                    return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.CodecProfile) {
                if (par->mfx.CodecProfile != MFX_PROFILE_HEVC_MAIN &&
                    par->mfx.CodecProfile != MFX_PROFILE_HEVC_MAIN10)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN;
            }

            if (par->mfx.CodecLevel) {
                if (par->mfx.CodecLevel != MFX_LEVEL_HEVC_1 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_2 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_21 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_3 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_31 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_4 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_41 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_5 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_51 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_52 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_6 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_61 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_62)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.CodecLevel = MFX_LEVEL_HEVC_31;
            }

            break;
        case MFX_CODEC_JPEG:
            // default: baseline
            if (par->mfx.FrameInfo.Width < 64 ||
                par->mfx.FrameInfo.Width > 8192)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.FrameInfo.Height < 64 ||
                par->mfx.FrameInfo.Height > 8192)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.Quality) {
                if (par->mfx.Quality < 1 || par->mfx.Quality > 100)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.Quality = 80;
            }

            if (par->mfx.CodecProfile) {
                if (par->mfx.CodecProfile != MFX_PROFILE_JPEG_BASELINE)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.CodecProfile = MFX_PROFILE_JPEG_BASELINE;
            }

            break;
        case MFX_CODEC_AV1:
            // default: VBR
            if (par->mfx.FrameInfo.Width < 64 ||
                par->mfx.FrameInfo.Width > 4096)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.FrameInfo.Height < 64 ||
                par->mfx.FrameInfo.Height > 2304)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.RateControlMethod) {
                if (par->mfx.RateControlMethod != MFX_RATECONTROL_CQP &&
                    par->mfx.RateControlMethod != MFX_RATECONTROL_CBR &&
                    par->mfx.RateControlMethod != MFX_RATECONTROL_VBR)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
            }

            if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP)
                if (par->mfx.QPI > 63 || par->mfx.QPP > 63 || par->mfx.QPB > 63)
                    return MFX_ERR_INVALID_VIDEO_PARAM;

            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.TargetKbps == 0)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.FrameInfo.FrameRateExtN == 0 ||
        par->mfx.FrameInfo.FrameRateExtN > 65535)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.FrameInfo.FrameRateExtD == 0 ||
        par->mfx.FrameInfo.FrameRateExtD > 65535)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.FrameInfo.AspectRatioW == 0)
        par->mfx.FrameInfo.AspectRatioW = 1;

    if (par->mfx.FrameInfo.AspectRatioH == 0)
        par->mfx.FrameInfo.AspectRatioH = 1;

    if (par->mfx.GopOptFlag != 0 && par->mfx.GopOptFlag != MFX_GOP_CLOSED)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.FrameInfo.BitDepthChroma) {
        if (par->mfx.FrameInfo.BitDepthChroma != 8 &&
            par->mfx.FrameInfo.BitDepthChroma != 10)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        par->mfx.FrameInfo.BitDepthChroma = 8;
    }

    if (par->mfx.FrameInfo.BitDepthLuma) {
        if (par->mfx.FrameInfo.BitDepthLuma != 8 &&
            par->mfx.FrameInfo.BitDepthLuma != 10)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        par->mfx.FrameInfo.BitDepthLuma = 8;
    }

    if (par->mfx.TargetUsage) {
        if (par->mfx.TargetUsage < MFX_TARGETUSAGE_1 ||
            par->mfx.TargetUsage > MFX_TARGETUSAGE_7)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        par->mfx.TargetKbps = MFX_TARGETUSAGE_BALANCED;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuWorkstream::InitEncode(mfxVideoParam *par) {
    mfxStatus sts = ValidateEncodeParams(par);
    if (sts != MFX_ERR_NONE)
        return sts;

    m_encCodecId = par->mfx.CodecId;

    AVCodecID cid = AV_CODEC_ID_NONE;

    switch (m_encCodecId) {
        case MFX_CODEC_AVC:
            cid = AV_CODEC_ID_H264;
            break;
        case MFX_CODEC_HEVC:
            cid = AV_CODEC_ID_HEVC;
            break;
        case MFX_CODEC_JPEG:
            cid = AV_CODEC_ID_MJPEG;
            break;
        case MFX_CODEC_AV1:
            cid = AV_CODEC_ID_AV1;
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    m_avEncCodec = avcodec_find_encoder(cid);
    if (!m_avEncCodec)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    m_avEncContext = avcodec_alloc_context3(m_avEncCodec);
    if (!m_avEncContext)
        return MFX_ERR_MEMORY_ALLOC;

    m_avEncPacket = av_packet_alloc();
    if (!m_avEncPacket)
        return MFX_ERR_MEMORY_ALLOC;

    //------------------------------
    // Set general libav parameters
    // values not set in mfxVideoParam should keep defaults
    //------------------------------
    m_avEncContext->width  = par->mfx.FrameInfo.Width;
    m_avEncContext->height = par->mfx.FrameInfo.Height;

    m_avEncContext->gop_size     = par->mfx.GopPicSize;
    m_avEncContext->max_b_frames = par->mfx.GopRefDist;
    m_avEncContext->bit_rate = par->mfx.TargetKbps * 1000; // prop is in kbps;

    m_avEncContext->framerate.num = par->mfx.FrameInfo.FrameRateExtN;
    m_avEncContext->framerate.den = par->mfx.FrameInfo.FrameRateExtD;
    m_avEncContext->time_base.num = par->mfx.FrameInfo.FrameRateExtD;
    m_avEncContext->time_base.den = par->mfx.FrameInfo.FrameRateExtN;

    m_avEncContext->sample_aspect_ratio.num = par->mfx.FrameInfo.AspectRatioH;
    m_avEncContext->sample_aspect_ratio.den = par->mfx.FrameInfo.AspectRatioW;

    m_avEncContext->slices = par->mfx.NumSlice;
    m_avEncContext->refs   = par->mfx.NumRefFrame;

    if (par->mfx.GopOptFlag == MFX_GOP_CLOSED)
        m_avEncContext->flags &= AV_CODEC_FLAG_CLOSED_GOP;

    if (par->mfx.FrameInfo.BitDepthChroma == 10) {
        // Main10: 10-bit 420
        m_avEncContext->pix_fmt = AV_PIX_FMT_YUV420P10;
    }
    else {
        // default: 8-bit 420
        if (m_encCodecId == MFX_CODEC_JPEG)
            m_avEncContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
        else
            m_avEncContext->pix_fmt = AV_PIX_FMT_YUV420P;
    }

    if (m_avEncContext->pix_fmt == AV_PIX_FMT_YUV420P10LE)
        m_encInFormat = MFX_FOURCC_I010;
    else if (m_avEncContext->pix_fmt == AV_PIX_FMT_YUV420P)
        m_encInFormat = MFX_FOURCC_I420;
    else
        m_encInFormat = MFX_FOURCC_I420;

    m_encWidth  = m_avEncContext->width;
    m_encHeight = m_avEncContext->height;

    // set defaults for anything not passed in
    if (!m_avEncContext->gop_size)
        m_avEncContext->gop_size =
            2 *
            static_cast<int>(static_cast<float>(m_avEncContext->framerate.num) /
                             m_avEncContext->framerate.den);

    switch (m_encCodecId) {
        case MFX_CODEC_HEVC:
            sts = InitHEVCParams(par);
            if (sts != MFX_ERR_NONE)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        case MFX_CODEC_AV1:
            sts = InitAV1Params(par);
            if (sts != MFX_ERR_NONE)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        case MFX_CODEC_AVC:
            sts = InitAVCParams(par);
            if (sts != MFX_ERR_NONE)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        case MFX_CODEC_JPEG:
            sts = InitJPEGParams(par);
            if (sts != MFX_ERR_NONE)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
    }

#ifdef ENABLE_LIBAV_AUTO_THREADS
    m_avEncContext->thread_count = 0;
#endif

    int err = 0;
    err     = avcodec_open2(m_avEncContext, m_avEncCodec, NULL);
    if (err)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    m_avEncFrameIn = av_frame_alloc();
    if (!m_avEncFrameIn)
        return MFX_ERR_MEMORY_ALLOC;

    m_avEncFrameIn->format = m_avEncContext->pix_fmt;
    m_avEncFrameIn->width  = m_avEncContext->width;
    m_avEncFrameIn->height = m_avEncContext->height;

    m_encInit = true;
    return MFX_ERR_NONE;
}

mfxStatus CpuWorkstream::InitHEVCParams(mfxVideoParam *par) {
    int ret;

    // set rate control
    if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP) {
        // SVT-HEVC rc 0=CBR
        ret = av_opt_set(m_avEncContext->priv_data,
                         "rc",
                         "0",
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        // since SVT-HEVC does not distinguish between QPI/P/B, use QPP value
        std::stringstream qpss;
        qpss << par->mfx.QPP;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "qp",
                         qpss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        //SVT-HEVC rc 1=VBR
        ret = av_opt_set(m_avEncContext->priv_data,
                         "rc",
                         "1",
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        m_avEncContext->bit_rate =
            par->mfx.TargetKbps * 1000; // prop is in kbps;

        m_avEncContext->rc_initial_buffer_occupancy =
            par->mfx.InitialDelayInKB * 8000;
        m_avEncContext->rc_buffer_size = par->mfx.BufferSizeInKB * 1000;
        m_avEncContext->rc_max_rate    = par->mfx.MaxKbps * 1000;
    }

    if (par->mfx.TargetUsage) {
        // set targetUsage
        // note, HEVC encode can be 0-9 for <=1080p
        // 0-10 for 1082-4k
        // and 0-11 for >=4K
        // however, in the most common cases we don't know the resolution yet
        int encMode = 0;
        switch (par->mfx.TargetUsage) {
            case 1:
                encMode = 0;
                break;
            case 2:
                encMode = 1;
                break;
            case 3:
                encMode = 3;
                break;
            case 4:
                encMode = 5;
                break;
            case 5:
                encMode = 7;
                break;
            case 6:
                encMode = 8;
                break;
            case 7:
            default:
                encMode = 9;
                break;
        }
        std::stringstream tuss;
        tuss << encMode;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "preset",
                         tuss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.CodecProfile) {
        //       **Profile** | -profile | [1,2] | 2 | 1: Main, 2: Main 10 |
        ret = av_opt_set_int(
            m_avEncContext->priv_data,
            "profile",
            (par->mfx.CodecProfile == MFX_PROFILE_HEVC_MAIN10) ? 2 : 1,
            AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.CodecLevel) {
        //   int tier; | **Tier** | -tier | [0, 1] | 0 | 0: Main, 1: High |
        //   int level; | **Level** | -level | [1, 2, 2.1,3, 3.1, 4, 4.1, 5, 5.1, 5.2, 6, 6.1, 6.2] | 0 | 0 to 6.2 [0 for auto determine Level] |

        //   In MSDK, tier is combined with level.  In SVT-HEVC it is set
        //   separately.
        std::stringstream tierss;
        tierss << (par->mfx.CodecLevel & MFX_TIER_HEVC_HIGH) ? 1 : 0;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "tier",
                         tierss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        std::string levelstr;
        uint8_t level = par->mfx.CodecLevel & 0xFF;
        switch (level) {
            case MFX_LEVEL_HEVC_1:
                levelstr = "1";
                break;
            case MFX_LEVEL_HEVC_2:
                levelstr = "2";
                break;
            case MFX_LEVEL_HEVC_21:
                levelstr = "2.1";
                break;
            case MFX_LEVEL_HEVC_3:
                levelstr = "3";
                break;
            case MFX_LEVEL_HEVC_31:
                levelstr = "3.1";
                break;
            case MFX_LEVEL_HEVC_4:
                levelstr = "4";
                break;
            case MFX_LEVEL_HEVC_41:
                levelstr = "4.1";
                break;
            case MFX_LEVEL_HEVC_5:
                levelstr = "5";
                break;
            case MFX_LEVEL_HEVC_51:
                levelstr = "5.1";
                break;
            case MFX_LEVEL_HEVC_52:
                levelstr = "5.2";
                break;
            case MFX_LEVEL_HEVC_6:
                levelstr = "6";
                break;
            case MFX_LEVEL_HEVC_61:
                levelstr = "6.1";
                break;
            case MFX_LEVEL_HEVC_62:
                levelstr = "6.2";
                break;
            default:
                return MFX_ERR_INVALID_VIDEO_PARAM;
                break;
        }
        ret = av_opt_set(m_avEncContext->priv_data,
                         "level",
                         tierss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuWorkstream::InitAVCParams(mfxVideoParam *par) {
    return MFX_ERR_NONE;
}

mfxStatus CpuWorkstream::InitJPEGParams(mfxVideoParam *par) {
    if (par->mfx.Quality) {
        uint32_t jpegQuality;

        // convert scale from 1 - 100 (VPL, worst to best) to
        //   2 - 31 (ffmpeg, best to worst)
        float q = (float)(par->mfx.Quality);
        q       = 31 - ((31 - 2) * (q - 1) / (100 - 1));

        jpegQuality = (int)(q + 0.5f);
        if (jpegQuality < 2)
            jpegQuality = 2;
        if (jpegQuality > 31)
            jpegQuality = 31;

        // enable CQP for MJPEG
        m_avEncContext->flags |= AV_CODEC_FLAG_QSCALE;
        m_avEncContext->global_quality = jpegQuality * FF_QP2LAMBDA;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuWorkstream::InitAV1Params(mfxVideoParam *par) {
    int ret;

    // set AV1 rate control (0=CQP, 1=VBR, 2 = CVBR)
    if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP) {
        //SVT-AV1 rc 0=CBR
        ret = av_opt_set(m_avEncContext->priv_data,
                         "rc",
                         "0",
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        // since SVT-AV1 does not distinguish between QPI/P/B, use the QPP value
        std::stringstream qpss;
        qpss << par->mfx.QPP;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "qp",
                         qpss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else if (par->mfx.RateControlMethod == MFX_RATECONTROL_CBR) {
        // since CVBR is available using this as a slightly better map than just
        // defaulting to VBR

        // SVT-AV1 rc 2=CVBR
        ret = av_opt_set(m_avEncContext->priv_data,
                         "rc",
                         "2",
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        m_avEncContext->bit_rate =
            par->mfx.TargetKbps * 1000; // prop is in kbps;

        m_avEncContext->rc_initial_buffer_occupancy =
            par->mfx.InitialDelayInKB * 8000;
        m_avEncContext->rc_buffer_size = par->mfx.BufferSizeInKB * 1000;
        m_avEncContext->rc_max_rate    = par->mfx.MaxKbps * 1000;
    }
    else {
        // default to VBR
        // SVT-HEVC rc 1=VBR
        ret = av_opt_set(m_avEncContext->priv_data,
                         "rc",
                         "1",
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        m_avEncContext->bit_rate =
            par->mfx.TargetKbps * 1000; // prop is in kbps;

        m_avEncContext->rc_initial_buffer_occupancy =
            par->mfx.InitialDelayInKB * 8000;
        m_avEncContext->rc_buffer_size = par->mfx.BufferSizeInKB * 1000;
        m_avEncContext->rc_max_rate    = par->mfx.MaxKbps * 1000;
    }

    // set targetUsage
    // note, AV1 encode can be 0-8
    if (par->mfx.TargetUsage) {
        int encMode = 0;
        switch (par->mfx.TargetUsage) {
            case 1:
                encMode = 0;
                break;
            case 2:
                encMode = 1;
                break;
            case 3:
                encMode = 3;
                break;
            case 4:
                encMode = 5;
                break;
            case 5:
                encMode = 6;
                break;
            case 6:
                encMode = 7;
                break;
            case 7:
            default:
                encMode = 8;
                break;
        }
        std::stringstream tuss;
        tuss << encMode;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "preset",
                         tuss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return MFX_ERR_NONE;
}

void CpuWorkstream::FreeEncode(void) {
    if (m_avEncFrameIn) {
        av_frame_free(&m_avEncFrameIn);
        m_avEncFrameIn = nullptr;
    }

    if (m_avEncPacket) {
        av_packet_free(&m_avEncPacket);
        m_avEncPacket = nullptr;
    }

    if (m_avEncContext) {
        avcodec_close(m_avEncContext);
        avcodec_free_context(&m_avEncContext);
        m_avEncContext = nullptr;
    }
}

mfxStatus CpuWorkstream::EncodeFrame(mfxFrameSurface1 *surface,
                                     mfxBitstream *bs) {
    int err = 0;

    // encode one frame
    if (surface) {
        m_avEncFrameIn->data[0] = surface->Data.Y;
        m_avEncFrameIn->data[1] = surface->Data.U;
        m_avEncFrameIn->data[2] = surface->Data.V;

        m_avEncFrameIn->linesize[0] = surface->Data.Pitch;
        m_avEncFrameIn->linesize[1] = surface->Data.Pitch / 2;
        m_avEncFrameIn->linesize[2] = surface->Data.Pitch / 2;

        if (m_encCodecId == MFX_CODEC_JPEG) {
            // must be set for every frame
            m_avEncFrameIn->quality = m_avEncContext->global_quality;
        }

        err = avcodec_send_frame(m_avEncContext, m_avEncFrameIn);
        if (err < 0)
            return MFX_ERR_UNKNOWN;
    }
    else {
        // send NULL packet to drain frames
        err = avcodec_send_frame(m_avEncContext, NULL);
        if (err < 0 && err != AVERROR_EOF)
            return MFX_ERR_UNKNOWN;
    }

    // get encoded packet, if available
    mfxU32 nBytesOut = 0, nBytesAvail = 0;

    err = avcodec_receive_packet(m_avEncContext, m_avEncPacket);
    if (err == AVERROR(EAGAIN)) {
        // need more data - nothing to do
        return MFX_ERR_MORE_DATA;
    }
    else if (err == AVERROR_EOF) {
        return MFX_ERR_MORE_DATA;
    }
    else if (err < 0) {
        // other error
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
    else if (err == 0) {
        // copy encoded data to output buffer
        nBytesOut   = m_avEncPacket->size;
        nBytesAvail = bs->MaxLength - (bs->DataLength + bs->DataOffset);

        if (nBytesOut > nBytesAvail) {
            //error if encoded bytes out is larger than provided output buffer size
            return MFX_ERR_NOT_ENOUGH_BUFFER;
        }
        memcpy_s(bs->Data + bs->DataOffset,
                 nBytesAvail,
                 m_avEncPacket->data,
                 nBytesOut);
        bs->DataLength += nBytesOut;
    }

    av_packet_unref(m_avEncPacket);

    return MFX_ERR_NONE;
}

mfxStatus CpuWorkstream::EncodeQueryIOSurf(mfxVideoParam *par,
                                           mfxFrameAllocRequest *request) {
    // may be null for internal use
    if (par)
        request->Info = par->mfx.FrameInfo;
    else
        memset(&request->Info, 0, sizeof(mfxFrameInfo));

    request->NumFrameMin       = 3; // TO DO - calculate correctly from libav
    request->NumFrameSuggested = 3;
    request->Type = MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_FROM_ENCODE;

    return MFX_ERR_NONE;
}

mfxStatus CpuWorkstream::EncodeQuery(mfxVideoParam *in, mfxVideoParam *out) {
    mfxStatus sts = MFX_ERR_NONE;

    if (in) {
        // save a local copy of in, since user may set out == in
        mfxVideoParam inCopy = *in;
        in                   = &inCopy;

        // start with out = copy of in (does not deep copy extBufs)
        *out = *in;

        // validate fields in the input param struct
        sts = ValidateEncodeParams(in);
    }
    else {
        memset(out, 0, sizeof(mfxVideoParam));

        // set output struct to zero for unsupported params, non-zero for supported params
    }

    return sts;
}
