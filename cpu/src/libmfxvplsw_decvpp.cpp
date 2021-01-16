/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"
#include "vpl/mfxvideo.h"

mfxStatus MFXVideoDECODE_VPP_Init(mfxSession session,
                                  mfxVideoParam *decode_par,
                                  mfxVideoChannelParam **vpp_par_array,
                                  mfxU32 num_vpp_par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(decode_par, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(vpp_par_array, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(num_vpp_par, MFX_ERR_INVALID_VIDEO_PARAM)

    // init decoder
    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    if (ws->GetDecodeVPP())
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    std::unique_ptr<CpuDecodeVPP> decvpp(new CpuDecodeVPP(ws));
    RET_IF_FALSE(decvpp, MFX_ERR_MEMORY_ALLOC);
    mfxStatus sts = decvpp->InitDecodeVPP(decode_par, vpp_par_array, num_vpp_par);

    if (sts < MFX_ERR_NONE)
        return sts;
    else
        ws->SetDecodeVPP(decvpp.release());

    return sts;
}

mfxStatus MFXVideoDECODE_VPP_DecodeFrameAsync(mfxSession session,
                                              mfxBitstream *bs,
                                              mfxU32 *skip_channels,
                                              mfxU32 num_skip_channels,
                                              mfxSurfaceArray **surf_array_out) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);

    CpuWorkstream *ws    = reinterpret_cast<CpuWorkstream *>(session);
    CpuDecodeVPP *decvpp = ws->GetDecodeVPP();
    RET_IF_FALSE(decvpp, MFX_ERR_NOT_INITIALIZED);

    return decvpp->DecodeVPPFrame(bs, skip_channels, num_skip_channels, surf_array_out);
}

mfxStatus MFXVideoDECODE_VPP_Reset(mfxSession session,
                                   mfxVideoParam *decode_par,
                                   mfxVideoChannelParam **vpp_par_array,
                                   mfxU32 num_vpp_par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(decode_par, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(vpp_par_array, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(num_vpp_par, MFX_ERR_INVALID_VIDEO_PARAM)

    CpuWorkstream *ws    = reinterpret_cast<CpuWorkstream *>(session);
    CpuDecodeVPP *decvpp = ws->GetDecodeVPP();
    RET_IF_FALSE(decvpp, MFX_ERR_NOT_INITIALIZED);

    return decvpp->Reset(decode_par, vpp_par_array, num_vpp_par);
}

mfxStatus MFXVideoDECODE_VPP_GetChannelParam(mfxSession session,
                                             mfxVideoChannelParam *par,
                                             mfxU32 channel_id) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(channel_id, MFX_ERR_NOT_FOUND)

    CpuWorkstream *ws    = reinterpret_cast<CpuWorkstream *>(session);
    CpuDecodeVPP *decvpp = ws->GetDecodeVPP();
    RET_IF_FALSE(decvpp, MFX_ERR_NOT_INITIALIZED);

    return decvpp->GetChannelParam(par, channel_id);
}