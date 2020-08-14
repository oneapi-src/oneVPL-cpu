/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_common.h"
#include "src/cpu_frame.h"
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

static void DeleteFrameLock(void *opaque, uint8_t *data) {
    FrameLock *locker = static_cast<FrameLock *>(opaque);
    delete locker;
}

AVFrame *CreateAVFrame(mfxFrameSurface1 *surface,
                       mfxFrameAllocator *allocator) {
    auto locker = std::make_unique<FrameLock>();
    RET_IF_FALSE(locker, nullptr);
    RET_IF_FALSE(locker->Lock(surface, MFX_MAP_READ, allocator) == MFX_ERR_NONE,
                 nullptr);
    mfxFrameData *data = locker->GetData();
    mfxFrameInfo *info = &surface->Info;

    AVFrame *av_frame = av_frame_alloc();
    RET_IF_FALSE(av_frame, nullptr);
    av_frame->format = MFXFourCC2AVPixelFormat(info->FourCC);
    av_frame->width  = info->Width;
    av_frame->height = info->Height;
    if (info->FourCC == MFX_FOURCC_RGB4) {
        av_frame->data[0] = data->B;
    }
    else {
        av_frame->data[0] = data->Y;
        av_frame->data[1] = data->U;
        av_frame->data[2] = data->V;
        av_frame->data[3] = data->A;
    }
    av_frame->linesize[0] = data->Pitch;
    switch (info->FourCC) {
        case MFX_FOURCC_I420:
            av_frame->linesize[1] = data->Pitch / 2;
            av_frame->linesize[2] = data->Pitch / 2;
            break;
        case MFX_FOURCC_NV12:
            av_frame->linesize[1] = data->Pitch;
            break;
        case MFX_FOURCC_YUY2:
        case MFX_FOURCC_RGB4:
            break;
        default:
            RET_IF_FALSE(!"Unsupported format", nullptr);
    }
    // set deleter callback
    uint8_t *opaque = reinterpret_cast<uint8_t *>(locker.get());
    av_frame->buf[0] =
        av_buffer_create(opaque, sizeof(FrameLock), DeleteFrameLock, opaque, 0);
    RET_IF_FALSE(av_frame->buf[0], nullptr);
    locker.release();
    return av_frame;
}

std::shared_ptr<AVFrame> GetAVFrameFromMfxSurface(
    mfxFrameSurface1 *surface,
    mfxFrameAllocator *allocator) {
    // Try get AVFrame
    CpuFrame *cpu_frame = CpuFrame::TryCast(surface);
    if (cpu_frame) {
        AVFrame *av_frame = cpu_frame->GetAVFrame();
        if (av_frame) {
            // Empty deleter because AVFrame owned by CpuFrame
            return std::shared_ptr<AVFrame>(av_frame, [](AVFrame *) {
            });
        }
    }
    // Create new AVFrame with zero-copy reference to mfx data buffer
    AVFrame *av_frame = CreateAVFrame(surface, allocator);
    RET_IF_FALSE(av_frame, nullptr);
    // Call av_frame_unref in deleter
    return std::shared_ptr<AVFrame>(av_frame, [](AVFrame *frame) {
        av_frame_unref(frame);
    });
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
