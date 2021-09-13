/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "./smart_dispatcher.h"
#include "api/unit_api.h"

// leave table formatting alone
// clang-format off

struct TestPropVal {
    mfxVariantType varType;
    const char *name;
    mfxU64 validData;
    mfxU64 invalidData;
};

// all valid properties (CPU runtime) except non-filtering parameters (SetHandle, etc.)
// ApiVersion.Major and ApiVersion.Minor need to be used together, so not included here (handled in other test cases)
static const TestPropVal TestPropIntTab[] = {
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.Impl",                                                              MFX_IMPL_TYPE_SOFTWARE, 0xffffffff },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.AccelerationMode",                                                  MFX_ACCEL_MODE_NA, 0xffffffff },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.ApiVersion.Version",                                                (MFX_VERSION_MAJOR << 16) | (MFX_VERSION_MINOR), (99 << 16) | (0) },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.VendorID",                                                          0x8086, 0xefefefef },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.VendorImplID",                                                      0, 0xffffffff },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxSurfacePoolMode",                                                MFX_ALLOCATION_UNLIMITED, 0xefefefef },

    { MFX_VARIANT_TYPE_U16, "mfxImplDescription.mfxDeviceDescription.device.DeviceID",                              0, 0xefef },
    { MFX_VARIANT_TYPE_U16, "mfxImplDescription.mfxDeviceDescription.device.MediaAdapterType",                      MFX_MEDIA_UNKNOWN, MFX_MEDIA_INTEGRATED },

    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID",                             MFX_CODEC_AVC, MFX_CODEC_VC1 },
    { MFX_VARIANT_TYPE_U16, "mfxImplDescription.mfxDecoderDescription.decoder.MaxcodecLevel",                       MFX_LEVEL_AVC_52, MFX_LEVEL_AVC_62 },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile",                  MFX_PROFILE_AVC_HIGH, MFX_PROFILE_AVC_HIGH_422 },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.MemHandleType", MFX_RESOURCE_SYSTEM_SURFACE, MFX_RESOURCE_DX12_RESOURCE },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.ColorFormats",  MFX_FOURCC_I420, MFX_FOURCC_NV12 },

    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxEncoderDescription.encoder.CodecID",                             MFX_CODEC_HEVC, MFX_CODEC_VC1 },
    { MFX_VARIANT_TYPE_U16, "mfxImplDescription.mfxEncoderDescription.encoder.MaxcodecLevel",                       MFX_LEVEL_HEVC_51, MFX_LEVEL_HEVC_62 },
    { MFX_VARIANT_TYPE_U16, "mfxImplDescription.mfxEncoderDescription.encoder.BiDirectionalPrediction",             1, 0xefef },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.Profile",                  MFX_PROFILE_AV1_MAIN, MFX_PROFILE_AV1_PRO },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.MemHandleType", MFX_RESOURCE_SYSTEM_SURFACE, MFX_RESOURCE_DX12_RESOURCE },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.ColorFormats",  MFX_FOURCC_I420, MFX_FOURCC_NV12 },

    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxVPPDescription.filter.FilterFourCC",                             MFX_EXTBUFF_VPP_COLOR_CONVERSION, MFX_EXTBUFF_VPP_DEINTERLACING },
    { MFX_VARIANT_TYPE_U16, "mfxImplDescription.mfxVPPDescription.filter.MaxDelayInFrames",                         1, 0xefef },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxVPPDescription.filter.memdesc.MemHandleType",                    MFX_RESOURCE_SYSTEM_SURFACE, MFX_RESOURCE_DX12_RESOURCE },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxVPPDescription.filter.memdesc.format.InFormat",                  MFX_FOURCC_I420, MFX_FOURCC_NV12 },
    { MFX_VARIANT_TYPE_U32, "mfxImplDescription.mfxVPPDescription.filter.memdesc.format.OutFormats",                MFX_FOURCC_I010, MFX_FOURCC_P010 },
};

#define NUM_TEST_PROP_INT (sizeof(TestPropIntTab) / sizeof(TestPropVal))

struct TestPropPtr {
    mfxVariantType varType;
    const char *name;
    const void *validData;
    const void *invalidData;
};

static const mfxRange32U cpuRangeValid         = { 64, 4096, 8 };
static const mfxRange32U cpuRangeInvalidMin    = { 32, 4096, 8 };
static const mfxRange32U cpuRangeInvalidMax    = { 64, 9000, 8 };
static const mfxRange32U cpuRangeInvalidStep   = { 64, 4096, 4 };

static const TestPropPtr TestPropPtrTab[] = {
    { MFX_VARIANT_TYPE_PTR, "mfxImplDescription.ImplName",                                                          "oneAPI VPL CPU Reference Impl", "INVALID" },
    { MFX_VARIANT_TYPE_PTR, "mfxImplDescription.License",                                                           "MIT", "INVALID" },
    { MFX_VARIANT_TYPE_PTR, "mfxImplDescription.Keywords",                                                          "CPU", "INVALID" },

    { MFX_VARIANT_TYPE_PTR, "mfxImplDescription.mfxDeviceDescription.device.DeviceID",                              "0000", "efef" },

    { MFX_VARIANT_TYPE_PTR, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.Width",         reinterpret_cast<const void *>(&cpuRangeValid), reinterpret_cast<const void *>(&cpuRangeInvalidMin)  },
    { MFX_VARIANT_TYPE_PTR, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.Height",        reinterpret_cast<const void *>(&cpuRangeValid), reinterpret_cast<const void *>(&cpuRangeInvalidMax)  },

    { MFX_VARIANT_TYPE_PTR, "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.Width",         reinterpret_cast<const void *>(&cpuRangeValid), reinterpret_cast<const void *>(&cpuRangeInvalidStep) },
    { MFX_VARIANT_TYPE_PTR, "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.Height",        reinterpret_cast<const void *>(&cpuRangeValid), reinterpret_cast<const void *>(&cpuRangeInvalidMin)  },

    { MFX_VARIANT_TYPE_PTR, "mfxImplDescription.mfxVPPDescription.filter.memdesc.Width",                            reinterpret_cast<const void *>(&cpuRangeValid), reinterpret_cast<const void *>(&cpuRangeInvalidMax)  },
    { MFX_VARIANT_TYPE_PTR, "mfxImplDescription.mfxVPPDescription.filter.memdesc.Height",                           reinterpret_cast<const void *>(&cpuRangeValid), reinterpret_cast<const void *>(&cpuRangeInvalidStep) },

    { MFX_VARIANT_TYPE_PTR, "mfxImplementedFunctions.FunctionsName",                                                "MFXVideoDECODE_VPP_Init", "INVALID" },
};

#define NUM_TEST_PROP_PTR (sizeof(TestPropPtrTab) / sizeof(TestPropPtr))

// end table formatting
// clang-format on

// set all properties in a separate cfg object for each
TEST(Dispatcher_SW_MultiProp, AllPropsValidOnePerConfig) {
    mfxStatus sts    = MFX_ERR_NONE;
    mfxLoader loader = nullptr;

    loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    for (mfxU32 i = 0; i < NUM_TEST_PROP_INT; i++) {
        sts = SetSingleProperty<mfxU64>(loader, TestPropIntTab[i].name, TestPropIntTab[i].varType, (mfxU64)TestPropIntTab[i].validData);
        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    for (mfxU32 i = 0; i < NUM_TEST_PROP_PTR; i++) {
        sts = SetSingleProperty<mfxHDL>(loader, TestPropPtrTab[i].name, TestPropPtrTab[i].varType, (mfxHDL)TestPropPtrTab[i].validData);
        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

// set all properties in the same cfg object for each
TEST(Dispatcher_SW_MultiProp, AllPropsValidSingleConfig) {
    mfxStatus sts    = MFX_ERR_NONE;
    mfxLoader loader = nullptr;

    loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    for (mfxU32 i = 0; i < NUM_TEST_PROP_INT; i++) {
        sts = SetSingleProperty<mfxU64>(loader, cfg, TestPropIntTab[i].name, TestPropIntTab[i].varType, (mfxU64)TestPropIntTab[i].validData);
        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    for (mfxU32 i = 0; i < NUM_TEST_PROP_PTR; i++) {
        sts = SetSingleProperty<mfxHDL>(loader, cfg, TestPropPtrTab[i].name, TestPropPtrTab[i].varType, (mfxHDL)TestPropPtrTab[i].validData);
        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

static mfxStatus TestAllPropsIntSingleInvalid(mfxU32 invalidIdx, bool bUseSingleConfig = false) {
    mfxStatus sts    = MFX_ERR_NONE;
    mfxLoader loader = nullptr;

    loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = nullptr;
    if (bUseSingleConfig) {
        cfg = MFXCreateConfig(loader);
        EXPECT_FALSE(cfg == nullptr);
    }

    for (mfxU32 i = 0; i < NUM_TEST_PROP_INT; i++) {
        mfxU64 data = (i == invalidIdx ? (mfxU64)TestPropIntTab[i].invalidData : (mfxU64)TestPropIntTab[i].validData);

        if (bUseSingleConfig)
            sts = SetSingleProperty<mfxU64>(loader, cfg, TestPropIntTab[i].name, TestPropIntTab[i].varType, data);
        else
            sts = SetSingleProperty<mfxU64>(loader, TestPropIntTab[i].name, TestPropIntTab[i].varType, data);

        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    // expect to fail with one invalid property
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);

    return sts;
}

static mfxStatus TestAllPropsPtrSingleInvalid(mfxU32 invalidIdx, bool bUseSingleConfig = false) {
    mfxStatus sts    = MFX_ERR_NONE;
    mfxLoader loader = nullptr;

    loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = nullptr;
    if (bUseSingleConfig) {
        cfg = MFXCreateConfig(loader);
        EXPECT_FALSE(cfg == nullptr);
    }

    for (mfxU32 i = 0; i < NUM_TEST_PROP_PTR; i++) {
        mfxHDL data = (i == invalidIdx ? (mfxHDL)TestPropPtrTab[i].invalidData : (mfxHDL)TestPropPtrTab[i].validData);

        if (bUseSingleConfig)
            sts = SetSingleProperty<mfxHDL>(loader, cfg, TestPropPtrTab[i].name, TestPropPtrTab[i].varType, data);
        else
            sts = SetSingleProperty<mfxHDL>(loader, TestPropPtrTab[i].name, TestPropPtrTab[i].varType, data);

        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    // expect to fail with one invalid property
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);

    return sts;
}

// set all properties to valid (INT), except set one to invalid
TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_00) {
    TestAllPropsIntSingleInvalid(0);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_01) {
    TestAllPropsIntSingleInvalid(1);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_02) {
    TestAllPropsIntSingleInvalid(2);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_03) {
    TestAllPropsIntSingleInvalid(3);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_04) {
    TestAllPropsIntSingleInvalid(4);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_05) {
    TestAllPropsIntSingleInvalid(5);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_06) {
    TestAllPropsIntSingleInvalid(6);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_07) {
    TestAllPropsIntSingleInvalid(7);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_08) {
    TestAllPropsIntSingleInvalid(8);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_09) {
    TestAllPropsIntSingleInvalid(9);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_10) {
    TestAllPropsIntSingleInvalid(10);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_11) {
    TestAllPropsIntSingleInvalid(11);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_12) {
    TestAllPropsIntSingleInvalid(12);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_13) {
    TestAllPropsIntSingleInvalid(13);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_14) {
    TestAllPropsIntSingleInvalid(14);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_15) {
    TestAllPropsIntSingleInvalid(15);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_16) {
    TestAllPropsIntSingleInvalid(16);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_17) {
    TestAllPropsIntSingleInvalid(17);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_18) {
    TestAllPropsIntSingleInvalid(18);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_19) {
    TestAllPropsIntSingleInvalid(19);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_20) {
    TestAllPropsIntSingleInvalid(20);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_21) {
    TestAllPropsIntSingleInvalid(21);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_22) {
    TestAllPropsIntSingleInvalid(22);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsInt_InvalidProp_23) {
    TestAllPropsIntSingleInvalid(23);
}

// set all properties to valid (PTR), except set one to invalid
TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_00) {
    TestAllPropsPtrSingleInvalid(0);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_01) {
    TestAllPropsPtrSingleInvalid(1);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_02) {
    TestAllPropsPtrSingleInvalid(2);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_03) {
    TestAllPropsPtrSingleInvalid(3);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_04) {
    TestAllPropsPtrSingleInvalid(4);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_05) {
    TestAllPropsPtrSingleInvalid(5);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_06) {
    TestAllPropsPtrSingleInvalid(6);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_07) {
    TestAllPropsPtrSingleInvalid(7);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_08) {
    TestAllPropsPtrSingleInvalid(8);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_09) {
    TestAllPropsPtrSingleInvalid(9);
}

TEST(Dispatcher_SW_MultiProp, MultiConfig_AllPropsPtr_InvalidProp_10) {
    TestAllPropsPtrSingleInvalid(10);
}

// repeat tests, but use same config object for all properties
TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_00) {
    TestAllPropsIntSingleInvalid(0, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_01) {
    TestAllPropsIntSingleInvalid(1, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_02) {
    TestAllPropsIntSingleInvalid(2, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_03) {
    TestAllPropsIntSingleInvalid(3, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_04) {
    TestAllPropsIntSingleInvalid(4, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_05) {
    TestAllPropsIntSingleInvalid(5, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_06) {
    TestAllPropsIntSingleInvalid(6, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_07) {
    TestAllPropsIntSingleInvalid(7, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_08) {
    TestAllPropsIntSingleInvalid(8, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_09) {
    TestAllPropsIntSingleInvalid(9, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_10) {
    TestAllPropsIntSingleInvalid(10, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_11) {
    TestAllPropsIntSingleInvalid(11, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_12) {
    TestAllPropsIntSingleInvalid(12, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_13) {
    TestAllPropsIntSingleInvalid(13, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_14) {
    TestAllPropsIntSingleInvalid(14, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_15) {
    TestAllPropsIntSingleInvalid(15, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_16) {
    TestAllPropsIntSingleInvalid(16, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_17) {
    TestAllPropsIntSingleInvalid(17, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_18) {
    TestAllPropsIntSingleInvalid(18, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_19) {
    TestAllPropsIntSingleInvalid(19, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_20) {
    TestAllPropsIntSingleInvalid(20, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_21) {
    TestAllPropsIntSingleInvalid(21, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_22) {
    TestAllPropsIntSingleInvalid(22, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsInt_InvalidProp_23) {
    TestAllPropsIntSingleInvalid(23, true);
}

// set all properties to valid (PTR), except set one to invalid
TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_00) {
    TestAllPropsPtrSingleInvalid(0, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_01) {
    TestAllPropsPtrSingleInvalid(1, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_02) {
    TestAllPropsPtrSingleInvalid(2, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_03) {
    TestAllPropsPtrSingleInvalid(3, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_04) {
    TestAllPropsPtrSingleInvalid(4, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_05) {
    TestAllPropsPtrSingleInvalid(5, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_06) {
    TestAllPropsPtrSingleInvalid(6, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_07) {
    TestAllPropsPtrSingleInvalid(7, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_08) {
    TestAllPropsPtrSingleInvalid(8, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_09) {
    TestAllPropsPtrSingleInvalid(9, true);
}

TEST(Dispatcher_SW_MultiProp, SingleConfig_AllPropsPtr_InvalidProp_10) {
    TestAllPropsPtrSingleInvalid(10, true);
}