// Copyright (c) 2007-2020 Intel Corporation
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

// SEE: mdp_msdk-lib\_studio\mfx_lib\shared\src\libmfxsw.cpp

#include "cpu_workstream.h"
#include "mfxvideo.h"

mfxStatus MFXInit(mfxIMPL implParam, mfxVersion *ver, mfxSession *session) {
    mfxInitParam par = {};

    par.Implementation = implParam;
    if (ver) {
        par.Version = *ver;
    }
    else {
        par.Version.Major = MFX_VERSION_MAJOR;
        par.Version.Minor = MFX_VERSION_MINOR;
    }
    par.ExternalThreads = 0;

    return MFXInitEx(par, session);
}

mfxStatus MFXInitEx(mfxInitParam par, mfxSession *session) {
    mfxIMPL impl = par.Implementation & (MFX_IMPL_VIA_ANY - 1);

    // check the library version
    if ((MFX_VERSION_MAJOR != par.Version.Major) ||
        (MFX_VERSION_MINOR < par.Version.Minor)) {
        return MFX_ERR_UNSUPPORTED;
    }

    // SW plugin only
    if ((MFX_IMPL_AUTO != impl) && (MFX_IMPL_AUTO_ANY != impl) &&
        (MFX_IMPL_SOFTWARE_VPL != impl)) {
        return MFX_ERR_UNSUPPORTED;
    }

    // create CPU workstream
    CpuWorkstream *ws = new CpuWorkstream;

    if (!ws) {
        return MFX_ERR_UNSUPPORTED;
    }

    // save the handle
    *session = (mfxSession)(ws);

    return MFX_ERR_NONE;
}

mfxStatus MFXClose(mfxSession session) {
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }

    CpuWorkstream *ws = (CpuWorkstream *)session;

    delete ws;
    ws = nullptr;

    return MFX_ERR_NONE;
}

mfxStatus MFXQueryIMPL(mfxSession session, mfxIMPL *impl) {
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == impl) {
        return MFX_ERR_NULL_PTR;
    }

    *impl = MFX_IMPL_SOFTWARE_VPL;

    return MFX_ERR_NONE;
}

mfxStatus MFXQueryVersion(mfxSession session, mfxVersion *pVersion) {
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == pVersion) {
        return MFX_ERR_NULL_PTR;
    }

    // set the library's version
    pVersion->Major = MFX_VERSION_MAJOR;
    pVersion->Minor = MFX_VERSION_MINOR;

    return MFX_ERR_NONE;
}

// stubs
mfxStatus MFXDoWork(mfxSession session) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXJoinSession(mfxSession session, mfxSession child) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXDisjoinSession(mfxSession session) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXCloneSession(mfxSession session, mfxSession *clone) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXSetPriority(mfxSession session, mfxPriority priority) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXGetPriority(mfxSession session, mfxPriority *priority) {
    return MFX_ERR_UNSUPPORTED;
}

// DLL entry point

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID lpReserved) {
    return TRUE;
} // BOOL APIENTRY DllMain(HMODULE hModule,
#else // #if defined(_WIN32) || defined(_WIN64)
void __attribute__((constructor)) dll_init(void) {}
#endif // #if defined(_WIN32) || defined(_WIN64)
