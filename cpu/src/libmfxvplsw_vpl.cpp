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

#ifdef ENABLE_ENCODER_X264
    #include "./libmfxvplsw_caps_enc_x264.h"
#elif defined(ENABLE_ENCODER_OPENH264)
    #include "./libmfxvplsw_caps_enc_openh264.h"
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

#define NUM_CPU_IMPLS 1

#define NUM_ACCELERATION_MODES_CPU 1

static const mfxAccelerationMode AccelerationMode[NUM_ACCELERATION_MODES_CPU] = {
    MFX_ACCEL_MODE_NA,
};

#define NUM_POOL_POLICIES_CPU 1

static const mfxPoolAllocationPolicy PoolPolicy[NUM_POOL_POLICIES_CPU] = {
    MFX_ALLOCATION_UNLIMITED,
};

// leave table formatting alone
// clang-format off

static const mfxImplDescription cpuImplDesc = {
    { 2, 1 },                                       // struct Version
    MFX_IMPL_TYPE_SOFTWARE,                         // Impl
    MFX_ACCEL_MODE_NA,                              // AccelerationMode
    { MFX_VERSION_MINOR, MFX_VERSION_MAJOR },       // ApiVersion

    "oneAPI VPL CPU Implementation",                // ImplName

#ifdef ENABLE_ENCODER_X264
    "MIT,GPL",                                      // License
#elif defined(ENABLE_ENCODER_OPENH264)
    "MIT,BSD",                                      // License
#else
    "MIT",                                          // License
#endif

#if defined _M_IX86
    "VPL,CPU,x86",                                  // Keywords
#else
    "VPL,CPU,x64",                                  // Keywords
#endif

    0x8086,                                         // VendorID
    0,                                              // VendorImplID

    // mfxDeviceDescription Dev
    {
        { 1, 1 },          // struct Version
        {},                // reserved
        MFX_MEDIA_UNKNOWN, // MediaAdapterType
        "0000",            // DeviceID
        0,                 // NumSubDevices
        {},                // SubDevices
    },

    // mfxDecoderDescription Dec
    {
        { decoderDesc.Version.Minor, decoderDesc.Version.Major },
        {},
        decoderDesc.NumCodecs,
        (DecCodec *)decCodec,
    },

    // mfxEncoderDescription Enc
    {
        { encoderDesc.Version.Minor, encoderDesc.Version.Major },
        {},
        encoderDesc.NumCodecs,
        (EncCodec *)encCodec,
    },

    // mfxVPPDescription VPP
    {
        { vppDesc.Version.Minor, vppDesc.Version.Major },
        {},
        vppDesc.NumFilters,
        (VPPFilter *)vppFilter,
    },

    // union { mfxAccelerationModeDescription AccelerationModeDescription }
    { {
        { 0, 1 },
        {},
        NUM_ACCELERATION_MODES_CPU,
        (mfxAccelerationMode *)AccelerationMode,
    } },

    // mfxPoolPolicyDescription PoolPolicies
    {
        { 0, 1 },                       // struct Version
        {},                             // reserved
        NUM_POOL_POLICIES_CPU,          // NumPoolPolicies
        (mfxPoolAllocationPolicy*)PoolPolicy,
    },

    {},     // reserved
    0,      // NumExtParam
    {},     // ExtParams
};

static const mfxImplDescription *cpuImplDescArray[NUM_CPU_IMPLS] = {
    &cpuImplDesc,
};

// should match libvplsw.def (unless any are not actually implemented, of course)
static const mfxChar *cpuImplFuncsNames[] = {
    "MFXInit",
    "MFXClose",
    "MFXQueryIMPL",
    "MFXQueryVersion",
    "MFXJoinSession",
    "MFXDisjoinSession",
    "MFXCloneSession",
    "MFXSetPriority",
    "MFXGetPriority",
    "MFXVideoCORE_SetFrameAllocator",
    "MFXVideoCORE_SetHandle",
    "MFXVideoCORE_GetHandle",
    "MFXVideoCORE_QueryPlatform",
    "MFXVideoCORE_SyncOperation",
    "MFXVideoENCODE_Query",
    "MFXVideoENCODE_QueryIOSurf",
    "MFXVideoENCODE_Init",
    "MFXVideoENCODE_Reset",
    "MFXVideoENCODE_Close",
    "MFXVideoENCODE_GetVideoParam",
    "MFXVideoENCODE_GetEncodeStat",
    "MFXVideoENCODE_EncodeFrameAsync",
    "MFXVideoDECODE_Query",
    "MFXVideoDECODE_DecodeHeader",
    "MFXVideoDECODE_QueryIOSurf",
    "MFXVideoDECODE_Init",
    "MFXVideoDECODE_Reset",
    "MFXVideoDECODE_Close",
    "MFXVideoDECODE_GetVideoParam",
    "MFXVideoDECODE_GetDecodeStat",
    "MFXVideoDECODE_SetSkipMode",
    "MFXVideoDECODE_GetPayload",
    "MFXVideoDECODE_DecodeFrameAsync",
    "MFXVideoVPP_Query",
    "MFXVideoVPP_QueryIOSurf",
    "MFXVideoVPP_Init",
    "MFXVideoVPP_Reset",
    "MFXVideoVPP_Close",
    "MFXVideoVPP_GetVideoParam",
    "MFXVideoVPP_GetVPPStat",
    "MFXVideoVPP_RunFrameVPPAsync",
    "MFXInitEx",
    "MFXQueryImplsDescription",
    "MFXReleaseImplDescription",
    "MFXMemory_GetSurfaceForVPP",
    "MFXMemory_GetSurfaceForEncode",
    "MFXMemory_GetSurfaceForDecode",
    "MFXInitialize",
    "MFXMemory_GetSurfaceForVPPOut",
    "MFXVideoDECODE_VPP_Init",
    "MFXVideoDECODE_VPP_DecodeFrameAsync",
    "MFXVideoDECODE_VPP_Reset",
    "MFXVideoDECODE_VPP_GetChannelParam",
    "MFXVideoDECODE_VPP_Close",
    "MFXVideoVPP_ProcessFrameAsync", 
};

static const mfxImplementedFunctions cpuImplFuncs = {
    sizeof(cpuImplFuncsNames) / sizeof(mfxChar *),
    (mfxChar**)cpuImplFuncsNames
};

static const mfxImplementedFunctions *cpuImplFuncsArray[NUM_CPU_IMPLS] = {
    &cpuImplFuncs,
};

// end table formatting
// clang-format on

// query and release are independent of session - called during
//   caps query and config stage using oneVPL extensions
mfxHDL *MFXQueryImplsDescription(mfxImplCapsDeliveryFormat format, mfxU32 *num_impls) {
    VPL_TRACE_FUNC;

    *num_impls = NUM_CPU_IMPLS;

    switch (format) {
        case MFX_IMPLCAPS_IMPLDESCSTRUCTURE:
            return (mfxHDL *)(cpuImplDescArray);
        case MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS:
            return (mfxHDL *)(cpuImplFuncsArray);
#ifdef ONEVPL_EXPERIMENTAL
        case MFX_IMPLCAPS_DEVICE_ID_EXTENDED:
            return nullptr;
#endif // ONEVPL_EXPERIMENTAL
        default:
            return nullptr;
    }
}

// walk through implDesc and delete dynamically-allocated structs
mfxStatus MFXReleaseImplDescription(mfxHDL hdl) {
    VPL_TRACE_FUNC;

    if (!hdl)
        return MFX_ERR_NULL_PTR;

    // nothing to do - caps are stored in ROM table

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
