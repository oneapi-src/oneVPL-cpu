/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"

// minimal set of atmoic operations for refCount ++ and --
#if defined(_WIN32) || defined(_WIN64)

    #include <windows.h>

mfxU32 AtomicInc32(volatile mfxU32* count) {
    return InterlockedIncrement((LPLONG)count);
}

mfxU32 AtomicDec32(volatile mfxU32* count) {
    return InterlockedDecrement((LPLONG)count);
}
#else
// Linux
mfxU32 AtomicInc32(volatile mfxU32* count) {
    return (mfxU32)__sync_add_and_fetch(count, (mfxU32)1);
}

mfxU32 AtomicDec32(volatile mfxU32* count) {
    return (mfxU32)__sync_add_and_fetch(count, (mfxU32)-1);
}
#endif

// callbacks for the new mfxFrameSurface interface
// static functions (stateless) - all required state is
//   carried in surface->FrameInterface->Context

// increase refCount on surface (+1)
mfxStatus FrameSurfaceInterface::AddRef(mfxFrameSurface1* surface) {
    if (surface == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    context->m_refCount = AtomicInc32(&(context->m_refCount));

    return MFX_ERR_NONE;
}

// decrease refCount on surface (-1)
mfxStatus FrameSurfaceInterface::Release(mfxFrameSurface1* surface) {
    if (surface == 0)
        return MFX_ERR_NULL_PTR;

    mfxFrameSurfaceInterface* frameInterface =
        (mfxFrameSurfaceInterface*)surface->FrameInterface;

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    context->m_refCount = AtomicDec32(&(context->m_refCount));

    return MFX_ERR_NONE;
}

// return current refCount on surface
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

    *counter = context->m_refCount;

    return MFX_ERR_NONE;
}

// map surface to system memory according to "flags"
// currently does nothing for system memory
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

// unmap surface - no longer accessible to application
// currently does nothing for system memory
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

// return native handle and type
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

    // TO DO - what does resource mean in system memory?
    *resource      = (mfxHDL*)(frameInterface->Context);
    *resource_type = MFX_RESOURCE_SYSTEM_SURFACE;

    return MFX_ERR_NONE;
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

    if (!frameInterface || !frameInterface->Context)
        return MFX_ERR_INVALID_HANDLE;

    FrameInterfaceContext* context =
        (FrameInterfaceContext*)frameInterface->Context;

    *device_handle = (mfxHDL)0;
    *device_type   = (mfxHandleType)0;

    return MFX_ERR_NONE;
}

// synchronize on surface after calling DecodeFrameAsync or VPP
// alternative to calling MFXCore_SyncOperation
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