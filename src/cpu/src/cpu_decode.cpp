/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"

#define DEFAULT_MAX_DEC_BITSTREAM_SIZE (1024 * 1024 * 64)

// callback:
// int (*get_buffer2)(struct AVCodecContext *s, AVFrame *frame, int flags);

static int get_buffer2_msdk(struct AVCodecContext *s,
                            AVFrame *frame,
                            int flags) {
    // if AV_CODEC_CAP_DR1 is not set, use default method
    return avcodec_default_get_buffer2(s, frame, flags);
}

mfxStatus CpuWorkstream::InitDecode(mfxU32 FourCC) {
    // alloc bitstream buffer
    m_bsDecValidBytes = 0;
    m_bsDecMaxBytes =
        DEFAULT_MAX_DEC_BITSTREAM_SIZE + AV_INPUT_BUFFER_PADDING_SIZE;
    m_bsDecData = new uint8_t[m_bsDecMaxBytes];

    if (!m_bsDecData)
        return MFX_ERR_MEMORY_ALLOC;

    m_decCodecId  = FourCC;
    AVCodecID cid = AV_CODEC_ID_NONE;
    switch (m_decCodecId) {
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

    m_decInit = true;

    return MFX_ERR_NONE;
}

void CpuWorkstream::FreeDecode() {
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

    if (m_bsDecData) {
        delete[] m_bsDecData;
        m_bsDecData = nullptr;
    }
}

mfxStatus CpuWorkstream::DecodeHeader(mfxBitstream *bs, mfxVideoParam *par) {
    mfxFrameSurface1 *surface_out;
    mfxStatus sts;

    if (m_decInit == false) {
        sts = InitDecode(par->mfx.CodecId);
        if (sts < 0) {
            // error - can't continue
            return sts;
        }
        m_decInit = true;
    }

    sts = DecodeFrame(bs, nullptr, &surface_out);
    if (sts < 0) {
        // may return MFX_ERR_MORE_DATA
        return sts;
    }

    // just fills in the minimum parameters required to alloc buffers and start decoding
    // in next step, the app will call DECODE_Query() to confirm that it can decode this stream
    m_decWidth  = m_avDecContext->width;
    m_decHeight = m_avDecContext->height;

    if (m_avDecContext->pix_fmt == AV_PIX_FMT_YUV420P10LE)
        m_decOutFormat = MFX_FOURCC_I010;
    else if (m_avDecContext->pix_fmt == AV_PIX_FMT_YUV420P)
        m_decOutFormat = MFX_FOURCC_I420;
    else
        m_decOutFormat = MFX_FOURCC_I420;

    // fill in mfxVideoParam
    par->mfx.FrameInfo.Width  = (uint16_t)m_decWidth;
    par->mfx.FrameInfo.Height = (uint16_t)m_decHeight;
    par->mfx.FrameInfo.FourCC = m_decOutFormat;

    if (m_avDecContext->width == 0 || m_avDecContext->height == 0) {
        return MFX_ERR_NOT_INITIALIZED;
    }

    //reset the decoder
    FreeDecode();

    m_decInit = false;

    return MFX_ERR_NONE;
}

// bs == 0 is a signal to drain
mfxStatus CpuWorkstream::DecodeFrame(mfxBitstream *bs,
                                     mfxFrameSurface1 *surface_work,
                                     mfxFrameSurface1 **surface_out) {
    if (!m_decInit) {
        ERR_EXIT(VPL_WORKSTREAM_DECODE);
    }

    // copy new data into bitstream buffer
    if (bs && bs->DataLength > 0) {
        memcpy_s(m_bsDecData + m_bsDecValidBytes,
                 bs->DataLength,
                 bs->Data + bs->DataOffset,
                 bs->DataLength);
        m_bsDecValidBytes += (uint32_t)bs->DataLength;
    }

    // parse a packet
    // if packet is ready, m_avDecPacket->data and m_avDecPacket->size will be non-zero
    // otherwise, m_avDecPacket->size is 0 and parser needs more data to produce a packet
    // returns number of bytes consumed
    //
    // m_avDecPacket->data may point to the input buffer m_bsDecData after calling this,
    //   so do not overwrite m_bsDecData until calling avcodec_send_packet()
    int bytesParsed = av_parser_parse2(m_avDecParser,
                                       m_avDecContext,
                                       &m_avDecPacket->data,
                                       &m_avDecPacket->size,
                                       m_bsDecData,
                                       m_bsDecValidBytes,
                                       AV_NOPTS_VALUE,
                                       AV_NOPTS_VALUE,
                                       0);

    if (m_avDecPacket->size == 0) {
        if (bs == 0 && bytesParsed == 0) {
            // start draining
            m_avDecPacket->data = NULL;
            m_avDecPacket->size = 0;
        }
        else if (m_bsDecValidBytes - bytesParsed == 0) {
            // bitstream is empty - need more data from app
            m_bsDecValidBytes = 0;

            return MFX_ERR_MORE_DATA;
        }
    }

    int av_ret = 0;
    while (1) {
        // parse a packet
        // if packet is ready, m_avDecPacket->data and m_avDecPacket->size will be non-zero
        // otherwise, m_avDecPacket->size is 0 and parser needs more data to produce a packet
        // returns number of bytes consumed
        //
        // m_avDecPacket->data may point to the input buffer m_bsDecData after calling this,
        //   so do not overwrite m_bsDecData until calling avcodec_send_packet()

        av_ret = avcodec_send_packet(m_avDecContext, m_avDecPacket);

        memmove(m_bsDecData,
                m_bsDecData + bytesParsed,
                m_bsDecValidBytes - bytesParsed);
        m_bsDecValidBytes -= bytesParsed;

        // try to get a decoded frame
        av_ret = avcodec_receive_frame(m_avDecContext, m_avDecFrameOut);

        if (av_ret != AVERROR(EAGAIN)) {
            // don't need more data
            break;
        }

        bytesParsed = av_parser_parse2(m_avDecParser,
                                       m_avDecContext,
                                       &m_avDecPacket->data,
                                       &m_avDecPacket->size,
                                       m_bsDecData,
                                       m_bsDecValidBytes,
                                       AV_NOPTS_VALUE,
                                       AV_NOPTS_VALUE,
                                       0);
    }

    if (av_ret == AVERROR_EOF) {
        // no more data left to drain, processing is done
        return MFX_ERR_MORE_DATA;
    }

    // frame successfully decoded
    // don't save output frame if surface_work is 0 (e.g. DecodeHeader)
    if (surface_work) {
        AVFrame2mfxFrameSurface(surface_work, m_avDecFrameOut);
        *surface_out = surface_work;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuWorkstream::DecodeQueryIOSurf(mfxVideoParam *par,
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

mfxStatus CpuWorkstream::DecodeQuery(mfxVideoParam *in, mfxVideoParam *out) {
    mfxStatus sts = MFX_ERR_NONE;

    //CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
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
