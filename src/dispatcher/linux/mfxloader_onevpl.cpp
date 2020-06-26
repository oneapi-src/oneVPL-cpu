// Copyright (c) 2019-2020 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "linux/mfxloader_onevpl.h"

mfxLoader MFXLoad() {
    return nullptr;
}

void MFXUnload(mfxLoader loader) {}

mfxConfig MFXCreateConfig(mfxLoader loader) {
    return nullptr;
}

mfxStatus MFXSetConfigFilterProperty(mfxConfig config,
                                     const mfxU8* name,
                                     mfxVariant value) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXEnumImplementations(mfxLoader loader,
                                 mfxU32 i,
                                 mfxImplCapsDeliveryFormat format,
                                 mfxHDL* idesc) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXCreateSession(mfxLoader loader, mfxU32 i, mfxSession* session) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXDispReleaseImplDescription(mfxLoader loader, mfxHDL hdl) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXMemory_GetSurfaceForVPP(mfxSession session,
                                     mfxFrameSurface1** surface) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXMemory_GetSurfaceForEncode(mfxSession session,
                                        mfxFrameSurface1** surface) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXMemory_GetSurfaceForDecode(mfxSession session,
                                        mfxFrameSurface1** surface) {
    return MFX_ERR_UNSUPPORTED;
}
