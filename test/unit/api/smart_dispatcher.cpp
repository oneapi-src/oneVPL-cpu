/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfximplcaps.h"
#include "vpl/mfxvideo.h"

#define VPL_UTEST_DISPATCHER_TYPE_SOFTWARE

//MFXLoad
TEST(Dispatcher_Load, CallReturnsLoader) {
    mfxLoader loader = MFXLoad();
    EXPECT_NE(loader, nullptr);

    //free internal resources
    MFXUnload(loader);
}

// TO DO: modify test environment such that no valid DLL's
//   will be found in dispatcher search paths (see spec)
TEST(Dispatcher_Load, DISABLED_NoImplementationDllsReturnsNullLoader) {
    FAIL() << "Test not implemented";
}

//MFXCreateConfig
TEST(Dispatcher_CreateConfig, InitializedLoaderReturnsConfig) {
    mfxLoader loader = MFXLoad();
    EXPECT_NE(loader, nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_NE(loader, nullptr);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateConfig, NullLoaderReturnsErrNull) {
    mfxConfig cfg = MFXCreateConfig(nullptr);
    EXPECT_EQ(cfg, nullptr);
}

//MFXSetConfigFilterProperty
TEST(Dispatcher_SetConfigFilterProperty, VPLImplInReturnsErrNone) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_SetConfigFilterProperty, NullConfigReturnsErrNull) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(nullptr,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_SetConfigFilterProperty, NullNameReturnsErrNull) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg, nullptr, ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_SetConfigFilterProptery, UnknownParamReturnsNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts =
        MFXSetConfigFilterProperty(cfg,
                                   (const mfxU8 *)"mfxImplDescription.Unknown",
                                   ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_SetConfigFilterProptery,
     ValueTypeMismatchReturnsErrUnsupported) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U8;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    //free internal resources
    MFXUnload(loader);
}

//MFXEnumImplementations
TEST(Dispatcher_EnumImplementations, ValidInputsReturnValidDesc) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // confirm correct Impl type was found
    EXPECT_EQ(implDesc->Impl, MFX_IMPL_TYPE_SOFTWARE);

    sts = MFXDispReleaseImplDescription(loader, implDesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_EnumImplementations, NullLoaderReturnsErrNull) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(nullptr,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_EnumImplementations, NullDescReturnsErrNull) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_EnumImplementations, IndexOutOfRangeReturnsNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 999999,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

//MFXCreateSession
TEST(Dispatcher_CreateSession, SimpleConfigCanCreateSession) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    // create session with first implementation
    mfxSession session = nullptr;
    mfxStatus sts      = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestSWImplCreatesSession) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, DoubleLoadersCreatesTwoSWSessions) {
    mfxStatus sts;

    // first loader/session
    mfxLoader loader1 = MFXLoad();
    EXPECT_FALSE(loader1 == nullptr);

    mfxConfig cfg1 = MFXCreateConfig(loader1);
    EXPECT_FALSE(cfg1 == nullptr);

    mfxVariant ImplValue1;
    ImplValue1.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue1.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                 = MFXSetConfigFilterProperty(cfg1,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue1);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession session1 = nullptr;
    sts                 = MFXCreateSession(loader1, 0, &session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session1, nullptr);

    // second loader/session
    mfxLoader loader2 = MFXLoad();
    EXPECT_FALSE(loader2 == nullptr);

    mfxConfig cfg2 = MFXCreateConfig(loader2);
    EXPECT_FALSE(cfg2 == nullptr);

    mfxVariant ImplValue2;
    ImplValue2.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue2.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                 = MFXSetConfigFilterProperty(cfg2,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession session2 = nullptr;
    sts                 = MFXCreateSession(loader2, 0, &session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session2, nullptr);

    // teardown
    sts = MFXClose(session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    sts = MFXClose(session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader1);
    MFXUnload(loader2);
}

TEST(Dispatcher_CreateSession, DoubleConfigObjsCreatesTwoSessions) {
    mfxStatus sts;

    // first loader/session
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg1 = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg1 == nullptr);

    mfxVariant ImplValue1;
    ImplValue1.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue1.Data.U32 = MFX_CODEC_AVC;
    sts                 = MFXSetConfigFilterProperty(
        cfg1,
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue1);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg2 = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg2 == nullptr);

    mfxVariant ImplValue2;
    ImplValue2.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue2.Data.U32 = MFX_CODEC_HEVC;
    sts                 = MFXSetConfigFilterProperty(
        cfg2,
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create two sessions
    mfxSession session1 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session1, nullptr);

    mfxSession session2 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session2, nullptr);

    // teardown
    sts = MFXClose(session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    sts = MFXClose(session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, NullLoaderReturnsErrNull) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxSession session = nullptr;
    mfxStatus sts      = MFXCreateSession(nullptr, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, NullSessionReturnsErrNull) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxSession session = nullptr;
    mfxStatus sts      = MFXCreateSession(loader, 0, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, InvalidIndexReturnsErrNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxSession session = nullptr;
    mfxStatus sts      = MFXCreateSession(loader, 999999, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

//MFXDispReleaseImplDescription
TEST(Dispatcher_DispReleaseImplDescription, ValidInputReturnsErrNone) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.Impl",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // confirm correct Impl type was found
    EXPECT_EQ(implDesc->Impl, MFX_IMPL_TYPE_SOFTWARE);

    sts = MFXDispReleaseImplDescription(loader, implDesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_DispReleaseImplDescription, NullLoaderReturnsErrNull) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDispReleaseImplDescription(nullptr, implDesc);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_DispReleaseImplDescription, NullDescReturnsErrNull) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    mfxStatus sts =
        MFXEnumImplementations(loader,
                               0,
                               MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                               reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDispReleaseImplDescription(loader, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_DispReleaseImplDescription,
     HandleMismatchReturnsInvalidHandle) {
    // create 2 loaders
    mfxLoader loader1 = MFXLoad();
    EXPECT_FALSE(loader1 == nullptr);

    mfxLoader loader2 = MFXLoad();
    EXPECT_FALSE(loader2 == nullptr);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc1;
    mfxStatus sts =
        MFXEnumImplementations(loader1,
                               0,
                               MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                               reinterpret_cast<mfxHDL *>(&implDesc1));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass wrong loader for this handle
    sts = MFXDispReleaseImplDescription(loader2, implDesc1);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    //free internal resources
    MFXUnload(loader1);
    MFXUnload(loader2);
}
