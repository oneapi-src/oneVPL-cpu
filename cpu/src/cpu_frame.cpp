/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_frame.h"

// increase refCount on surface (+1)
mfxStatus CpuFrame::AddRef(mfxFrameSurface1 *surface) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    // if TryCast fails (e.g. invalid Context) then return INVALID_HANDLE
    CpuFrame *cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    cpu_frame->m_refCount++;

    return MFX_ERR_NONE;
}

// decrease refCount on surface (-1)
mfxStatus CpuFrame::Release(mfxFrameSurface1 *surface) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    CpuFrame *cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    if (cpu_frame->m_refCount == 0)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    cpu_frame->m_refCount--;

    return MFX_ERR_NONE;
}

// return current refCount on surface
mfxStatus CpuFrame::GetRefCounter(mfxFrameSurface1 *surface, mfxU32 *counter) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(counter, MFX_ERR_NULL_PTR);

    // if TryCast fails (e.g. invalid Context) then return INVALID_HANDLE
    CpuFrame *cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    *counter = cpu_frame->m_refCount;

    // Add 1 to ref counter if m_avframe is locked (ref_count > 1)
    if (cpu_frame->m_avframe && cpu_frame->m_avframe->data[0] &&
        !av_frame_is_writable(cpu_frame->m_avframe)) {
        (*counter)++;
    }

    return MFX_ERR_NONE;
}

// map surface to system memory according to "flags"
// currently does nothing for system memory
mfxStatus CpuFrame::Map(mfxFrameSurface1 *surface, mfxU32 flags) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    CpuFrame *cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    mfxU32 validFlags = MFX_MAP_READ | MFX_MAP_WRITE | MFX_MAP_NOWAIT;
    if ((flags | validFlags) != validFlags)
        return MFX_ERR_UNSUPPORTED;

    if ((flags & MFX_MAP_WRITE) && (cpu_frame->Data.Locked != 0))
        return MFX_ERR_LOCK_MEMORY;

    // save mapping flags
    cpu_frame->m_mappedFlags = flags;

    return MFX_ERR_NONE;
}

// unmap surface - no longer accessible to application
// currently does nothing for system memory
mfxStatus CpuFrame::Unmap(mfxFrameSurface1 *surface) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    CpuFrame *cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    // surface was already unmapped
    if (!cpu_frame->m_mappedFlags)
        return MFX_ERR_UNSUPPORTED;

    // clear mapping flags
    cpu_frame->m_mappedFlags = 0;

    return MFX_ERR_NONE;
}

// return native handle and type
mfxStatus CpuFrame::GetNativeHandle(mfxFrameSurface1 *surface,
                                    mfxHDL *resource,
                                    mfxResourceType *resource_type) {
    RET_IF_FALSE(surface && resource && resource_type, MFX_ERR_NULL_PTR);

    CpuFrame *cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    *resource      = nullptr;
    *resource_type = MFX_RESOURCE_SYSTEM_SURFACE;

    return MFX_ERR_UNSUPPORTED;
}

// return device handle and type
// not relevant for system memory
mfxStatus CpuFrame::GetDeviceHandle(mfxFrameSurface1 *surface,
                                    mfxHDL *device_handle,
                                    mfxHandleType *device_type) {
    RET_IF_FALSE(surface && device_handle && device_type, MFX_ERR_NULL_PTR);

    CpuFrame *cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    *device_handle = nullptr;
    *device_type   = (mfxHandleType)0;

    return MFX_ERR_UNSUPPORTED;
}

// synchronize on surface after calling DecodeFrameAsync or VPP
// alternative to calling MFXCore_SyncOperation
mfxStatus CpuFrame::Synchronize(mfxFrameSurface1 *surface, mfxU32 wait) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    CpuFrame *cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    // TO DO - VPL CPU ref is currently synchronous (Sync does nothing)
    // need to align sync behavior between legacy SyncOperation()
    //   and new sync on surfaces
    // probably need to track mapping of unique sync points to associated
    //   mfxFrameSurface1 surfaces, and sync here

    return MFX_ERR_NONE;
}

void CpuFrame::OnComplete(mfxStatus sts) {
    return;
}

mfxStatus CpuFrame::QueryInterface(mfxFrameSurface1 *surface, mfxGUID guid, mfxHDL *interface) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(interface, MFX_ERR_NULL_PTR);

    return MFX_ERR_NOT_IMPLEMENTED;
}
