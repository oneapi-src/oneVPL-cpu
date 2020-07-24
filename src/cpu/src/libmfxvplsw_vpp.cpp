/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"
#include "vpl/mfxvideo.h"

// stubs
mfxStatus MFXVideoVPP_Query(mfxSession session,
                            mfxVideoParam *in,
                            mfxVideoParam *out) {
    if (session == 0)
        return MFX_ERR_INVALID_HANDLE;

    if (out == 0)
        return MFX_ERR_NULL_PTR;

    // do not support protected mode yet
    if ((in != 0 && in->Protected != 0) || out->Protected != 0)
        return MFX_ERR_UNSUPPORTED;

    mfxStatus sts = MFX_ERR_NONE;

    CpuWorkstream *ws = (CpuWorkstream *)session;

    sts = ws->VPPQuery(in, out);

    return sts;
}

mfxStatus MFXVideoVPP_QueryIOSurf(mfxSession session,
                                  mfxVideoParam *par,
                                  mfxFrameAllocRequest request[2]) {
    if (session == 0)
        return MFX_ERR_INVALID_HANDLE;

    if (par == 0)
        return MFX_ERR_NULL_PTR;

    if (request == 0)
        return MFX_ERR_NULL_PTR;

    mfxStatus sts = MFX_ERR_NONE;

    CpuWorkstream *ws = (CpuWorkstream *)session;

    sts = ws->VPPQueryIOSurf(par, request);

    return sts;
}

mfxStatus MFXVideoVPP_Init(mfxSession session, mfxVideoParam *par) {
    if (session == 0)
        return MFX_ERR_INVALID_HANDLE;

    if (par == 0)
        return MFX_ERR_NULL_PTR;

    mfxStatus sts = MFX_ERR_NONE;

    CpuWorkstream *ws = (CpuWorkstream *)session;

    sts = ws->InitVPP(par);

    return sts;
}

mfxStatus MFXVideoVPP_Close(mfxSession session) {
    if (0 == session)
        return MFX_ERR_INVALID_HANDLE;

    CpuWorkstream *ws = (CpuWorkstream *)session;

    ws->FreeVPP();

    return MFX_ERR_NONE;
}

mfxStatus MFXVideoVPP_GetVideoParam(mfxSession session, mfxVideoParam *par) {
    mfxStatus sts = MFX_ERR_NONE;

    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == par) {
        return MFX_ERR_NULL_PTR;
    }

    return sts;
}

mfxStatus MFXVideoVPP_RunFrameVPPAsync(mfxSession session,
                                       mfxFrameSurface1 *in,
                                       mfxFrameSurface1 *out,
                                       mfxExtVppAuxData *aux,
                                       mfxSyncPoint *syncp) {
    mfxStatus sts = MFX_ERR_NONE;

    if (session == 0)
        return MFX_ERR_INVALID_HANDLE;

    if (in == 0 || out == 0 || syncp == 0)
        return MFX_ERR_NULL_PTR;

    CpuWorkstream *ws = (CpuWorkstream *)session;

    sts = ws->ProcessFrame(in, out, aux);

    *syncp = (mfxSyncPoint)(0x12345678);

    return sts;
}

mfxStatus MFXVideoVPP_Reset(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXVideoVPP_GetVPPStat(mfxSession session, mfxVPPStat *stat) {
    return MFX_ERR_UNSUPPORTED;
}