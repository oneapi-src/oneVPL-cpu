/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_decode.h"
#include <memory>
#include <utility>
#include "src/cpu_workstream.h"

// callback:
// int (*get_buffer2)(struct AVCodecContext *s, AVFrame *frame, int flags);

static int get_buffer2_msdk(struct AVCodecContext *s,
                            AVFrame *frame,
                            int flags) {
    // if AV_CODEC_CAP_DR1 is not set, use default method
    return avcodec_default_get_buffer2(s, frame, flags);
}

CpuDecode::CpuDecode(CpuWorkstream *session)
        : m_session(session),
          m_avDecCodec(nullptr),
          m_avDecContext(nullptr),
          m_avDecParser(nullptr),
          m_avDecPacket(nullptr),
          m_avDecFrameOut(nullptr),
          m_param(),
          m_decSurfaces(),
          m_bExtMemToBeReallocated(false),
          m_prevAVFrameWidth(0),
          m_prevAVFrameHeight(0) {}

mfxStatus CpuDecode::ValidateDecodeParams(mfxVideoParam *par) {
    //only system memory allowed
    if (par->IOPattern != MFX_IOPATTERN_OUT_SYSTEM_MEMORY)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    //only I420 and I010 colorspaces allowed
    switch (par->mfx.FrameInfo.FourCC) {
        case MFX_FOURCC_I420:
        case MFX_FOURCC_I010:
            //allowed FourCCs
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    //Must have width and height
    if (par->mfx.FrameInfo.Width == 0 || par->mfx.FrameInfo.Height == 0) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    //width and height must be <= max
    if (par->mfx.FrameInfo.Width > MAX_WIDTH ||
        par->mfx.FrameInfo.Height > MAX_HEIGHT ||
        par->mfx.FrameInfo.CropW > MAX_WIDTH ||
        par->mfx.FrameInfo.CropH > MAX_HEIGHT) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    //BitDepthLuma can only be 8 or 10
    switch (par->mfx.FrameInfo.BitDepthLuma) {
        case 8:
        case 10:
        case 0:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    //BitDepthChroma can only be 8 or 10
    switch (par->mfx.FrameInfo.BitDepthChroma) {
        case 8:
        case 10:
        case 0:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.CodecProfile > 0x1FF)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.CodecLevel > 0x1FF)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

//InitDecode can operate in two modes:
// With no bitstream: assumes header decoded elsewhere, validates params given
// With bitstream
//  1. Attempts to decode a frame
//  2. Gets parameters
mfxStatus CpuDecode::InitDecode(mfxVideoParam *par, mfxBitstream *bs) {
    AVCodecID cid = AV_CODEC_ID_NONE;
    switch (par->mfx.CodecId) {
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

    if (!bs) {
        mfxStatus sts = ValidateDecodeParams(par);
        if (sts != MFX_ERR_NONE)
            return sts;
    }

    m_avDecCodec = avcodec_find_decoder(cid);
    if (!m_avDecCodec) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    m_avDecContext = avcodec_alloc_context3(m_avDecCodec);
    if (!m_avDecContext) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    m_avDecContext->get_buffer2 = get_buffer2_msdk;

    m_avDecParser = av_parser_init(m_avDecCodec->id);
    if (!m_avDecParser) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

#ifdef ENABLE_LIBAV_AUTO_THREADS
    m_avDecContext->thread_count = 0;
#endif

    if (avcodec_open2(m_avDecContext, m_avDecCodec, NULL) < 0) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    m_avDecPacket = av_packet_alloc();
    if (!m_avDecPacket) {
        return MFX_ERR_MEMORY_ALLOC;
    }

    m_avDecFrameOut = av_frame_alloc();
    if (!m_avDecFrameOut) {
        return MFX_ERR_MEMORY_ALLOC;
    }

    m_param = *par;

    if (bs) {
        // create copy to not modify caller's mfxBitstream
        // todo: this only works if input is large enough to
        // decode a frame
        mfxBitstream bs2 = *bs;
        DecodeFrame(&bs2, nullptr, nullptr);
        GetVideoParam(par);

        //RET_ERROR(ValidateDecodeParams(par));
    }

    return MFX_ERR_NONE;
}

CpuDecode::~CpuDecode() {
    if (m_avDecFrameOut) {
        av_frame_free(&m_avDecFrameOut);
        m_avDecFrameOut = nullptr;
    }

    if (m_avDecParser) {
        av_parser_close(m_avDecParser);
        m_avDecParser = nullptr;
    }

    if (m_avDecPacket) {
        av_packet_free(&m_avDecPacket);
        m_avDecPacket = nullptr;
    }

    if (m_avDecContext) {
        avcodec_close(m_avDecContext);
        avcodec_free_context(&m_avDecContext);
        m_avDecContext = nullptr;
    }
}

// bs == 0 is a signal to drain
mfxStatus CpuDecode::DecodeFrame(mfxBitstream *bs,
                                 mfxFrameSurface1 *surface_work,
                                 mfxFrameSurface1 **surface_out) {
    if (bs == nullptr && m_bExtMemToBeReallocated == true) {
        if (surface_work && surface_out) {
            RET_ERROR(AVFrame2mfxFrameSurface(surface_work,
                                              m_avDecFrameOut,
                                              m_session->GetFrameAllocator()));

            *surface_out             = surface_work;
            m_bExtMemToBeReallocated = false;

            return MFX_ERR_NONE;
        }
    }

    // Try get AVFrame from surface_work
    AVFrame *avframe    = nullptr;
    CpuFrame *cpu_frame = CpuFrame::TryCast(surface_work);
    if (cpu_frame) {
        avframe = cpu_frame->GetAVFrame();
    }
    if (!avframe) { // Otherwise use AVFrame allocated in this class
        avframe = m_avDecFrameOut;
    }

    for (;;) {
        // parse
        auto data_ptr    = bs ? (bs->Data + bs->DataOffset) : nullptr;
        int data_size    = bs ? bs->DataLength : 0;
        int bytes_parsed = av_parser_parse2(m_avDecParser,
                                            m_avDecContext,
                                            &m_avDecPacket->data,
                                            &m_avDecPacket->size,
                                            data_ptr,
                                            data_size,
                                            AV_NOPTS_VALUE,
                                            AV_NOPTS_VALUE,
                                            0);
        if (bs && bytes_parsed) {
            bs->DataOffset += bytes_parsed;
            bs->DataLength -= bytes_parsed;
        }

        // send packet
        if (m_avDecPacket->size) {
            auto av_ret = avcodec_send_packet(m_avDecContext, m_avDecPacket);
            if (av_ret < 0) {
                return MFX_ERR_ABORTED;
            }
        }
        // send EOF packet
        if (!bs) {
            avcodec_send_packet(m_avDecContext, nullptr);
        }

        // receive frame
        auto av_ret = avcodec_receive_frame(m_avDecContext, avframe);
        if (av_ret == 0) {
            if (m_param.mfx.FrameInfo.Width != m_avDecContext->width ||
                m_param.mfx.FrameInfo.Height != m_avDecContext->height) {
                m_param.mfx.FrameInfo.Width  = m_avDecContext->width;
                m_param.mfx.FrameInfo.Height = m_avDecContext->height;

                if (m_avDecContext->pix_fmt == AV_PIX_FMT_YUV420P10LE)
                    m_param.mfx.FrameInfo.FourCC = MFX_FOURCC_I010;
                else if (m_avDecContext->pix_fmt == AV_PIX_FMT_YUV420P)
                    m_param.mfx.FrameInfo.FourCC = MFX_FOURCC_I420;
                else
                    m_param.mfx.FrameInfo.FourCC = MFX_FOURCC_I420;
            }

            if (!m_decSurfaces) { // external memory mode
                if (surface_out) { // regular decode process, not part of decodeheader() process
                    // if resolution is changed
                    if (m_prevAVFrameWidth != avframe->width ||
                        m_prevAVFrameHeight != avframe->height) {
                        if (m_prevAVFrameWidth == 0 &&
                            m_prevAVFrameHeight == 0) {
                            m_prevAVFrameWidth  = avframe->width;
                            m_prevAVFrameHeight = avframe->height;
                        }
                        else {
                            m_prevAVFrameWidth       = avframe->width;
                            m_prevAVFrameHeight      = avframe->height;
                            m_avDecFrameOut          = avframe;
                            m_bExtMemToBeReallocated = true;

                            return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
                        }
                    }
                }
            }
            if (surface_out) {
                if (avframe == m_avDecFrameOut) { // copy image data
                    RET_ERROR(AVFrame2mfxFrameSurface(
                        surface_work,
                        m_avDecFrameOut,
                        m_session->GetFrameAllocator()));
                }
                else {
                    if (cpu_frame) { // update MFXFrameSurface from AVFrame
                        cpu_frame->Update();
                    }
                }
                *surface_out = surface_work;
            }
            return MFX_ERR_NONE;
        }
        if (av_ret == AVERROR(EAGAIN)) {
            if (bs && bs->DataLength) {
                continue; // we have more input data
            }
            else {
                return MFX_ERR_MORE_DATA;
            }
        }
        if (av_ret == AVERROR_EOF) {
            return MFX_ERR_MORE_DATA;
        }
        return MFX_ERR_ABORTED;
    }
}

mfxStatus CpuDecode::DecodeQueryIOSurf(mfxVideoParam *par,
                                       mfxFrameAllocRequest *request) {
    // may be null for internal use
    if (par)
        request->Info = par->mfx.FrameInfo;
    else
        memset(&request->Info, 0, sizeof(mfxFrameInfo));

    request->NumFrameMin       = 16; // TO DO - calculate correctly from libav
    request->NumFrameSuggested = 16;
    request->Type = MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_FROM_DECODE;

    return MFX_ERR_NONE;
}

mfxStatus CpuDecode::DecodeQuery(mfxVideoParam *in, mfxVideoParam *out) {
    mfxStatus sts = MFX_ERR_NONE;

    if (in) {
        // save a local copy of in, since user may set out == in
        mfxVideoParam inCopy = *in;
        in                   = &inCopy;

        // start with out = copy of in (does not deep copy extBufs)
        *out = *in;

        // validate fields in the input param struct

        if (in->mfx.FrameInfo.Width == 0 || in->mfx.FrameInfo.Height == 0)
            sts = MFX_ERR_UNSUPPORTED;

        if (in->mfx.CodecId != MFX_CODEC_AVC &&
            in->mfx.CodecId != MFX_CODEC_HEVC &&
            in->mfx.CodecId != MFX_CODEC_AV1 &&
            in->mfx.CodecId != MFX_CODEC_JPEG &&
            in->mfx.CodecId != MFX_CODEC_MPEG2)
            sts = MFX_ERR_UNSUPPORTED;
    }
    else {
        memset(out, 0, sizeof(mfxVideoParam));

        // set output struct to zero for unsupported params, non-zero for supported params
    }

    return sts;
}

// return free surface and set refCount to 1
mfxStatus CpuDecode::GetDecodeSurface(mfxFrameSurface1 **surface) {
    if (!m_decSurfaces) {
        mfxFrameAllocRequest DecRequest = { 0 };
        RET_ERROR(DecodeQueryIOSurf(&m_param, &DecRequest));

        auto pool = std::make_unique<CpuFramePool>();
        RET_ERROR(pool->Init(DecRequest.NumFrameSuggested));
        m_decSurfaces = std::move(pool);
    }

    return m_decSurfaces->GetFreeSurface(surface);
}

mfxStatus CpuDecode::GetVideoParam(mfxVideoParam *par) {
    par->mfx = m_param.mfx;

    //Get parameters from the decode context
    //This allows checking if parameters have
    //been effectively set

    switch (m_avDecCodec->id) {
        case AV_CODEC_ID_H264:
            par->mfx.CodecId = MFX_CODEC_AVC;
            break;
        case AV_CODEC_ID_HEVC:
            par->mfx.CodecId = MFX_CODEC_HEVC;
            break;
        case AV_CODEC_ID_MJPEG:
            par->mfx.CodecId = MFX_CODEC_JPEG;
            break;
        case AV_CODEC_ID_AV1:
            par->mfx.CodecId = MFX_CODEC_AV1;
            break;
        default:
            par->mfx.CodecId = 0;
    }

    // resolution
    par->mfx.FrameInfo.Width  = (uint16_t)m_avDecContext->width;
    par->mfx.FrameInfo.Height = (uint16_t)m_avDecContext->height;
    par->mfx.FrameInfo.CropW  = (uint16_t)m_avDecContext->width;
    par->mfx.FrameInfo.CropH  = (uint16_t)m_avDecContext->height;

    // FourCC and chroma format
    switch (m_avDecContext->pix_fmt) {
        case AV_PIX_FMT_YUV420P10LE:
            par->mfx.FrameInfo.FourCC         = MFX_FOURCC_I010;
            par->mfx.FrameInfo.BitDepthLuma   = 10;
            par->mfx.FrameInfo.BitDepthChroma = 10;
            par->mfx.FrameInfo.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
            break;
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P:
            par->mfx.FrameInfo.FourCC         = MFX_FOURCC_IYUV;
            par->mfx.FrameInfo.BitDepthLuma   = 8;
            par->mfx.FrameInfo.BitDepthChroma = 8;
            par->mfx.FrameInfo.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
            break;
        default:
            //zero value after decodeheader indicates that
            //a supported decode fourcc could not be found
            par->mfx.FrameInfo.FourCC = 0;
    }

    // Frame rate
    par->mfx.FrameInfo.FrameRateExtD = (uint16_t)m_avDecContext->framerate.num;
    par->mfx.FrameInfo.FrameRateExtN = (uint16_t)m_avDecContext->framerate.den;

    // Aspect ratio
    par->mfx.FrameInfo.AspectRatioW =
        (uint16_t)m_avDecContext->sample_aspect_ratio.num;
    par->mfx.FrameInfo.AspectRatioH =
        (uint16_t)m_avDecContext->sample_aspect_ratio.den;

    // Profile/Level
    int profile = m_avDecContext->profile;
    int level   = m_avDecContext->level;

    switch (par->mfx.CodecId) {
        case MFX_CODEC_AV1:
            //if (profile==FF_PROFILE_AV1_MAIN)
            //if (profile==FF_PROFILE_AV1_HIGH)
            break;

        case MFX_CODEC_HEVC:
            if (profile == FF_PROFILE_HEVC_MAIN)
                par->mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN;
            else if (profile == FF_PROFILE_HEVC_MAIN_10)
                par->mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN10;

            //TODO(jeff) check if true for all levels
            par->mfx.CodecLevel = level;
            break;

        case MFX_CODEC_AVC:
            if (profile == FF_PROFILE_H264_BASELINE)
                par->mfx.CodecProfile = MFX_PROFILE_AVC_BASELINE;
            if (profile == FF_PROFILE_H264_MAIN)
                par->mfx.CodecProfile = MFX_PROFILE_AVC_MAIN;
            if (profile == FF_PROFILE_H264_HIGH)
                par->mfx.CodecProfile = MFX_PROFILE_AVC_HIGH;

            //TODO(jeff) check if true for all levels
            par->mfx.CodecLevel = level;

            break;
        case MFX_CODEC_JPEG:
            if (profile == FF_PROFILE_MJPEG_HUFFMAN_BASELINE_DCT)
                par->mfx.CodecProfile = MFX_PROFILE_JPEG_BASELINE;
            break;
        default:
            par->mfx.CodecProfile = 0;
            par->mfx.CodecLevel   = 0;
            return MFX_ERR_NONE;
    }

    par->IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    return MFX_ERR_NONE;
}
