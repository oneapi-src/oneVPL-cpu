/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "vpl/mfx_dispatcher_vpl.h"

// implementation of config context (mfxConfig)
// each loader instance can have one or more configs
//   associated with it - used for filtering implementations
//   based on what they support (codec types, etc.)
ConfigCtxOneVPL::ConfigCtxOneVPL()
        : m_propName(),
          m_propValue(),
          m_propIdx(),
          m_propParsedString() {
    m_propValue.Version = { 0, 1 };
    m_propValue.Type    = MFX_VARIANT_TYPE_UNSET;

    return;
}

ConfigCtxOneVPL::~ConfigCtxOneVPL() {
    return;
}

struct PropVariant {
    const char* Name;
    mfxVariantType Type;
};

enum PropIdx {
    // settable config properties for mfxImplDescription
    ePropMain_Impl = 0,
    ePropMain_accelerationMode,
    ePropMain_VendorID,
    ePropMain_VendorImplID,

    // settable config properties for mfxDecoderDescription
    ePropDec_CodecID,
    ePropDec_MaxcodecLevel,
    ePropDec_Profile,
    ePropDec_MemHandleType,
    ePropDec_ColorFormats,

    // settable config properties for mfxEncoderDescription
    ePropEnc_CodecID,
    ePropEnc_MaxcodecLevel,
    ePropEnc_BiDirectionalPrediction,
    ePropEnc_Profile,
    ePropEnc_MemHandleType,
    ePropEnc_ColorFormats,

    // settable config properties for mfxVPPDescription
    ePropVPP_FilterFourCC,
    ePropVPP_MaxDelayInFrames,
    ePropVPP_MemHandleType,
    ePropVPP_OutFormats,

    // number of entries (always last)
    eProp_TotalProps
};

// order must align exactly with PropIdx list
// to avoid mismatches, this should be automated (e.g. pre-processor step)
static const PropVariant PropIdxTab[] = {
    { "ePropMain_Impl", MFX_VARIANT_TYPE_U32 },
    { "ePropMain_accelerationMode", MFX_VARIANT_TYPE_U16 },
    { "ePropMain_VendorID", MFX_VARIANT_TYPE_U32 },
    { "ePropMain_VendorImplID", MFX_VARIANT_TYPE_U32 },

    { "ePropDec_CodecID", MFX_VARIANT_TYPE_U32 },
    { "ePropDec_MaxcodecLevel", MFX_VARIANT_TYPE_U16 },
    { "ePropDec_Profile", MFX_VARIANT_TYPE_U32 },
    { "ePropDec_MemHandleType", MFX_VARIANT_TYPE_I32 },
    { "ePropDec_ColorFormats", MFX_VARIANT_TYPE_U32 },

    { "ePropEnc_CodecID", MFX_VARIANT_TYPE_U32 },
    { "ePropEnc_MaxcodecLevel", MFX_VARIANT_TYPE_U16 },
    { "ePropEnc_BiDirectionalPrediction", MFX_VARIANT_TYPE_U16 },
    { "ePropEnc_Profile", MFX_VARIANT_TYPE_U32 },
    { "ePropEnc_MemHandleType", MFX_VARIANT_TYPE_I32 },
    { "ePropEnc_ColorFormats", MFX_VARIANT_TYPE_U32 },

    { "ePropVPP_FilterFourCC", MFX_VARIANT_TYPE_U32 },
    { "ePropVPP_MaxDelayInFrames", MFX_VARIANT_TYPE_U16 },
    { "ePropVPP_MemHandleType", MFX_VARIANT_TYPE_I32 },
    { "ePropVPP_OutFormats", MFX_VARIANT_TYPE_U32 },
};

mfxStatus ConfigCtxOneVPL::ValidateAndSetProp(mfxI32 idx, mfxVariant value) {
    if (idx < 0 || idx >= eProp_TotalProps)
        return MFX_ERR_NOT_FOUND;

    if (value.Type != PropIdxTab[idx].Type)
        return MFX_ERR_UNSUPPORTED;

    m_propIdx = idx;

    m_propValue.Type = value.Type;
    m_propValue.Data = value.Data;

    return MFX_ERR_NONE;
}

mfxStatus ConfigCtxOneVPL::SetFilterPropertyDec(mfxVariant value) {
    std::string nextProp;

    nextProp = GetNextProp(&m_propParsedString);

    // no settable top-level members
    if (nextProp != "decoder")
        return MFX_ERR_NOT_FOUND;

    // parse 'decoder'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "CodecID") {
        return ValidateAndSetProp(ePropDec_CodecID, value);
    }
    else if (nextProp == "MaxcodecLevel") {
        return ValidateAndSetProp(ePropDec_MaxcodecLevel, value);
    }
    else if (nextProp != "decprofile") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'decprofile'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "Profile") {
        return ValidateAndSetProp(ePropDec_Profile, value);
    }
    else if (nextProp != "decmemdesc") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'decmemdesc'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "MemHandleType") {
        return ValidateAndSetProp(ePropDec_MemHandleType, value);
    }
    else if (nextProp == "ColorFormats") {
        return ValidateAndSetProp(ePropDec_ColorFormats, value);
    }

    // end of mfxDecoderDescription options
    return MFX_ERR_NOT_FOUND;
}

mfxStatus ConfigCtxOneVPL::SetFilterPropertyEnc(mfxVariant value) {
    std::string nextProp;

    nextProp = GetNextProp(&m_propParsedString);

    // no settable top-level members
    if (nextProp != "encoder")
        return MFX_ERR_NOT_FOUND;

    // parse 'encoder'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "CodecID") {
        return ValidateAndSetProp(ePropEnc_CodecID, value);
    }
    else if (nextProp == "MaxcodecLevel") {
        return ValidateAndSetProp(ePropEnc_MaxcodecLevel, value);
    }
    else if (nextProp == "BiDirectionalPrediction") {
        return ValidateAndSetProp(ePropEnc_BiDirectionalPrediction, value);
    }
    else if (nextProp != "encprofile") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'encprofile'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "Profile") {
        return ValidateAndSetProp(ePropEnc_Profile, value);
    }
    else if (nextProp != "encmemdesc") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'encmemdesc'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "MemHandleType") {
        return ValidateAndSetProp(ePropEnc_MemHandleType, value);
    }
    else if (nextProp == "ColorFormats") {
        return ValidateAndSetProp(ePropEnc_ColorFormats, value);
    }

    // end of mfxEncoderDescription options
    return MFX_ERR_NOT_FOUND;
}

mfxStatus ConfigCtxOneVPL::SetFilterPropertyVPP(mfxVariant value) {
    std::string nextProp;

    nextProp = GetNextProp(&m_propParsedString);

    // no settable top-level members
    if (nextProp != "filter")
        return MFX_ERR_NOT_FOUND;

    // parse 'filter'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "FilterFourCC") {
        return ValidateAndSetProp(ePropVPP_FilterFourCC, value);
    }
    else if (nextProp == "MaxDelayInFrames") {
        return ValidateAndSetProp(ePropVPP_MaxDelayInFrames, value);
    }
    else if (nextProp != "memdesc") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'memdesc'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "MemHandleType") {
        return ValidateAndSetProp(ePropVPP_MemHandleType, value);
    }
    else if (nextProp != "format") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'format'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "OutFormats") {
        return ValidateAndSetProp(ePropVPP_OutFormats, value);
    }

    // end of mfxVPPDescription options
    return MFX_ERR_NOT_FOUND;
}

// return codes (from spec):
//   MFX_ERR_NOT_FOUND - name contains unknown parameter name
//   MFX_ERR_UNSUPPORTED - value data type != parameter with provided name
mfxStatus ConfigCtxOneVPL::SetFilterProperty(const mfxU8* name,
                                             mfxVariant value) {
    m_propName = std::string((char*)name);

    // definitions will be added to API
    m_propValue.Version.Major = 1;
    m_propValue.Version.Minor = 0;

    // initially set Type = unset (invalid)
    // if valid property string and value are passed in,
    //   this will be updated
    // otherwise loader will ignore this cfg during EnumImplementations
    m_propValue.Type     = MFX_VARIANT_TYPE_UNSET;
    m_propValue.Data.U32 = 0;

    // parse property string into individual properties,
    //   separated by '.'
    std::stringstream prop((char*)name);
    std::string s;
    while (getline(prop, s, '.')) {
        m_propParsedString.push_back(s);
    }

    // get first property descriptor - must be "mfxImplDescription"
    std::string nextProp = GetNextProp(&m_propParsedString);
    if (nextProp != "mfxImplDescription") {
        return MFX_ERR_NOT_FOUND;
    }

    // get next property descriptor
    nextProp = GetNextProp(&m_propParsedString);

    // property is a top-level member of mfxImplDescription
    if (nextProp == "Impl") {
        return ValidateAndSetProp(ePropMain_Impl, value);
    }
    else if (nextProp == "accelerationMode") {
        return ValidateAndSetProp(ePropMain_accelerationMode, value);
    }
    else if (nextProp == "VendorID") {
        return ValidateAndSetProp(ePropMain_VendorID, value);
    }
    else if (nextProp == "VendorImplID") {
        return ValidateAndSetProp(ePropMain_VendorImplID, value);
    }

    // property is a member of mfxDecoderDescription
    if (nextProp == "mfxDecoderDescription") {
        return SetFilterPropertyDec(value);
    }

    if (nextProp == "mfxEncoderDescription") {
        return SetFilterPropertyEnc(value);
    }

    if (nextProp == "mfxVPPDescription") {
        return SetFilterPropertyVPP(value);
    }

    return MFX_ERR_NOT_FOUND;
}
