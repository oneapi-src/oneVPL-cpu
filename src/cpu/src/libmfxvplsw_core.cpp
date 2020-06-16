/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"
#include "onevpl/mfxplugin.h"
#include "onevpl/mfxvideo.h"

// stubs
mfxStatus MFXVideoCORE_SetBufferAllocator(mfxSession session,
                                          mfxBufferAllocator *allocator) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoCORE_SetFrameAllocator(mfxSession session,
                                         mfxFrameAllocator *allocator) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoCORE_SetHandle(mfxSession session,
                                 mfxHandleType type,
                                 mfxHDL hdl) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoCORE_GetHandle(mfxSession session,
                                 mfxHandleType type,
                                 mfxHDL *hdl) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoCORE_QueryPlatform(mfxSession session,
                                     mfxPlatform *platform) {
    return MFX_ERR_UNSUPPORTED;
}

// USER plugins will not be supported in this implementation, but
// we implement stub functions for dispatcher compatibility

mfxStatus MFXVideoUSER_GetPlugin(mfxSession session,
                                 mfxU32 type,
                                 mfxPlugin *par) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXVideoUSER_Register(mfxSession session,
                                mfxU32 type,
                                const mfxPlugin *par) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXVideoUSER_Unregister(mfxSession session, mfxU32 type) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXVideoUSER_ProcessFrameAsync(mfxSession session,
                                         const mfxHDL *in,
                                         mfxU32 in_num,
                                         const mfxHDL *out,
                                         mfxU32 out_num,
                                         mfxSyncPoint *syncp) {
    return MFX_ERR_UNSUPPORTED;
}
