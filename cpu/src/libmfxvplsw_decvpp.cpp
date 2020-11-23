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
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_VPP_DecodeFrameAsync(mfxSession session,
                                              mfxBitstream *bs,
                                              mfxU32 *skip_channels,
                                              mfxU32 num_skip_channels,
                                              mfxSurfaceArray **surf_array_out) {
    VPL_TRACE_FUNC;
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_VPP_Reset(mfxSession session,
                                   mfxVideoParam *decode_par,
                                   mfxVideoChannelParam **vpp_par_array,
                                   mfxU32 num_vpp_par) {
    VPL_TRACE_FUNC;
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_VPP_GetChannelParam(mfxSession session,
                                             mfxVideoChannelParam *par,
                                             mfxU32 channel_id) {
    VPL_TRACE_FUNC;
    return MFX_ERR_NOT_IMPLEMENTED;
}
