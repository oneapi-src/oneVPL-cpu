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
          m_decSurfaces() {}

mfxStatus CpuDecode::InitDecode(mfxVideoParam *par) {
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
        case MFX_CODEC_MPEG2:
            cid = AV_CODEC_ID_MPEG2VIDEO;
            break;
        case MFX_CODEC_AV1:
            cid = AV_CODEC_ID_AV1;
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
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
        auto av_ret = avcodec_receive_frame(m_avDecContext, m_avDecFrameOut);
        if (av_ret == 0) {
            if (surface_work && surface_out) {
                RET_ERROR(
                    AVFrame2mfxFrameSurface(surface_work,
                                            m_avDecFrameOut,
                                            m_session->GetFrameAllocator()));
                *surface_out = surface_work;
            }
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
            //av_frame_free(&m_avDecFrameOut);
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

    request->NumFrameMin       = 4; // TO DO - calculate correctly from libav
    request->NumFrameSuggested = 4;
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
        RET_ERROR(pool->Init(m_param.mfx.FrameInfo.FourCC,
                             m_param.mfx.FrameInfo.Width,
                             m_param.mfx.FrameInfo.Height,
                             DecRequest.NumFrameSuggested));
        m_decSurfaces = std::move(pool);
    }

    return m_decSurfaces->GetFreeSurface(surface);
}

mfxStatus CpuDecode::GetVideoParam(mfxVideoParam *par) {
    //RET_IF_FALSE(m_param.mfx.FrameInfo.Width, MFX_ERR_NOT_INITIALIZED);
    //RET_IF_FALSE(m_param.mfx.FrameInfo.Height, MFX_ERR_NOT_INITIALIZED);
    //*par = m_param;
    par->mfx = m_param.mfx;
    return MFX_ERR_NONE;
}
