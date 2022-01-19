/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_common.h"
#include "src/frame_lock.h"

AVPixelFormat MFXFourCC2AVPixelFormat(uint32_t fourcc) {
    switch (fourcc) {
        case MFX_FOURCC_I010:
            return AV_PIX_FMT_YUV420P10LE;
        case MFX_FOURCC_P010:
            return AV_PIX_FMT_P010LE;
        case MFX_FOURCC_NV12:
            return AV_PIX_FMT_NV12;
        case MFX_FOURCC_RGB4:
            return AV_PIX_FMT_BGRA;
        case MFX_FOURCC_I420:
            return AV_PIX_FMT_YUV420P;
        case MFX_FOURCC_I422:
            return AV_PIX_FMT_YUV422P;
        case MFX_FOURCC_I210:
            return AV_PIX_FMT_YUV422P10LE;
    }
    return (AVPixelFormat)-1;
}

uint32_t AVPixelFormat2MFXFourCC(int format) {
    switch (format) {
        case AV_PIX_FMT_YUV420P10LE:
            return MFX_FOURCC_I010;
        case AV_PIX_FMT_P010LE:
            return MFX_FOURCC_P010;
        case AV_PIX_FMT_NV12:
            return MFX_FOURCC_NV12;
        case AV_PIX_FMT_BGRA:
            return MFX_FOURCC_RGB4;
        case AV_PIX_FMT_YUV420P:
            return MFX_FOURCC_I420;
        case AV_PIX_FMT_YUV422P:
            return MFX_FOURCC_I422;
        case AV_PIX_FMT_YUV422P10LE:
            return MFX_FOURCC_I210;
    }
    return 0;
}

AVCodecID MFXCodecId_to_AVCodecID(mfxU32 CodecId) {
    switch (CodecId) {
        case MFX_CODEC_AVC:
            return AV_CODEC_ID_H264;
        case MFX_CODEC_HEVC:
            return AV_CODEC_ID_HEVC;
        case MFX_CODEC_JPEG:
            return AV_CODEC_ID_MJPEG;
        case MFX_CODEC_AV1:
            return AV_CODEC_ID_AV1;
        case MFX_CODEC_MPEG2:
            return AV_CODEC_ID_MPEG2VIDEO;
    }
    return AV_CODEC_ID_NONE;
}

mfxU32 AVCodecID_to_MFXCodecId(AVCodecID CodecId) {
    switch (CodecId) {
        case AV_CODEC_ID_H264:
            return MFX_CODEC_AVC;
        case AV_CODEC_ID_HEVC:
            return MFX_CODEC_HEVC;
        case AV_CODEC_ID_MJPEG:
            return MFX_CODEC_JPEG;
        case AV_CODEC_ID_AV1:
            return MFX_CODEC_AV1;
        case AV_CODEC_ID_MPEG2VIDEO:
            return MFX_CODEC_MPEG2;
        default:
            return 0;
    }
    return 0;
}

mfxStatus AVFrame2mfxFrameSurface(mfxFrameSurface1 *surface,
                                  AVFrame *frame,
                                  mfxFrameAllocator *allocator) {
    FrameLock locker;
    RET_ERROR(locker.Lock(surface, MFX_MAP_WRITE, allocator));
    mfxFrameData *data = locker.GetData();
    mfxFrameInfo *info = &surface->Info;

    mfxU32 w, h, y, pitch, offset;

    RET_IF_FALSE(info->Width == frame->width && info->Height == frame->height,
                 MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

    info->CropX = 0;
    info->CropY = 0;
    info->CropW = frame->width;
    info->CropH = frame->height;
    switch (frame->format) {
        case AV_PIX_FMT_YUV420P10LE:
            info->FourCC         = MFX_FOURCC_I010;
            info->BitDepthLuma   = 10;
            info->BitDepthChroma = 10;
            info->ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
            break;
        case AV_PIX_FMT_YUV422P10LE:
            info->FourCC         = MFX_FOURCC_I210;
            info->BitDepthLuma   = 10;
            info->BitDepthChroma = 10;
            info->ChromaFormat   = MFX_CHROMAFORMAT_YUV422;
            break;
        case AV_PIX_FMT_YUV422P:
            info->FourCC         = MFX_FOURCC_I422;
            info->BitDepthLuma   = 8;
            info->BitDepthChroma = 8;
            info->ChromaFormat   = MFX_CHROMAFORMAT_YUV422;
            break;
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P:
            info->FourCC         = MFX_FOURCC_IYUV;
            info->BitDepthLuma   = 8;
            info->BitDepthChroma = 8;
            info->ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
            break;
        case AV_PIX_FMT_BGRA:
            info->FourCC         = MFX_FOURCC_BGRA;
            info->BitDepthLuma   = 8;
            info->BitDepthChroma = 8;
            info->ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
            break;
        default:
            info->FourCC         = 0;
            info->BitDepthLuma   = 0;
            info->BitDepthChroma = 0;
            break;
    }
    info->PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    if (frame->sample_aspect_ratio.num == 0 && frame->sample_aspect_ratio.den == 1) {
        info->AspectRatioW = 1;
        info->AspectRatioH = 1;
    }
    else {
        info->AspectRatioW = frame->sample_aspect_ratio.num;
        info->AspectRatioH = frame->sample_aspect_ratio.den;
    }
    if (frame->format == AV_PIX_FMT_YUV420P10LE) {
        RET_IF_FALSE(info->FourCC == MFX_FOURCC_I010, MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

        w = info->Width * 2;
        h = info->Height;
    }
    else if (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUVJ420P) {
        RET_IF_FALSE(info->FourCC == MFX_FOURCC_I420, MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

        w = info->Width;
        h = info->Height;
    }
    else if (frame->format == AV_PIX_FMT_YUV422P10LE) {
        RET_IF_FALSE(info->FourCC == MFX_FOURCC_I210, MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

        w = info->Width * 2;
        h = info->Height;
    }
    else if (frame->format == AV_PIX_FMT_YUV422P) {
        RET_IF_FALSE(info->FourCC == MFX_FOURCC_I422, MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

        w = info->Width;
        h = info->Height;
    }
    else if (frame->format == AV_PIX_FMT_BGRA) {
        RET_IF_FALSE(info->FourCC == MFX_FOURCC_RGB4, MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

        w = info->Width * 4;
        h = info->Height;
    }
    else {
        RET_ERROR(MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);
    }

    pitch = data->Pitch;

    if (frame->format == AV_PIX_FMT_BGRA) {
        for (y = 0; y < h; y++) {
            offset = pitch * (y + info->CropY) + info->CropX;
            memcpy_s(data->B + offset, w, frame->data[0] + y * frame->linesize[0], w);
        }
    }
    else if ((frame->format == AV_PIX_FMT_YUV422P) || (frame->format == AV_PIX_FMT_YUV422P10LE)) {
        // copy Y plane
        for (y = 0; y < h; y++) {
            offset = pitch * (y + info->CropY) + info->CropX;
            memcpy_s(data->Y + offset, w, frame->data[0] + y * frame->linesize[0], w);
        }

        // copy U plane
        for (y = 0; y < h; y++) {
            offset = pitch / 2 * (y + info->CropY) + info->CropX;
            memcpy_s(data->U + offset, w / 2, frame->data[1] + y * frame->linesize[1], w / 2);
        }

        // copy V plane
        for (y = 0; y < h; y++) {
            offset = pitch / 2 * (y + info->CropY) + info->CropX;
            memcpy_s(data->V + offset, w / 2, frame->data[2] + y * frame->linesize[2], w / 2);
        }
    }
    else {
        // copy Y plane
        for (y = 0; y < h; y++) {
            offset = pitch * (y + info->CropY) + info->CropX;
            memcpy_s(data->Y + offset, w, frame->data[0] + y * frame->linesize[0], w);
        }

        // copy U plane
        for (y = 0; y < h / 2; y++) {
            offset = pitch / 2 * (y + info->CropY) + info->CropX;
            memcpy_s(data->U + offset, w / 2, frame->data[1] + y * frame->linesize[1], w / 2);
        }

        // copy V plane
        for (y = 0; y < h / 2; y++) {
            offset = pitch / 2 * (y + info->CropY) + info->CropX;
            memcpy_s(data->V + offset, w / 2, frame->data[2] + y * frame->linesize[2], w / 2);
        }
    }

    if (frame->pts) {
        surface->Data.TimeStamp = frame->pts;
        surface->Data.DataFlag  = MFX_FRAMEDATA_ORIGINAL_TIMESTAMP;
    }

    return MFX_ERR_NONE;
}

mfxStatus CheckFrameInfoCommon(mfxFrameInfo *info, mfxU32 codecId) {
    RET_IF_FALSE(info, MFX_ERR_NULL_PTR);

    switch (info->FourCC) {
        case MFX_FOURCC_I420:
        case MFX_FOURCC_I010:
        case MFX_FOURCC_I422:
        case MFX_FOURCC_I210:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    switch (info->PicStruct) {
        case MFX_PICSTRUCT_UNKNOWN:
        case MFX_PICSTRUCT_PROGRESSIVE:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    RET_IF_FALSE((!info->BitDepthLuma || (info->BitDepthLuma >= 8)) &&
                     (!info->BitDepthChroma || (info->BitDepthChroma >= 8)),
                 MFX_ERR_INVALID_VIDEO_PARAM);

    if (info->BitDepthLuma > 8 || info->BitDepthChroma > 8) {
        switch (info->FourCC) {
            case MFX_FOURCC_I010:
            case MFX_FOURCC_I210:
                //case MFX_FOURCC_P010: // for later
                break;
            default:
                return MFX_ERR_INVALID_VIDEO_PARAM;
        }
    }

    // for later, shift = 1 for ms10bit in msdk
    //if (info->FourCC == MFX_FOURCC_P010) {
    //    RET_IF_FALSE(info->Shift, MFX_ERR_INVALID_VIDEO_PARAM);
    //}

    RET_IF_FALSE((info->ChromaFormat == MFX_CHROMAFORMAT_YUV420) ||
                     (info->ChromaFormat == MFX_CHROMAFORMAT_YUV422),
                 MFX_ERR_INVALID_VIDEO_PARAM);
    RET_IF_FALSE((info->FrameRateExtN == 0 && info->FrameRateExtD == 0) ||
                     (info->FrameRateExtN != 0 && info->FrameRateExtD != 0),
                 MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);
    RET_IF_FALSE(
        (!info->AspectRatioW && !info->AspectRatioH) || (info->AspectRatioW && info->AspectRatioH),
        MFX_ERR_INVALID_VIDEO_PARAM);

    return MFX_ERR_NONE;
}

mfxStatus CheckFrameInfoCodecs(mfxFrameInfo *info, mfxU32 codecId) {
    mfxStatus sts = CheckFrameInfoCommon(info, codecId);
    RET_ERROR(sts);

    switch (codecId) {
        case MFX_CODEC_JPEG:
        case MFX_CODEC_MPEG2:
            if (info->FourCC != MFX_FOURCC_I420)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        case MFX_CODEC_AVC:
        case MFX_CODEC_HEVC:
        case MFX_CODEC_AV1:
            if (info->FourCC != MFX_FOURCC_I420 && info->FourCC != MFX_FOURCC_I010 &&
                info->FourCC != MFX_FOURCC_I422 && info->FourCC != MFX_FOURCC_I210)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        default:
            RET_IF_FALSE(info->FourCC == MFX_FOURCC_I420, MFX_ERR_INVALID_VIDEO_PARAM);
            break;
    }

    switch (codecId) {
        case MFX_CODEC_JPEG:
        case MFX_CODEC_AVC:
        case MFX_CODEC_HEVC:
        case MFX_CODEC_MPEG2:
        case MFX_CODEC_AV1:
            if (info->ChromaFormat != MFX_CHROMAFORMAT_YUV420 &&
                info->ChromaFormat != MFX_CHROMAFORMAT_YUV422)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        default:
            RET_IF_FALSE(info->ChromaFormat == MFX_CHROMAFORMAT_YUV420 ||
                             info->ChromaFormat == MFX_CHROMAFORMAT_YUV422,
                         MFX_ERR_INVALID_VIDEO_PARAM);
            break;
    }

    switch (codecId) {
        case MFX_CODEC_JPEG:
        case MFX_CODEC_AVC:
        case MFX_CODEC_MPEG2:
        case MFX_CODEC_AV1:
            if (info->Width > 3840 || info->Height > 2160)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        case MFX_CODEC_HEVC:
            if (info->Width > 8192 || info->Height > 4096)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        default:
            break;
    }

    return MFX_ERR_NONE;
}

mfxStatus CheckVideoParamCommon(mfxVideoParam *in) {
    RET_IF_FALSE(in, MFX_ERR_NULL_PTR);

    mfxStatus sts = CheckFrameInfoCodecs(&in->mfx.FrameInfo, in->mfx.CodecId);
    RET_ERROR(sts);

    RET_IF_FALSE(in->Protected == 0, MFX_ERR_INVALID_VIDEO_PARAM);

    switch (in->mfx.CodecId) {
        case MFX_CODEC_AVC:
        case MFX_CODEC_HEVC:
        case MFX_CODEC_JPEG:
        case MFX_CODEC_MPEG2:
        case MFX_CODEC_AV1:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return MFX_ERR_NONE;
}
