/*############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################*/

//NOLINT(build/header_guard)

#include "src/libmfxvplsw_caps.h"

// av1
const mfxU32 encColorFmt_c00_p00_m00[] = {
    MFX_FOURCC_I420,
    MFX_FOURCC_I010,
};

const EncMemDesc encMemDesc_c00_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        2,
        (mfxU32 *)encColorFmt_c00_p00_m00,
    },
};

const EncProfile encProfile_c00[] = {
    {
        MFX_PROFILE_AV1_MAIN,
        {},
        1,
        (EncMemDesc *)encMemDesc_c00_p00,
    },
};

#ifdef BUILD_GPL_X264
// x264
const mfxU32 encColorFmt_c01_p00_m00[] = {
    MFX_FOURCC_I420,
};

const EncMemDesc encMemDesc_c01_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)encColorFmt_c01_p00_m00,
    },
};

const mfxU32 encColorFmt_c01_p01_m00[] = {
    MFX_FOURCC_I010,
};

const EncMemDesc encMemDesc_c01_p01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)encColorFmt_c01_p01_m00,
    },
};

const EncProfile encProfile_c01[] = {
    {
        MFX_PROFILE_AVC_HIGH,
        {},
        1,
        (EncMemDesc *)encMemDesc_c01_p00,
    },
    {
        MFX_PROFILE_AVC_HIGH10,
        {},
        1,
        (EncMemDesc *)encMemDesc_c01_p01,
    },
};

#define ENABLED_H264

#else
#ifdef __linux__
    // openh264
    const mfxU32 encColorFmt_c01_p00_m00[] = {
        MFX_FOURCC_I420,
    };

    const EncMemDesc encMemDesc_c01_p00[] = {
        {
            MFX_RESOURCE_SYSTEM_SURFACE,
            { 64, 4096, 8 },
            { 64, 4096, 8 },
            {},
            1,
            (mfxU32 *)encColorFmt_c01_p00_m00,
        },
    };

    const EncProfile encProfile_c01[] = {
        {
            MFX_PROFILE_AVC_CONSTRAINED_BASELINE,
            {},
            1,
            (EncMemDesc *)encMemDesc_c01_p00,
        },
        {
            MFX_PROFILE_AVC_MAIN,
            {},
            1,
            (EncMemDesc *)encMemDesc_c01_p00,
        },
        {
            MFX_PROFILE_AVC_HIGH,
            {},
            1,
            (EncMemDesc *)encMemDesc_c01_p00,
        },
    };

#define ENABLED_H264

#endif
#endif

#ifdef ENABLED_H264
const mfxU32 encColorFmt_c02_p00_m00[] = {
    MFX_FOURCC_I420,
};

const EncMemDesc encMemDesc_c02_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)encColorFmt_c02_p00_m00,
    },
};

const mfxU32 encColorFmt_c02_p01_m00[] = {
    MFX_FOURCC_I010,
};

const EncMemDesc encMemDesc_c02_p01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)encColorFmt_c02_p01_m00,
    },
};

const EncProfile encProfile_c02[] = {
    {
        MFX_PROFILE_HEVC_MAIN,
        {},
        1,
        (EncMemDesc *)encMemDesc_c02_p00,
    },
    {
        MFX_PROFILE_HEVC_MAIN10,
        {},
        1,
        (EncMemDesc *)encMemDesc_c02_p01,
    },
};

const mfxU32 encColorFmt_c03_p00_m00[] = {
    MFX_FOURCC_I420,
};

const EncMemDesc encMemDesc_c03_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)encColorFmt_c03_p00_m00,
    },
};

const EncProfile encProfile_c03[] = {
    {
        MFX_PROFILE_JPEG_BASELINE,
        {},
        1,
        (EncMemDesc *)encMemDesc_c03_p00,
    },
};

const EncCodec encCodec[] = {
    {
        MFX_CODEC_AV1,
        MFX_LEVEL_AV1_53,
        1,
        {},
        1,
        (EncProfile *)encProfile_c00,
    },
#ifdef BUILD_GPL_X264
    {
        MFX_CODEC_AVC,
        MFX_LEVEL_AVC_52,
        1,
        {},
        2,
        (EncProfile *)encProfile_c01,
    },
#else   // openh264
    {
        MFX_CODEC_AVC,
        MFX_LEVEL_UNKNOWN,
        1,
        {},
        3,
        (EncProfile *)encProfile_c01,
    },
#endif
    {
        MFX_CODEC_HEVC,
        MFX_LEVEL_HEVC_51,
        1,
        {},
        2,
        (EncProfile *)encProfile_c02,
    },
    {
        MFX_CODEC_JPEG,
        MFX_LEVEL_UNKNOWN,
        0,
        {},
        1,
        (EncProfile *)encProfile_c03,
    },
};

const mfxEncoderDescription encoderDesc = {
    { 0, 1 },
    {},
    4,
    (EncCodec *)encCodec,
};
#else
const mfxU32 encColorFmt_c01_p00_m00[] = {
    MFX_FOURCC_I420,
};

const EncMemDesc encMemDesc_c01_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)encColorFmt_c01_p00_m00,
    },
};

const mfxU32 encColorFmt_c01_p01_m00[] = {
    MFX_FOURCC_I010,
};

const EncMemDesc encMemDesc_c01_p01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)encColorFmt_c01_p01_m00,
    },
};

const EncProfile encProfile_c01[] = {
    {
        MFX_PROFILE_HEVC_MAIN,
        {},
        1,
        (EncMemDesc *)encMemDesc_c01_p00,
    },
    {
        MFX_PROFILE_HEVC_MAIN10,
        {},
        1,
        (EncMemDesc *)encMemDesc_c01_p01,
    },
};

const mfxU32 encColorFmt_c02_p00_m00[] = {
    MFX_FOURCC_I420,
};

const EncMemDesc encMemDesc_c02_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)encColorFmt_c02_p00_m00,
    },
};

const EncProfile encProfile_c02[] = {
    {
        MFX_PROFILE_JPEG_BASELINE,
        {},
        1,
        (EncMemDesc *)encMemDesc_c02_p00,
    },
};

const EncCodec encCodec[] = {
    {
        MFX_CODEC_AV1,
        MFX_LEVEL_AV1_53,
        1,
        {},
        1,
        (EncProfile *)encProfile_c00,
    },
    {
        MFX_CODEC_HEVC,
        MFX_LEVEL_HEVC_51,
        1,
        {},
        2,
        (EncProfile *)encProfile_c01,
    },
    {
        MFX_CODEC_JPEG,
        MFX_LEVEL_UNKNOWN,
        0,
        {},
        1,
        (EncProfile *)encProfile_c02,
    },
};

const mfxEncoderDescription encoderDesc = {
    { 0, 1 },
    {},
    3,
    (EncCodec *)encCodec,
};
#endif