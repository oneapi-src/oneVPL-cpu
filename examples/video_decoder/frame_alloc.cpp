#include "frame_alloc.h"

typedef struct {
    mfxU16 width;
    mfxU16 height;
    mfxU8 *base;
} mmidInfo;

// external frame allocator - supports: NV12, system memory
mfxStatus sys_alloc(mfxHDL pthis,
                    mfxFrameAllocRequest *request,
                    mfxFrameAllocResponse *response) {
    mfxU32 surfIdx, surfBytes;
    mfxU16 width, height;
    mfxU8 *surfBuf;
    mmidInfo *mmidPool;

    if (!(request->Type & MFX_MEMTYPE_SYSTEM_MEMORY) ||
        request->Info.FourCC != MFX_FOURCC_NV12)
        return MFX_ERR_UNSUPPORTED;

    // allocate number of frames suggested by MSDK
    response->NumFrameActual = request->NumFrameSuggested;
    width                    = (mfxU16)ALIGN_N(request->Info.Width, 32);
    height                   = (mfxU16)ALIGN_N(request->Info.Height, 32);
    surfBytes                = width * height * 3 / 2;

    // allocate contiguous chunk of memory large enough for all surfaces
    surfBuf        = new mfxU8[surfBytes * response->NumFrameActual];
    mmidPool       = new mmidInfo[response->NumFrameActual];
    response->mids = new mfxMemId[request->NumFrameSuggested];

    // init mmid pool with pointer to memory for each frame
    for (surfIdx = 0; surfIdx < response->NumFrameActual; surfIdx++) {
        mmidPool[surfIdx].width  = width;
        mmidPool[surfIdx].height = height;
        mmidPool[surfIdx].base   = surfBuf + (surfIdx * surfBytes);

        // save pointer to mmidInfo struct associated with this surface
        response->mids[surfIdx] = (mmidPool + surfIdx);
    }

    return MFX_ERR_NONE;
}

// NOTE - mid is actually a _pointer_ to the mid stored in sys_alloc (i.e. pointer to a pointer)
// the documentation in mediasdk-man is confusing on this - source code in Example 12 is wrong as written
mfxStatus sys_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr) {
    mmidInfo *mmid = (mmidInfo *)(*(mfxMemId *)mid);

    if (!mmid || !ptr) {
        return MFX_ERR_NULL_PTR;
    }

    if (ptr->Locked) {
        return MFX_ERR_LOCK_MEMORY;
    }

    ptr->Pitch = mmid->width;
    ptr->Y     = mmid->base;
    ptr->U     = ptr->Y + mmid->width * mmid->height;
    ptr->V     = ptr->U + 1;

    return MFX_ERR_NONE;
}

// unlock frame
mfxStatus sys_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr) {
    mmidInfo *mmid = (mmidInfo *)(*(mfxMemId *)mid);

    if (!mmid || !ptr) {
        return MFX_ERR_NULL_PTR;
    }

    ptr->Y = 0;
    ptr->U = 0;
    ptr->V = 0;

    return MFX_ERR_NONE;
}

// device-specific handle not necessary for system memory
mfxStatus sys_gethdl(mfxHDL pthis, mfxMemId mid, mfxHDL *handle) {
    mmidInfo *mmid = (mmidInfo *)(*(mfxMemId *)mid);

    return MFX_ERR_UNSUPPORTED;
}

// free resources
mfxStatus sys_free(mfxHDL pthis, mfxFrameAllocResponse *response) {
    // mmid pool and surface memory (base) were allocated as single large chunks
    //   so deleting the arrays from mids[0] is sufficient
    mmidInfo *mmid = (mmidInfo *)response->mids[0];
    delete[] mmid->base;
    delete[] mmid;
    delete[] response->mids;

    return MFX_ERR_NONE;
}
