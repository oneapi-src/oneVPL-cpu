#include "cpu_workstream.h"

#ifdef ENABLE_ENCODE

mfxStatus CpuWorkstream::InitEncode(mfxVideoParam *par) {
    m_encCodecId = par->mfx.CodecId;

    AVCodecID cid = AV_CODEC_ID_NONE;

    switch (m_encCodecId) {
        case MFX_CODEC_AVC:
            cid = AV_CODEC_ID_H264;
            break;
        case MFX_CODEC_HEVC:
            cid = AV_CODEC_ID_HEVC;
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

    // set minimal parameters and keep defaults for everything else
    m_avEncContext->width  = par->mfx.FrameInfo.Width;
    m_avEncContext->height = par->mfx.FrameInfo.Height;

    m_avEncContext->gop_size = par->mfx.GopRefDist;
    m_avEncContext->bit_rate = par->mfx.TargetKbps * 1000; // prop is in kbps;

    m_avEncContext->framerate.num = par->mfx.FrameInfo.FrameRateExtN;
    m_avEncContext->framerate.den = par->mfx.FrameInfo.FrameRateExtD;
    m_avEncContext->time_base.num = par->mfx.FrameInfo.FrameRateExtD;
    m_avEncContext->time_base.den = par->mfx.FrameInfo.FrameRateExtN;

    // set defaults for anything not passed in
    if (!m_avEncContext->gop_size)
        m_avEncContext->gop_size =
            2 * (int)((float)m_avEncContext->framerate.num /
                      m_avEncContext->framerate.den);

    // set codec-specific parameters
    if (m_encCodecId == MFX_CODEC_HEVC) {
        // set SVT rate control
        if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP) {
            // TODO - plumb QPI/QPP/QPB
            av_opt_set(m_avEncContext, "rc", "cqp", AV_OPT_SEARCH_CHILDREN);
        }
        else {
            av_opt_set(m_avEncContext, "rc", "vbr", AV_OPT_SEARCH_CHILDREN);
        }
    }

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

    return MFX_ERR_NONE;
}

void CpuWorkstream::FreeEncode(void) {
    if (m_avEncFrameIn) {
        av_frame_free(&m_avEncFrameIn);
    }

    if (m_avEncPacket) {
        av_packet_free(&m_avEncPacket);
    }

    if (m_avEncContext) {
        avcodec_close(m_avEncContext);
        avcodec_free_context(&m_avEncContext);
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

#else // ENABLE_ENCODE

mfxStatus CpuWorkstream::InitEncode(mfxVideoParam *par) {
    return MFX_ERR_UNSUPPORTED;
}

void CpuWorkstream::FreeEncode(void) {
    return;
}

mfxStatus CpuWorkstream::EncodeFrame(mfxFrameSurface1 *surface,
                                     mfxBitstream *bs) {
    return MFX_ERR_UNSUPPORTED;
}

#endif // ENABLE_ENCODE
