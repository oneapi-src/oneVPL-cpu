/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"

mfxStatus FrameSurfaceInterface::AddRef(mfxFrameSurface1* surface) {
    if (surface == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    // TO DO - needs to be atomic
    context->m_refCount++;

    return MFX_ERR_NONE;
}

mfxStatus FrameSurfaceInterface::Release(mfxFrameSurface1* surface) {
    if (surface == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    // TO DO - needs to be atomic
    context->m_refCount--;

    return MFX_ERR_NONE;
}

mfxStatus FrameSurfaceInterface::GetRefCounter(mfxFrameSurface1* surface,
                                               mfxU32* counter) {
    if (surface == 0 || counter == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    // TO DO - needs to be atomic
    *counter = context->m_refCount;

    return MFX_ERR_NONE;
}

mfxStatus FrameSurfaceInterface::Map(mfxFrameSurface1* surface, mfxU32 flags) {
    if (surface == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    return MFX_ERR_NONE;
}

mfxStatus FrameSurfaceInterface::Unmap(mfxFrameSurface1* surface) {
    if (surface == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    return MFX_ERR_NONE;
}

mfxStatus FrameSurfaceInterface::GetNativeHandle(
    mfxFrameSurface1* surface,
    mfxHDL* resource,
    mfxResourceType* resource_type) {
    if (surface == 0 || resource == 0 || resource_type == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    return MFX_ERR_NONE;
}

mfxStatus FrameSurfaceInterface::GetDeviceHandle(mfxFrameSurface1* surface,
                                                 mfxHDL* device_handle,
                                                 mfxHandleType* device_type) {
    if (surface == 0 || device_handle == 0 || device_type == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    *device_handle = (mfxHDL)0;
    *device_type   = (mfxHandleType)0;

    return MFX_ERR_NONE;
}

mfxStatus FrameSurfaceInterface::Synchronize(mfxFrameSurface1* surface,
                                             mfxU32 wait) {
    if (surface == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    return MFX_ERR_NONE;
}

mfxFrameSurfaceInterface* FrameSurfaceInterface::AllocFrameSurfaceInterface() {
    mfxFrameSurfaceInterface* frameInterface = new mfxFrameSurfaceInterface;
    if (!frameInterface)
        return nullptr;

    *frameInterface = { 0 };

    frameInterface->Context = (mfxHDL*)new FrameInterfaceContext;
    if (!frameInterface->Context)
        return nullptr;

    frameInterface->Version.Version = MFX_FRAMESURFACEINTERFACE_VERSION;

    frameInterface->AddRef          = FrameSurfaceInterface::AddRef;
    frameInterface->Release         = FrameSurfaceInterface::Release;
    frameInterface->GetRefCounter   = FrameSurfaceInterface::GetRefCounter;
    frameInterface->Map             = FrameSurfaceInterface::Map;
    frameInterface->Unmap           = FrameSurfaceInterface::Unmap;
    frameInterface->GetNativeHandle = FrameSurfaceInterface::GetNativeHandle;
    frameInterface->GetDeviceHandle = FrameSurfaceInterface::GetDeviceHandle;
    frameInterface->Synchronize     = FrameSurfaceInterface::Synchronize;

    return frameInterface;
}

void FrameSurfaceInterface::FreeFrameSurfaceInterface(
    mfxFrameSurfaceInterface* frameInterface) {
    if (!frameInterface || !frameInterface->Context)
        return;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    delete context;
    delete frameInterface;
}