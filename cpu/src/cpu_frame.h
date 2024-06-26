/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef CPU_SRC_CPU_FRAME_H_
#define CPU_SRC_CPU_FRAME_H_

#include "src/cpu_common.h"

// interface for MFX_GUID_SURFACE_POOL
struct CpuFramePoolInterface {
public:
    CpuFramePoolInterface() : m_surfacePoolInterface(), m_parentPool(), m_refCount(0) {
        m_surfacePoolInterface.Context             = (mfxHDL *)this;
        m_surfacePoolInterface.AddRef              = AddRef;
        m_surfacePoolInterface.Release             = Release;
        m_surfacePoolInterface.GetRefCounter       = GetRefCounter;
        m_surfacePoolInterface.SetNumSurfaces      = SetNumSurfaces;
        m_surfacePoolInterface.RevokeSurfaces      = RevokeSurfaces;
        m_surfacePoolInterface.GetAllocationPolicy = GetAllocationPolicy;
        m_surfacePoolInterface.GetMaximumPoolSize  = GetMaximumPoolSize;
        m_surfacePoolInterface.GetCurrentPoolSize  = GetCurrentPoolSize;
    }

    ~CpuFramePoolInterface() {}

    void SetParentPool(mfxHDL parentPool) {
        m_parentPool = parentPool;
    }

    mfxHDL GetParentPool() {
        return m_parentPool;
    }

    mfxSurfacePoolInterface m_surfacePoolInterface;

private:
    mfxHDL m_parentPool; // pointer to CpuFramePool (opaque handle to avoid circular dependency)
    std::atomic<mfxU32> m_refCount;

    static mfxStatus AddRef(struct mfxSurfacePoolInterface *pool);
    static mfxStatus Release(struct mfxSurfacePoolInterface *pool);
    static mfxStatus GetRefCounter(struct mfxSurfacePoolInterface *pool, mfxU32 *counter);
    static mfxStatus SetNumSurfaces(struct mfxSurfacePoolInterface *pool, mfxU32 num_surfaces);
    static mfxStatus RevokeSurfaces(struct mfxSurfacePoolInterface *pool, mfxU32 num_surfaces);
    static mfxStatus GetAllocationPolicy(struct mfxSurfacePoolInterface *pool,
                                         mfxPoolAllocationPolicy *policy);
    static mfxStatus GetMaximumPoolSize(struct mfxSurfacePoolInterface *pool, mfxU32 *size);
    static mfxStatus GetCurrentPoolSize(struct mfxSurfacePoolInterface *pool, mfxU32 *size);
};

// Implemented via AVFrame
class CpuFrame : public mfxFrameSurface1 {
public:
    explicit CpuFrame(CpuFramePoolInterface *parentPoolInterface)
            : m_refCount(0),
              m_mappedFlags(0),
              m_interface(),
              m_parentPoolInterface(parentPoolInterface) {
        m_avframe = av_frame_alloc();

        *(mfxFrameSurface1 *)this   = {};
        Version.Version             = MFX_FRAMESURFACE1_VERSION;
        FrameInterface              = &m_interface;
        m_interface.Context         = (mfxHDL *)this;
        m_interface.Version.Version = MFX_FRAMESURFACEINTERFACE_VERSION;
        m_interface.AddRef          = AddRef;
        m_interface.Release         = Release;
        m_interface.GetRefCounter   = GetRefCounter;
        m_interface.Map             = Map;
        m_interface.Unmap           = Unmap;
        m_interface.GetNativeHandle = GetNativeHandle;
        m_interface.GetDeviceHandle = GetDeviceHandle;
        m_interface.Synchronize     = Synchronize;
        m_interface.OnComplete      = OnComplete;
        m_interface.QueryInterface  = QueryInterface;
    }

    ~CpuFrame() {
        if (m_avframe) {
            av_frame_free(&m_avframe);
        }
    }

    static CpuFrame *TryCast(mfxFrameSurface1 *surface) {
        if (surface && surface->FrameInterface && surface->FrameInterface->Context &&
            surface->Version.Version >= MFX_FRAMESURFACE1_VERSION &&
            surface->FrameInterface->Map == Map) {
            return (CpuFrame *)surface->FrameInterface->Context;
        }
        else {
            return nullptr;
        }
    }

    AVFrame *GetAVFrame() {
        return m_avframe;
    }

    mfxStatus Allocate(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
        m_avframe->width  = width;
        m_avframe->height = height;
        m_avframe->format = MFXFourCC2AVPixelFormat(FourCC);
        RET_IF_FALSE(m_avframe->format != AV_PIX_FMT_NONE, MFX_ERR_INVALID_VIDEO_PARAM);
        RET_IF_FALSE(av_frame_get_buffer(m_avframe, 1) == 0, MFX_ERR_MEMORY_ALLOC);
        return Update();
    }

    mfxStatus ImportAVFrame(AVFrame *avframe) {
        RET_IF_FALSE(avframe, MFX_ERR_NULL_PTR);
        RET_IF_FALSE(m_avframe == nullptr || avframe == m_avframe, MFX_ERR_UNDEFINED_BEHAVIOR);
        m_avframe   = avframe;
        Info.Width  = avframe->width;
        Info.Height = avframe->height;
        Info.FourCC = AVPixelFormat2MFXFourCC(avframe->format);
        switch (avframe->format) {
            case AV_PIX_FMT_YUV420P10LE:
                Info.BitDepthLuma   = 10;
                Info.BitDepthChroma = 10;
                Info.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
                break;
            case AV_PIX_FMT_YUV422P10LE:
                Info.BitDepthLuma   = 10;
                Info.BitDepthChroma = 10;
                Info.ChromaFormat   = MFX_CHROMAFORMAT_YUV422;
                break;
            case AV_PIX_FMT_YUV420P:
            case AV_PIX_FMT_YUVJ420P:
                Info.BitDepthLuma   = 8;
                Info.BitDepthChroma = 8;
                Info.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
                break;
            case AV_PIX_FMT_YUV422P:
                Info.BitDepthLuma   = 8;
                Info.BitDepthChroma = 8;
                Info.ChromaFormat   = MFX_CHROMAFORMAT_YUV422;
                break;
            case AV_PIX_FMT_BGRA:
                Info.BitDepthLuma   = 8;
                Info.BitDepthChroma = 8;
                Info.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
                break;
            default:
                Info.BitDepthLuma   = 0;
                Info.BitDepthChroma = 0;
        }
        Info.CropW     = avframe->width;
        Info.CropH     = avframe->height;
        Info.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;

        if (avframe->sample_aspect_ratio.num == 0 && avframe->sample_aspect_ratio.den == 1) {
            Info.AspectRatioW = 1;
            Info.AspectRatioH = 1;
        }
        else {
            Info.AspectRatioW = (uint16_t)avframe->sample_aspect_ratio.num;
            Info.AspectRatioH = (uint16_t)avframe->sample_aspect_ratio.den;
        }

        if (Info.FourCC == MFX_FOURCC_RGB4) {
            Data.B = avframe->data[0] + 0;
            Data.G = avframe->data[0] + 1;
            Data.R = avframe->data[0] + 2;
            Data.A = avframe->data[0] + 3;
        }
        else {
            Data.Y = avframe->data[0];
            Data.U = avframe->data[1];
            Data.V = avframe->data[2];
            Data.A = avframe->data[3];
        }
        Data.Pitch     = avframe->linesize[0];
        Data.TimeStamp = avframe->pts; // TODO(check units)
        // TODO(fill more fields)
        return MFX_ERR_NONE;
    }

    mfxStatus Update() {
        return ImportAVFrame(m_avframe);
    }

private:
    std::atomic<mfxU32> m_refCount; // TODO(we have C++11, correct?)
    mfxU32 m_mappedFlags;
    AVFrame *m_avframe;
    mfxFrameSurfaceInterface m_interface;
    CpuFramePoolInterface *m_parentPoolInterface;

    static mfxStatus AddRef(mfxFrameSurface1 *surface);
    static mfxStatus Release(mfxFrameSurface1 *surface);
    static mfxStatus GetRefCounter(mfxFrameSurface1 *surface, mfxU32 *counter);
    static mfxStatus Map(mfxFrameSurface1 *surface, mfxU32 flags);
    static mfxStatus Unmap(mfxFrameSurface1 *surface);
    static mfxStatus GetNativeHandle(mfxFrameSurface1 *surface,
                                     mfxHDL *resource,
                                     mfxResourceType *resource_type);
    static mfxStatus GetDeviceHandle(mfxFrameSurface1 *surface,
                                     mfxHDL *device_handle,
                                     mfxHandleType *device_type);
    static mfxStatus Synchronize(mfxFrameSurface1 *surface, mfxU32 wait);
    static void OnComplete(mfxStatus sts);
    static mfxStatus QueryInterface(mfxFrameSurface1 *surface, mfxGUID guid, mfxHDL *interface);
};

#endif // CPU_SRC_CPU_FRAME_H_
