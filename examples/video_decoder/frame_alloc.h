#ifndef _FRAME_ALLOC_H
#define _FRAME_ALLOC_H

#include "video_decoder.h"

// callback functions for external frame allocator
// functions do not keep any internal state, are fully re-entrant
mfxStatus sys_alloc(mfxHDL pthis,
                    mfxFrameAllocRequest* request,
                    mfxFrameAllocResponse* response);
mfxStatus sys_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData* ptr);
mfxStatus sys_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData* ptr);
mfxStatus sys_gethdl(mfxHDL pthis, mfxMemId mid, mfxHDL* handle);
mfxStatus sys_free(mfxHDL pthis, mfxFrameAllocResponse* response);

#endif // _FRAME_ALLOC_H
