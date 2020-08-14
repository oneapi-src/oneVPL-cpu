/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./vpl-common.h"

const char *MemoryModeString[MEM_MODE_COUNT] = { "MEM_MODE_EXTERNAL",
                                                 "MEM_MODE_INTERNAL",
                                                 "MEM_MODE_AUTO" };

const char *DispatcherModeString[DISPATCHER_MODE_COUNT] = {
    "DISPATCHER_MODE_LEGACY",
    "DISPATCHER_MODE_ONEVPL_20"
};

// check if this implementation can decode our stream
bool CheckImplCaps(mfxImplDescription *implDesc, mfxU32 codecID) {
    mfxU32 i;

    for (i = 0; i < implDesc->Dec.NumCodecs; i++) {
        mfxDecoderDescription::decoder *currDec = &(implDesc->Dec.Codecs[i]);

        if (currDec->CodecID == codecID) {
            return true;
        }
    }

    return false;
}

#define TEST_CFG(type, dType, val)                                           \
    cfg                  = MFXCreateConfig(loader);                          \
    ImplValue.Type       = (type);                                           \
    ImplValue.Data.dType = (val);                                            \
    sts                  = MFXSetConfigFilterProperty(cfg, name, ImplValue); \
    printf("Test config: sts = %d, name = %s\n", sts, name);

static void TestCfgPropsMain(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8 *)"mfxImplDescription.Impl";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_IMPL_TYPE_SOFTWARE);

    name = (const mfxU8 *)"mfxImplDescription.AccelerationMode";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 3);

    name = (const mfxU8 *)"mfxImplDescription.VendorID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 0xabcd);

    name = (const mfxU8 *)"mfxImplDescription.VendorImplID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 0x1234);
}

static void TestCfgPropsDec(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8
                *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_CODEC_HEVC);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.MaxcodecLevel";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 54);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 150);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.MemHandleType";
    TEST_CFG(MFX_VARIANT_TYPE_I32, I32, MFX_RESOURCE_SYSTEM_SURFACE);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.ColorFormats";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_FOURCC_I420);
}

static void TestCfgPropsEnc(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8
                *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_CODEC_HEVC);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.MaxcodecLevel";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 54);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.BiDirectionalPrediction";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 1);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.encprofile.Profile";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 150);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.MemHandleType";
    TEST_CFG(MFX_VARIANT_TYPE_I32, I32, MFX_RESOURCE_SYSTEM_SURFACE);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.ColorFormats";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_FOURCC_I420);
}

static void TestCfgPropsVPP(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8
                *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_CODEC_HEVC);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxVPPDescription.filter.MaxDelayInFrames";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 3);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxVPPDescription.filter.memdesc.MemHandleType";
    TEST_CFG(MFX_VARIANT_TYPE_I32, I32, MFX_RESOURCE_SYSTEM_SURFACE);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxVPPDescription.filter.memdesc.format.OutFormats";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_FOURCC_I420);
}

mfxStatus InitNewDispatcher(Params *params, mfxSession *session) {
    mfxStatus sts = MFX_ERR_NONE;
    *session      = nullptr;

    mfxLoader loader = MFXLoad();
    if (!loader) {
        printf("Error - MFXLoad() returned NULL\n");
        return MFX_ERR_UNSUPPORTED;
    }

    mfxVariant ImplValue;
    mfxConfig cfg;

    cfg                = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    MFXSetConfigFilterProperty(cfg,
                               (const mfxU8 *)"mfxImplDescription.Impl",
                               ImplValue);

    cfg                = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_HEVC;
    MFXSetConfigFilterProperty(
        cfg,
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue);

    mfxU32 implIdx = 0;
    while (1) {
        mfxImplDescription *implDesc;
        sts = MFXEnumImplementations(loader,
                                     implIdx,
                                     MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                     reinterpret_cast<mfxHDL *>(&implDesc));

        // out of range - we've tested all implementations
        if (sts == MFX_ERR_NOT_FOUND)
            break;

        if (CheckImplCaps(implDesc, params->srcFourCC) == true) {
            // this implementation is capable of decoding the input stream
            sts = MFXCreateSession(loader, implIdx, session);
            if (sts != MFX_ERR_NONE) {
                printf("Error in MFXCreateSession, sts = %d", sts);
                return sts;
            }
            MFXDispReleaseImplDescription(loader, implDesc);
            break;
        }
        else {
            MFXDispReleaseImplDescription(loader, implDesc);
        }

        implIdx++;
    }

    return sts;
}
