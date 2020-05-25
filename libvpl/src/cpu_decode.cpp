#include "cpu_workstream.h"

#define DEFAULT_MAX_DEC_BITSTREAM_SIZE (1024 * 1024 * 64)

// callback:
// int (*get_buffer2)(struct AVCodecContext *s, AVFrame *frame, int flags);

static int get_buffer2_msdk(struct AVCodecContext *s, AVFrame *frame, int flags)
{
    if (1) {
        // if AV_CODEC_CAP_DR1 is not set, use default method
        return avcodec_default_get_buffer2(s, frame, flags);
    }
    else {
    }

    return 0;
}

mfxStatus CpuWorkstream::InitDecode(mfxU32 FourCC)
{
    // TODO - get rid of internal BS buffer (use mfxBitstream)
    // alloc bitstream buffer
    m_bsDecValidBytes = 0;
    m_bsDecMaxBytes = DEFAULT_MAX_DEC_BITSTREAM_SIZE + AV_INPUT_BUFFER_PADDING_SIZE;
    m_bsDecData = new uint8_t[m_bsDecMaxBytes];

    if (!m_bsDecData)
        return MFX_ERR_MEMORY_ALLOC;

    AVCodecID cid = AV_CODEC_ID_NONE;
    switch (FourCC) {
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

void CpuWorkstream::FreeDecode()
{
    if (m_avDecFrameOut) {
        av_frame_free(&m_avDecFrameOut);
    }

    if (m_avDecParser) {
        av_parser_close(m_avDecParser);
    }

    if (m_avDecPacket) {
        av_packet_free(&m_avDecPacket);
    }

    if (m_avDecContext) {
        avcodec_close(m_avDecContext);
        avcodec_free_context(&m_avDecContext);
    }

    if (m_bsDecData)
        delete[] m_bsDecData;
    m_bsDecData = nullptr;
}

// bs == 0 is a signal to drain
mfxStatus CpuWorkstream::DecodeFrame(mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out) {
    int av_ret = 0;

    if (!m_decInit) {
        ERR_EXIT(VPL_WORKSTREAM_DECODE);
    }

    // TODO - get rid of internal BS buffer (use mfxBitstream)

    // copy new data into bitstream buffer
    if (bs && bs->DataLength > 0) {
        memcpy(m_bsDecData + m_bsDecValidBytes, bs->Data + bs->DataOffset, bs->DataLength);
        m_bsDecValidBytes += (uint32_t)bs->DataLength;
    }

    int bytesParsed = 0;

    while (1) {
        // parse a packet
        // if packet is ready, m_avDecPacket->data and m_avDecPacket->size will be non-zero
        // otherwise, m_avDecPacket->size is 0 and parser needs more data to produce a packet
        // returns number of bytes consumed
        //
        // m_avDecPacket->data may point to the input buffer m_bsDecData after calling this, 
        //   so do not overwrite m_bsDecData until calling avcodec_send_packet()
        bytesParsed += av_parser_parse2(m_avDecParser, m_avDecContext, &m_avDecPacket->data, &m_avDecPacket->size,
            m_bsDecData + bytesParsed, (int)m_bsDecValidBytes - bytesParsed, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

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

        // TODO: get_buffer2_msdk() will be called from inside avcodec_send_packet()
        // this is where we can associate the user-allocated decoded output frame buffer
        //   (surface_work) with the compressed frame
        // so need to break up this function so it only calls avcodec_send_packet() one time
        // avcodec_receive_frame() should be part of SyncOp (or separate thread)

        // send prepared packets from parser
        //printf("Sending packet - size = % 8d bytes\n", m_avDecPacket->size);
        av_ret = avcodec_send_packet(m_avDecContext, m_avDecPacket);
        if (av_ret < 0 && av_ret != AVERROR_EOF)
            ERR_EXIT(VPL_WORKSTREAM_DECODE);

        // remove used data from bitstream after sending packet
        // packets are not ref counted, so data will be copied internally by avcodec_send_packet()
        memmove(m_bsDecData, m_bsDecData + bytesParsed, m_bsDecValidBytes - bytesParsed);
        m_bsDecValidBytes -= bytesParsed;
        bytesParsed = 0;

        // try to get a decoded frame
        av_ret = avcodec_receive_frame(m_avDecContext, m_avDecFrameOut);

        if (av_ret == AVERROR(EAGAIN)) {
            // need more data
            continue;
        }
        else if (av_ret == AVERROR_EOF) {
            // all done
            return MFX_ERR_MORE_DATA;
        }
        else if (av_ret < 0) {
            // unknown error
            ERR_EXIT(VPL_WORKSTREAM_DECODE);
        }
        else {
            // frame successfully decoded
            // don't save output frame if surface_work is 0 (e.g. DecodeHeader)
            if (surface_work) {
                mfxU32 w, h, y, pitch, offset;

                surface_work->Info.Width = m_avDecContext->width;
                surface_work->Info.Height = m_avDecContext->height;

                surface_work->Info.CropX = 0;
                surface_work->Info.CropY = 0;
                surface_work->Info.CropW = m_avDecContext->width;
                surface_work->Info.CropH = m_avDecContext->height;

                surface_work->Data.Pitch = m_avDecContext->width;

                w = surface_work->Info.Width;
                h = surface_work->Info.Height;
                pitch = surface_work->Data.Pitch;

                // copy Y plane
                for (y = 0; y < h; y++) {
                    offset = pitch * (y + surface_work->Info.CropY) + surface_work->Info.CropX;
                    memcpy(surface_work->Data.Y + offset, m_avDecFrameOut->data[0] + y * m_avDecFrameOut->linesize[0], m_avDecFrameOut->width);
                }

                // copy U plane
                for (y = 0; y < h / 2; y++) {
                    offset = pitch / 2 * (y + surface_work->Info.CropY) + surface_work->Info.CropX;
                    memcpy(surface_work->Data.U + offset, m_avDecFrameOut->data[1] + y * m_avDecFrameOut->linesize[1], m_avDecFrameOut->width / 2);
                }

                // copy V plane
                for (y = 0; y < h / 2; y++) {
                    offset = pitch / 2 * (y + surface_work->Info.CropY) + surface_work->Info.CropX;
                    memcpy(surface_work->Data.V + offset, m_avDecFrameOut->data[2] + y * m_avDecFrameOut->linesize[2], m_avDecFrameOut->width / 2);
                }

                *surface_out = surface_work;
            }

#if 0
            // TODO - if codec capability AV_CODEC_CAP_DR1 is set, can use user-provided buffers (callback)
            int wTmp = 1280, hTmp = 720, linesize_align[AV_NUM_DATA_POINTERS];
            avcodec_align_dimensions2(m_avDecContext, &wTmp, &hTmp, linesize_align);
#endif

            return MFX_ERR_NONE;
        }
    }
}

mfxStatus CpuWorkstream::DecodeGetVideoParams(mfxVideoParam *par)
{
    // call after decoding at least one frame
    par->mfx.FrameInfo.Width = (uint16_t)m_avDecContext->width;
    par->mfx.FrameInfo.Height = (uint16_t)m_avDecContext->height;

    if (m_avDecContext->width == 0 || m_avDecContext->height == 0) {
        return MFX_ERR_NOT_INITIALIZED;
    }

    return MFX_ERR_NONE;
}
