/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_frames.h"

// callbacks for the new mfxFrameSurface interface
// static functions (stateless) - all required state is
//   carried in surface->FrameInterface->Context

// increase refCount on surface (+1)
mfxStatus FrameSurfaceInterface::AddRef(mfxFrameSurface1* surface) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;
    RET_IF_FALSE(frameInterface && frameInterface->Context,
                 MFX_ERR_INVALID_HANDLE);

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    context->m_refCount = ++context->m_refCount;

    return MFX_ERR_NONE;
}

// decrease refCount on surface (-1)
mfxStatus FrameSurfaceInterface::Release(mfxFrameSurface1* surface) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;
    RET_IF_FALSE(frameInterface && frameInterface->Context,
                 MFX_ERR_INVALID_HANDLE);

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    context->m_refCount = --context->m_refCount;

    return MFX_ERR_NONE;
}

// return current refCount on surface
mfxStatus FrameSurfaceInterface::GetRefCounter(mfxFrameSurface1* surface,
                                               mfxU32* counter) {
    RET_IF_FALSE(surface && counter, MFX_ERR_NULL_PTR);

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;
    RET_IF_FALSE(frameInterface && frameInterface->Context,
                 MFX_ERR_INVALID_HANDLE);

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    *counter = context->m_refCount;

    return MFX_ERR_NONE;
}

// map surface to system memory according to "flags"
// currently does nothing for system memory
mfxStatus FrameSurfaceInterface::Map(mfxFrameSurface1* surface, mfxU32 flags) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;
    RET_IF_FALSE(frameInterface && frameInterface->Context,
                 MFX_ERR_INVALID_HANDLE);

    return MFX_ERR_NONE;
}

// unmap surface - no longer accessible to application
// currently does nothing for system memory
mfxStatus FrameSurfaceInterface::Unmap(mfxFrameSurface1* surface) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;
    RET_IF_FALSE(frameInterface && frameInterface->Context,
                 MFX_ERR_INVALID_HANDLE);

    return MFX_ERR_NONE;
}

// return native handle and type
mfxStatus FrameSurfaceInterface::GetNativeHandle(
    mfxFrameSurface1* surface,
    mfxHDL* resource,
    mfxResourceType* resource_type) {
    if (surface == 0 || resource == 0 || resource_type == 0)
        return MFX_ERR_NULL_PTR;

    return MFX_ERR_NOT_FOUND;
}

// return device handle and type
// not relevant for system memory
mfxStatus FrameSurfaceInterface::GetDeviceHandle(mfxFrameSurface1* surface,
                                                 mfxHDL* device_handle,
                                                 mfxHandleType* device_type) {
    if (surface == 0 || device_handle == 0 || device_type == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;
    RET_IF_FALSE(frameInterface && frameInterface->Context,
                 MFX_ERR_INVALID_HANDLE);

    return MFX_ERR_NOT_FOUND;
}

// synchronize on surface after calling DecodeFrameAsync or VPP
// alternative to calling MFXCore_SyncOperation
mfxStatus FrameSurfaceInterface::Synchronize(mfxFrameSurface1* surface,
                                             mfxU32 wait) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;
    RET_IF_FALSE(frameInterface && frameInterface->Context,
                 MFX_ERR_INVALID_HANDLE);

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    // TO DO - VPL CPU ref is currently synchronous (Sync does nothing)
    // need to align sync behavior between legacy SyncOperation()
    //   and new sync on surfaces
    // probably need to track mapping of unique sync points to associated
    //   mfxFrameSurface1 surfaces, and sync here

    return MFX_ERR_NONE;
}

// allocated new mfxFrameSurfaceInterface and associated context
// set pointers to callback functions
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

// free mfxFrameSurfaceInterface and associated context allocated with
//   AllocFrameSurfaceInterface
void FrameSurfaceInterface::FreeFrameSurfaceInterface(
    mfxFrameSurfaceInterface* frameInterface) {
    if (!frameInterface || !frameInterface->Context)
        return;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    delete context;
    delete frameInterface;
}