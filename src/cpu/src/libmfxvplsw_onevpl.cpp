/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"

#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#if defined(_WIN32) || defined(_WIN64)
#else
    // Linux
    #define strcpy_s(dst, size, src)       strcpy((dst), (src)) // NOLINT
    #define strncpy_s(dst, size, src, cnt) strcpy((dst), (src)) // NOLINT
#endif

// query and release are independent of session - called during
//   caps query and config stage using oneVPL extensions
mfxHDL MFXQueryImplDescription(mfxImplCapsDeliveryFormat format) {
    // only structure format is currently supported
    if (format != MFX_IMPLCAPS_IMPLDESCSTRUCTURE)
        return nullptr;

    mfxImplDescription *implDesc = new mfxImplDescription;

    memset(implDesc, 0, sizeof(mfxImplDescription));

    implDesc->Version.Major = 1; // should be defined in header
    implDesc->Version.Minor = 0;

    implDesc->Impl             = MFX_IMPL_SOFTWARE;
    implDesc->accelerationMode = 0;

    implDesc->ApiVersion.Major = MFX_VERSION_MAJOR;
    implDesc->ApiVersion.Minor = MFX_VERSION_MINOR;

    strncpy_s((char *)implDesc->ImplName,
              sizeof(implDesc->ImplName),
              "TODO(JR) - implementation name",
              sizeof(implDesc->ImplName) - 1);
    strncpy_s((char *)implDesc->License,
              sizeof(implDesc->License),
              "TODO(JR) - license name",
              sizeof(implDesc->ImplName) - 1);
    strncpy_s((char *)implDesc->Keywords,
              sizeof(implDesc->Keywords),
              "TODO(JR) - keyword1, keyword2...",
              sizeof(implDesc->ImplName) - 1);

    implDesc->VendorID     = 0x8086;
    implDesc->VendorImplID = 0;
    implDesc->NumExtParam  = 0;

    mfxU32 i;

    // fill decoder caps
    implDesc->Dec.Version.Major = 1;
    implDesc->Dec.Version.Minor = 0;
    implDesc->Dec.NumCodecs     = 1;

    // allocate one structure per supported codec
    implDesc->Dec.Codecs =
        new struct mfxDecoderDescription::decoder[implDesc->Dec.NumCodecs];
    for (i = 0; i < implDesc->Dec.NumCodecs; i++) {
        mfxDecoderDescription::decoder *currDec = &(implDesc->Dec.Codecs[i]);
        memset(currDec, 0, sizeof(mfxDecoderDescription::decoder));

        currDec->CodecID = MFX_CODEC_HEVC;
        // TODO(JR) - fill in levels, profiles, etc.
        // add some utility functions to automate all this
    }

    // fill encoder caps
    implDesc->Enc.Version.Major = 1;
    implDesc->Enc.Version.Minor = 0;
    implDesc->Enc.NumCodecs     = 1;

    // allocate one structure per supported codec
    implDesc->Enc.Codecs =
        new struct mfxEncoderDescription::encoder[implDesc->Enc.NumCodecs];
    for (i = 0; i < implDesc->Enc.NumCodecs; i++) {
        mfxEncoderDescription::encoder *currEnc = &(implDesc->Enc.Codecs[i]);
        memset(currEnc, 0, sizeof(mfxEncoderDescription::encoder));

        currEnc->CodecID = MFX_CODEC_HEVC;
        // TODO(JR) - fill in levels, profiles, etc.
    }

    // fill VPP caps
    implDesc->VPP.Version.Major = 1;
    implDesc->VPP.Version.Minor = 0;
    implDesc->VPP.NumFilters    = 0;

    return (mfxHDL)implDesc;
}

mfxStatus MFXReleaseImplDescription(mfxHDL hdl) {
    mfxImplDescription *implDesc = (mfxImplDescription *)hdl;

    if (!implDesc) {
        return MFX_ERR_NULL_PTR;
    }

    // TODO(JR) - walk through implDesc and delete dynamically-allocated structs

    delete implDesc;

    return MFX_ERR_NONE;
}

// memory functions are associated with initialized session
mfxStatus MFXMemory_GetSurfaceForVPP(mfxSession session,
                                     mfxFrameSurface1 **surface) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXMemory_GetSurfaceForEncode(mfxSession session,
                                        mfxFrameSurface1 **surface) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXMemory_GetSurfaceForDecode(mfxSession session,
                                        mfxFrameSurface1 **surface) {
    return MFX_ERR_UNSUPPORTED;
}
