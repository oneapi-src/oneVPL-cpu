/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
/// oneAPI Video Processing Library (oneVPL) dispatcher query implementation
#include "vpl/mfxdispatcher.h"
#include "vpl/mfximplcaps.h"

#include "./cpu_workstream.h"
#include "./libmfxvplsw_caps.h"

// the auto-generated capabilities structs
// only include one time in this library
#include "./libmfxvplsw_caps_dec.h"
#include "./libmfxvplsw_caps_vpp.h"

#ifdef ENABLE_ENCODER_H264
    #include "./libmfxvplsw_caps_enc_h264.h"
#else
    #include "./libmfxvplsw_caps_enc.h"
#endif

// preferred entrypoint for 2.0 implementations (instead of MFXInitEx)
mfxStatus MFXInitialize(mfxInitializationParam par, mfxSession *session) {
    if (par.AccelerationMode != MFX_ACCEL_MODE_NA)
        return MFX_ERR_UNSUPPORTED;

    if (!session)
        return MFX_ERR_NULL_PTR;

    // create CPU workstream
    CpuWorkstream *ws = new CpuWorkstream;

    if (!ws) {
        return MFX_ERR_UNSUPPORTED;
    }

    // save the handle
    *session = (mfxSession)(ws);

    return MFX_ERR_NONE;
}

static const mfxChar strImplName[MFX_IMPL_NAME_LEN] = "oneAPI VPL CPU Reference Impl";
static const mfxChar strLicense[MFX_STRFIELD_LEN]   = "";
static const mfxChar strKeywords[MFX_STRFIELD_LEN]  = "";
static const mfxChar strDeviceID[MFX_STRFIELD_LEN]  = "CPU";

#define NUM_ACCELERATION_MODES_CPU 1

static const mfxAccelerationMode AccelerationMode[NUM_ACCELERATION_MODES_CPU] = {
    MFX_ACCEL_MODE_NA,
};

static const mfxAccelerationModeDescription AccelerationModeDescription = {
    { 0, 1 },
    {},
    NUM_ACCELERATION_MODES_CPU,
    (mfxAccelerationMode *)AccelerationMode,
};

// query and release are independent of session - called during
//   caps query and config stage using oneVPL extensions
mfxHDL *MFXQueryImplsDescription(mfxImplCapsDeliveryFormat format, mfxU32 *num_impls) {
    VPL_TRACE_FUNC;
    // only structure format is currently supported
    if (format != MFX_IMPLCAPS_IMPLDESCSTRUCTURE)
        return nullptr;

    // allocate array of mfxHDL for each implementation
    //   (currently there is just one)
    *num_impls     = 1;
    mfxHDL *hImpls = new mfxHDL[*num_impls];
    if (!hImpls)
        return nullptr;
    memset(hImpls, 0, sizeof(mfxHDL) * (*num_impls));

    // allocate ImplDescriptionArray for each implementation
    // the first element must be a struct of type mfxImplDescription
    //   so the dispatcher can cast mfxHDL to mfxImplDescription, and
    //   will just be unaware of any other fields that follow
    ImplDescriptionArray *implDescArray = new ImplDescriptionArray;
    if (!implDescArray) {
        if (hImpls)
            delete[] hImpls;
        return nullptr;
    }

    // in _each_ implDescArray we allocate, save the pointer to the array of handles
    //   and the number of elements
    // MFXReleaseImplDescription can then be called on the individual
    //   handles in any order, and when the last one is freed it
    //   will delete the array of handles
    implDescArray->basePtr  = hImpls;
    implDescArray->currImpl = 0;
    implDescArray->numImpl  = *num_impls;

    // clear everything first, then fill in fields with read-only caps data
    mfxImplDescription *implDesc = &(implDescArray->implDesc);
    memset(implDesc, 0, sizeof(mfxImplDescription));
    hImpls[0] = &(implDescArray[0]);

    implDesc->Version.Version = MFX_IMPLDESCRIPTION_VERSION;

    implDesc->Impl             = MFX_IMPL_TYPE_SOFTWARE;
    implDesc->AccelerationMode = MFX_ACCEL_MODE_NA;

    implDesc->ApiVersion.Major = MFX_VERSION_MAJOR;
    implDesc->ApiVersion.Minor = MFX_VERSION_MINOR;

    strncpy_s(implDesc->ImplName, sizeof(implDesc->ImplName), strImplName, sizeof(strImplName));
    strncpy_s(implDesc->License, sizeof(implDesc->License), strLicense, sizeof(strLicense));
    strncpy_s(implDesc->Keywords, sizeof(implDesc->Keywords), strKeywords, sizeof(strKeywords));

    implDesc->VendorID     = 0x8086;
    implDesc->VendorImplID = 0;
    implDesc->NumExtParam  = 0;

    implDesc->AccelerationModeDescription = AccelerationModeDescription;

    // initialize mfxDeviceDescription
    mfxDeviceDescription *Dev = &(implDesc->Dev);
    memset(Dev, 0, sizeof(mfxDeviceDescription)); // initially empty

    Dev->Version.Version = MFX_DEVICEDESCRIPTION_VERSION;
    strncpy_s(Dev->DeviceID, sizeof(Dev->DeviceID), strDeviceID, sizeof(strDeviceID));
    Dev->NumSubDevices = 0; // CPU should report 0

    // dec, enc, and vpp caps are auto-generated from description files
    // at runtime we just need to copy the top-level structure into
    //   the mfxImplDescription object passed back to the dispatcher
    memcpy_s(&(implDesc->Dec), sizeof(implDesc->Dec), &decoderDesc, sizeof(mfxDecoderDescription));
    memcpy_s(&(implDesc->Enc), sizeof(implDesc->Enc), &encoderDesc, sizeof(mfxEncoderDescription));
    memcpy_s(&(implDesc->VPP), sizeof(implDesc->VPP), &vppDesc, sizeof(mfxVPPDescription));

    return hImpls;
}

// walk through implDesc and delete dynamically-allocated structs
mfxStatus MFXReleaseImplDescription(mfxHDL hdl) {
    VPL_TRACE_FUNC;
    ImplDescriptionArray *implDescArray = (ImplDescriptionArray *)hdl;
    if (!implDescArray) {
        return MFX_ERR_NULL_PTR;
    }

    mfxImplDescription *implDesc = &(implDescArray->implDesc);
    if (!implDesc) {
        return MFX_ERR_NULL_PTR;
    }

    memset(implDesc, 0, sizeof(mfxImplDescription));

    // remove description from the array of handles (set to null)
    // check if this was the last description to be freed - if so,
    //   delete the array of handles
    mfxHDL *hImpls  = implDescArray->basePtr;
    mfxU32 currImpl = implDescArray->currImpl;
    mfxU32 numImpl  = implDescArray->numImpl;

    hImpls[currImpl] = nullptr;
    delete implDescArray;

    mfxU32 idx;
    for (idx = 0; idx < numImpl; idx++) {
        if (hImpls[idx])
            break;
    }

    if (idx == numImpl)
        delete[] hImpls;

    return MFX_ERR_NONE;
}

// memory functions are associated with initialized session
mfxStatus MFXMemory_GetSurfaceForVPP(mfxSession session, mfxFrameSurface1 **surface) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    CpuVPP *vpp       = ws->GetVPP();
    RET_IF_FALSE(vpp, MFX_ERR_NOT_INITIALIZED);

    return vpp->GetVPPSurface(surface);
}

mfxStatus MFXMemory_GetSurfaceForEncode(mfxSession session, mfxFrameSurface1 **surface) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws  = reinterpret_cast<CpuWorkstream *>(session);
    CpuEncode *encoder = ws->GetEncoder();
    RET_IF_FALSE(encoder, MFX_ERR_NOT_INITIALIZED);

    return encoder->GetEncodeSurface(surface);
}

mfxStatus MFXMemory_GetSurfaceForDecode(mfxSession session, mfxFrameSurface1 **surface) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws  = reinterpret_cast<CpuWorkstream *>(session);
    CpuDecode *decoder = ws->GetDecoder();
    RET_IF_FALSE(decoder, MFX_ERR_NOT_INITIALIZED);

    return decoder->GetDecodeSurface(surface);
}

mfxStatus MFXMemory_GetSurfaceForVPPOut(mfxSession session, mfxFrameSurface1 **surface) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    CpuVPP *vpp       = ws->GetVPP();
    RET_IF_FALSE(vpp, MFX_ERR_NOT_INITIALIZED);

    return vpp->GetVPPSurfaceOut(surface);
}
