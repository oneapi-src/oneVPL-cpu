/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef CPU_SRC_CPU_DECODEVPP_H_
#define CPU_SRC_CPU_DECODEVPP_H_

#include <memory>
#include "src/cpu_common.h"
#include "src/cpu_decode.h"
#include "src/cpu_frame_pool.h"
#include "src/cpu_vpp.h"

class CpuWorkstream;

class CpuDecodeVPP {
public:
    explicit CpuDecodeVPP(CpuWorkstream* session);
    ~CpuDecodeVPP();

    mfxStatus InitDecodeVPP(mfxVideoParam* par,
                            mfxVideoChannelParam** vpp_par_array,
                            mfxU32 num_vpp_par);
    mfxStatus Reset(mfxVideoParam* par, mfxVideoChannelParam** vpp_par_array, mfxU32 num_vpp_par);
    mfxStatus DecodeVPPFrame(mfxBitstream* bs,
                             mfxU32* skip_channels,
                             mfxU32 num_skip_channels,
                             mfxSurfaceArray** surf_array_out);

    mfxU32 GetVPPChannelCount(void);
    mfxStatus GetChannelParam(mfxVideoChannelParam* par, mfxU32 channel_id);

    mfxStatus CheckVideoParamDecodeVPP(mfxVideoParam* in);
    mfxStatus IsSameVideoParam(mfxVideoParam* newPar, mfxVideoParam* oldPar);

    mfxStatus CheckVideoChannelParamDecodeVPP(mfxVideoChannelParam** inChPar, mfxU32 num_ch);
    mfxStatus IsSameVideoChannelParam(mfxVideoChannelParam* newChPar,
                                      mfxVideoChannelParam* oldChPar);
    mfxStatus Close();

private:
    CpuVPP* m_cpuVPP;
    mfxVideoChannelParam** m_vppChParams;
    mfxFrameSurface1** m_surfOut;
    mfxU32 m_numVPPCh;
    mfxU32 m_numSurfs;
    CpuWorkstream* m_session;
    mfxSession m_mfxsession;

    mfxStatus InitVPP(mfxVideoParam* par, mfxVideoChannelParam** vpp_par_array, mfxU32 num_vpp_par);

    /* copy not allowed */
    CpuDecodeVPP(const CpuDecodeVPP&);
    CpuDecodeVPP& operator=(const CpuDecodeVPP&);
};

#endif // CPU_SRC_CPU_DECODEVPP_H_
