/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfximplcaps.h"
#include "vpl/mfxvideo.h"

// if linking directly against the runtime, 2.0 dispatcher
//   functions will not be available
#ifndef VPL_UTEST_LINK_RUNTIME

    #define VPL_UTEST_DISPATCHER_TYPE_SOFTWARE

//MFXLoad
TEST(Dispatcher_Load, CallReturnsLoader) {
    mfxLoader loader = MFXLoad();
    EXPECT_NE(loader, nullptr);

    //free internal resources
    MFXUnload(loader);
}

//MFXCreateConfig
TEST(Dispatcher_CreateConfig, InitializedLoaderReturnsConfig) {
    mfxLoader loader = MFXLoad();
    EXPECT_NE(loader, nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_NE(cfg, nullptr);

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

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

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

    sts = MFXSetConfigFilterProperty(nullptr, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

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

TEST(Dispatcher_SetConfigFilterProperty, UnknownParamReturnsNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Unknown", ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_SetConfigFilterProperty, ValueTypeMismatchReturnsErrUnsupported) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U8;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_SetConfigFilterProperty, PartialFilterReturnsErrNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = 0;
    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription", ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_SetConfigFilterProperty, OutOfRangeValueReturnsErrNone) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = 9999;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

    ASSERT_EQ(sts, MFX_ERR_NONE);

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

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

    ASSERT_EQ(sts, MFX_ERR_NONE);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    ASSERT_EQ(sts, MFX_ERR_NONE);

    // confirm correct Impl type was found
    ASSERT_EQ(implDesc->Impl, MFX_IMPL_TYPE_SOFTWARE);

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

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

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

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXEnumImplementations(loader, 0, MFX_IMPLCAPS_IMPLDESCSTRUCTURE, nullptr);
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

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

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

TEST(Dispatcher_CreateSession, UnusedCfgCreatesSession) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    // create mfxConfig but do not apply filter property
    // should still create session with default props
    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

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

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

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
    sts = MFXSetConfigFilterProperty(cfg1, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue1);
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
    sts = MFXSetConfigFilterProperty(cfg2, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue2);
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
        (const mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue1);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg2 = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg2 == nullptr);

    mfxVariant ImplValue2;
    ImplValue2.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue2.Data.U32 = MFX_CODEC_HEVC;
    sts                 = MFXSetConfigFilterProperty(
        cfg2,
        (const mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
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

    mfxStatus sts = MFXCreateSession(loader, 0, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, FilterNoMatchReturnsErrNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxVariant impl_value;
    mfxConfig cfg;

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    impl_value.Type     = MFX_VARIANT_TYPE_U32;
    impl_value.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                 = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     impl_value);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    impl_value.Type     = MFX_VARIANT_TYPE_U32;
    impl_value.Data.U32 = MFX_CODEC_MPEG2;
    sts                 = MFXSetConfigFilterProperty(
        cfg,
        reinterpret_cast<const mfxU8 *>("mfxImplDescription.mfxDecoderDescription.decoder.CodecID"),
        impl_value);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    ASSERT_EQ(sts, MFX_ERR_NOT_FOUND);

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

TEST(Dispatcher_CreateSession, RequestAV1DecCreatesSession) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;
    mfxConfig cfg;

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_AV1;

    sts = MFXSetConfigFilterProperty(
        cfg,
        (const mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
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

TEST(Dispatcher_CreateSession, RequestAV1EncCreatesSession) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;
    mfxConfig cfg;

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_AV1;

    sts = MFXSetConfigFilterProperty(
        cfg,
        (const mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
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

TEST(Dispatcher_CreateSession, RequestI010toI420VPPCreatesSession) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg;
    mfxStatus sts;
    mfxVariant ImplValue;

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg                = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_FOURCC_I010;
    sts                = MFXSetConfigFilterProperty(
        cfg,
        (const mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.memdesc.format.InFormat",
        ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg2     = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_FOURCC_I420;
    sts                = MFXSetConfigFilterProperty(
        cfg2,
        (const mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.memdesc.format.OutFormat",
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

TEST(Dispatcher_CreateSession, RequestVP9DecReturnsErrNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg;
    mfxStatus sts;
    mfxVariant ImplValue;

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_VP9;
    sts                = MFXSetConfigFilterProperty(
        cfg,
        (const mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestVP9EncReturnsErrNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg;
    mfxStatus sts;
    mfxVariant ImplValue;

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_VP9;

    sts = MFXSetConfigFilterProperty(
        cfg,
        (const mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
        ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestAV1AndHEVCDecReturnsErrNone) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg;
    mfxStatus sts;
    mfxVariant ImplValue;

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_AV1;
    sts                = MFXSetConfigFilterProperty(
        cfg,
        (const mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg2     = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_HEVC;
    sts                = MFXSetConfigFilterProperty(
        cfg2,
        (const mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXClose(session);
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestAV1AndVP9DecReturnsErrNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg;
    mfxStatus sts;
    mfxVariant ImplValue;

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_AV1;
    sts                = MFXSetConfigFilterProperty(
        cfg,
        (const mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg2     = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_VP9;
    sts                = MFXSetConfigFilterProperty(
        cfg2,
        (const mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestAccelNoneCreatesSession) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_ACCEL_MODE_NA;

    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.AccelerationMode",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestAccelD3D9ReturnsNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg;
    mfxStatus sts;
    mfxVariant ImplValue;

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_ACCEL_MODE_VIA_D3D9;

    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.AccelerationMode",
                                     ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestCurrentAPIVersionCreatesSession) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;

    // load CPU runtime
    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxVersion ver     = {};
    ver.Major          = MFX_VERSION_MAJOR;
    ver.Minor          = MFX_VERSION_MINOR;
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = ver.Version;

    sts =
        MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.ApiVersion", ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestLowerAPIVersionCreatesSession) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;

    // load CPU runtime
    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // request version (API - 0.1), which should pass
    mfxVersion ver     = {};
    ver.Major          = MFX_VERSION_MAJOR;
    ver.Minor          = MFX_VERSION_MINOR - 1;
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = ver.Version;

    sts =
        MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.ApiVersion", ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestHigherAPIVersionReturnsNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;

    // load CPU runtime
    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // request version (API + 0.1), which should fail
    mfxVersion ver     = {};
    ver.Major          = MFX_VERSION_MAJOR;
    ver.Minor          = MFX_VERSION_MINOR + 1;
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = ver.Version;

    sts =
        MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.ApiVersion", ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestImplementedFunctionCreatesSession) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;

    // load CPU runtime
    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // request an implemented function, which should pass
    ImplValue.Type     = mfxVariantType::MFX_VARIANT_TYPE_PTR;
    ImplValue.Data.Ptr = (mfxHDL) "MFXVideoDECODE_VPP_Init";
    sts                = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplementedFunctions.FunctionsName",
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, RequestNotImplementedFunctionReturnsNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;

    // load CPU runtime
    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // request an implemented function, which should pass
    ImplValue.Type     = mfxVariantType::MFX_VARIANT_TYPE_PTR;
    ImplValue.Data.Ptr = (mfxHDL) "MFXVideoDECODE_VPP_NOT_A_FUNCTION";
    sts                = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplementedFunctions.FunctionsName",
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_CreateSession, ConfigHandleReturnsHandle) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;
    mfxConfig cfg;

    mfxHDL testHandle = (mfxHDL)0xabcd1234eeff5566;
    #if defined(_WIN32) || defined(_WIN64)
    mfxHandleType testHandleType = MFX_HANDLE_D3D11_DEVICE;
    #else
    mfxHandleType testHandleType = MFX_HANDLE_VA_DISPLAY;
    #endif

    cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    sts                = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - mfxHandleType
    cfg                = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = testHandleType;
    sts                = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxHandleType", ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - mfxHDL
    cfg                = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_PTR;
    ImplValue.Data.Ptr = testHandle;
    sts                = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxHDL", ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // verify that handle was set properly
    mfxHDL returnHandle = nullptr;
    sts                 = MFXVideoCORE_GetHandle(session, testHandleType, &returnHandle);
    EXPECT_EQ(returnHandle, testHandle);

    //free internal resources
    MFXUnload(loader);
}

//MFXDispReleaseImplDescription
TEST(Dispatcher_DispReleaseImplDescription, ValidInputReturnsErrNone) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

    ASSERT_EQ(sts, MFX_ERR_NONE);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    ASSERT_EQ(sts, MFX_ERR_NONE);

    // confirm correct Impl type was found
    ASSERT_EQ(implDesc->Impl, MFX_IMPL_TYPE_SOFTWARE);

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
    mfxStatus sts = MFXEnumImplementations(loader,
                                           0,
                                           MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                           reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDispReleaseImplDescription(loader, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_DispReleaseImplDescription, HandleMismatchReturnsInvalidHandle) {
    mfxLoader loader1 = MFXLoad();
    EXPECT_FALSE(loader1 == nullptr);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc1;
    mfxStatus sts = MFXEnumImplementations(loader1,
                                           0,
                                           MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                           reinterpret_cast<mfxHDL *>(&implDesc1));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass invalid handle
    // Note: CPU RT now uses a read-only table for its implDesc, so multiple mfxLoader
    //   objects may return the same pointer to implDesc (not a sufficient test to create
    //   two loaders and release the handle from the wrong one)
    mfxImplDescription implDescLocal;
    sts = MFXDispReleaseImplDescription(loader1, &implDescLocal);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    //free internal resources
    MFXUnload(loader1);
}

TEST(Dispatcher_DispReleaseImplDescription, ReleaseTwiceReturnsErrNone) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

    ASSERT_EQ(sts, MFX_ERR_NONE);

    // first pass - Enum/CreateSession/DispRelease

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    ASSERT_EQ(sts, MFX_ERR_NONE);

    // confirm correct Impl type was found
    ASSERT_NE(implDesc, nullptr);
    ASSERT_EQ(implDesc->Impl, MFX_IMPL_TYPE_SOFTWARE);

    // create session
    mfxSession session1 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDispReleaseImplDescription(loader, implDesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // second pass - Enum/CreateSession/DispRelease
    // we are testing that calling EnumImplementations again on the same impl
    //   (after calling DispRelease) returns a valid implDesc and creates a new session

    // enumerate implementations, check capabilities of first one
    implDesc = nullptr;
    sts      = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    ASSERT_EQ(sts, MFX_ERR_NONE);

    // confirm correct Impl type was found
    ASSERT_NE(implDesc, nullptr);
    ASSERT_EQ(implDesc->Impl, MFX_IMPL_TYPE_SOFTWARE);

    // create session
    mfxSession session2 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDispReleaseImplDescription(loader, implDesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    MFXUnload(loader);
}

#endif // VPL_UTEST_LINK_RUNTIME
