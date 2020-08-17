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

    if (frame->format == AV_PIX_FMT_YUV420P10LE) {
        RET_IF_FALSE(info->FourCC == MFX_FOURCC_I010,
                     MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

        w = info->Width * 2;
        h = info->Height;
    }
    else if (frame->format == AV_PIX_FMT_YUV420P ||
             frame->format == AV_PIX_FMT_YUVJ420P) {
        RET_IF_FALSE(info->FourCC == MFX_FOURCC_I420,
                     MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

        w = info->Width;
        h = info->Height;
    }
    else if (frame->format == AV_PIX_FMT_BGRA) {
        RET_IF_FALSE(info->FourCC == MFX_FOURCC_RGB4,
                     MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

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
            memcpy_s(data->B + offset,
                     w,
                     frame->data[0] + y * frame->linesize[0],
                     w);
        }
    }
    else {
        // copy Y plane
        for (y = 0; y < h; y++) {
            offset = pitch * (y + info->CropY) + info->CropX;
            memcpy_s(data->Y + offset,
                     w,
                     frame->data[0] + y * frame->linesize[0],
                     w);
        }

        // copy U plane
        for (y = 0; y < h / 2; y++) {
            offset = pitch / 2 * (y + info->CropY) + info->CropX;
            memcpy_s(data->U + offset,
                     w / 2,
                     frame->data[1] + y * frame->linesize[1],
                     w / 2);
        }

        // copy V plane
        for (y = 0; y < h / 2; y++) {
            offset = pitch / 2 * (y + info->CropY) + info->CropX;
            memcpy_s(data->V + offset,
                     w / 2,
                     frame->data[2] + y * frame->linesize[2],
                     w / 2);
        }
    }

    return MFX_ERR_NONE;
}
