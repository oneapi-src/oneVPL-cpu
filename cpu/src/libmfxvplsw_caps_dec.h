/*############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################*/

//NOLINT(build/header_guard)

#include "src/libmfxvplsw_caps.h"

const mfxU32 decColorFmt_c00_p00_m00[] = {
    MFX_FOURCC_I420,
    MFX_FOURCC_I010,
};

const DecMemDesc decMemDesc_c00_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        2,
        (mfxU32 *)decColorFmt_c00_p00_m00,
    },
};

const DecProfile decProfile_c00[] = {
    {
        MFX_PROFILE_AV1_MAIN,
        {},
        1,
        (DecMemDesc *)decMemDesc_c00_p00,
    },
};

const mfxU32 decColorFmt_c01_p00_m00[] = {
    MFX_FOURCC_I420,
};

const DecMemDesc decMemDesc_c01_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)decColorFmt_c01_p00_m00,
    },
};

const DecProfile decProfile_c01[] = {
    {
        MFX_PROFILE_AVC_HIGH,
        {},
        1,
        (DecMemDesc *)decMemDesc_c01_p00,
    },
};

const mfxU32 decColorFmt_c02_p00_m00[] = {
    MFX_FOURCC_I420,
};

const DecMemDesc decMemDesc_c02_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)decColorFmt_c02_p00_m00,
    },
};

const mfxU32 decColorFmt_c02_p01_m00[] = {
    MFX_FOURCC_I010,
};

const DecMemDesc decMemDesc_c02_p01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)decColorFmt_c02_p01_m00,
    },
};

const DecProfile decProfile_c02[] = {
    {
        MFX_PROFILE_HEVC_MAIN,
        {},
        1,
        (DecMemDesc *)decMemDesc_c02_p00,
    },
    {
        MFX_PROFILE_HEVC_MAIN10,
        {},
        1,
        (DecMemDesc *)decMemDesc_c02_p01,
    },
};

const mfxU32 decColorFmt_c03_p00_m00[] = {
    MFX_FOURCC_I420,
};

const DecMemDesc decMemDesc_c03_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)decColorFmt_c03_p00_m00,
    },
};

const DecProfile decProfile_c03[] = {
    {
        MFX_PROFILE_JPEG_BASELINE,
        {},
        1,
        (DecMemDesc *)decMemDesc_c03_p00,
    },
};

const mfxU32 decColorFmt_c04_p00_m00[] = {
    MFX_FOURCC_I420,
};

const DecMemDesc decMemDesc_c04_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        1,
        (mfxU32 *)decColorFmt_c04_p00_m00,
    },
};

const DecProfile decProfile_c04[] = {
    {
        MFX_PROFILE_MPEG2_MAIN,
        {},
        1,
        (DecMemDesc *)decMemDesc_c04_p00,
    },
};

const DecCodec decCodec[] = {
    {
        MFX_CODEC_AV1,
        {},
        MFX_LEVEL_AV1_53,
        1,
        (DecProfile *)decProfile_c00,
    },
    {
        MFX_CODEC_AVC,
        {},
        MFX_LEVEL_AVC_52,
        1,
        (DecProfile *)decProfile_c01,
    },
    {
        MFX_CODEC_HEVC,
        {},
        MFX_LEVEL_HEVC_51,
        2,
        (DecProfile *)decProfile_c02,
    },
    {
        MFX_CODEC_JPEG,
        {},
        MFX_LEVEL_UNKNOWN,
        1,
        (DecProfile *)decProfile_c03,
    },
    {
        MFX_CODEC_MPEG2,
        {},
        MFX_LEVEL_MPEG2_MAIN,
        1,
        (DecProfile *)decProfile_c04,
    },
};

const mfxDecoderDescription decoderDesc = {
    { 0, 1 },
    {},
    5,
    (DecCodec *)decCodec,
};
