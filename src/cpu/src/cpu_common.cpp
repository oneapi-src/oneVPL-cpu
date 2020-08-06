/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_common.h"
#include "src/frame_lock.h"

AVPixelFormat MFXFourCC2AVPixelFormat(uint32_t fourcc) {
    return (fourcc == MFX_FOURCC_I010)
               ? AV_PIX_FMT_YUV420P10LE
               : (fourcc == MFX_FOURCC_P010)
                     ? AV_PIX_FMT_P010LE
                     : (fourcc == MFX_FOURCC_NV12)
                           ? AV_PIX_FMT_NV12
                           : (fourcc == MFX_FOURCC_RGB4)
                                 ? AV_PIX_FMT_BGRA
                                 : (fourcc == MFX_FOURCC_I420)
                                       ? AV_PIX_FMT_YUV420P
                                       : (AVPixelFormat)-1;
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

mfxStatus AVFrame2mfxFrameSurface(mfxFrameSurface1 *surface,
                                  AVFrame *frame,
                                  mfxFrameAllocator *allocator) {
    FrameLock locker;
    RET_ERROR(locker.Lock(surface, MFX_MAP_WRITE, allocator));
    mfxFrameData *data = locker.GetData();
    mfxFrameInfo *info = &surface->Info;

    mfxU32 w, h, y, pitch, offset;

    info->Width  = frame->width;
    info->Height = frame->height;
    info->CropX  = 0;
    info->CropY  = 0;
    info->CropW  = frame->width;
    info->CropH  = frame->height;

    if (frame->format == AV_PIX_FMT_YUV420P10LE) {
        info->FourCC = MFX_FOURCC_I010;
        data->Pitch  = (frame->width * 2);

        w     = info->Width * 2;
        h     = info->Height;
        pitch = data->Pitch;
    }
    else if (frame->format == AV_PIX_FMT_YUV420P) {
        info->FourCC = MFX_FOURCC_I420;
        data->Pitch  = frame->width;

        w     = info->Width;
        h     = info->Height;
        pitch = data->Pitch;
    }
    else if (frame->format == AV_PIX_FMT_BGRA) {
        info->FourCC = MFX_FOURCC_RGB4;
        data->Pitch  = (frame->width * 4);

        w     = info->Width * 4;
        h     = info->Height;
        pitch = data->Pitch;
    }
    else { // default
        info->FourCC = MFX_FOURCC_I420;
        data->Pitch  = frame->width;

        w     = info->Width;
        h     = info->Height;
        pitch = data->Pitch;
    }

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
