/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "api/test_bitstreams.h"
#include "vpl/mfxvideo.h"

/*
   Memory functions have the same states
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_NULL_PTR If surface is NULL. \n
   MFX_ERR_INVALID_HANDLE If session was not initialized. \n
   MFX_ERR_NOT_INITIALIZED If VPP wasn't initialized
*/

#define VPL_UTEST_MEMORY_TYPE_SYSTEM

static mfxStatus InitDecodeBasic(mfxSession* session) {
    mfxVersion ver = {};
    ver.Major      = 2;
    ver.Minor      = 0;

    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, session);
    if (sts)
        return sts;

    // init decode
    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));

    mfxDecParams.IOPattern  = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    mfxDecParams.AsyncDepth = 1;

    mfxDecParams.mfx.CodecId                = MFX_CODEC_HEVC;
    mfxDecParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    mfxDecParams.mfx.FrameInfo.Width        = 352;
    mfxDecParams.mfx.FrameInfo.Height       = 288;
    mfxDecParams.mfx.FrameInfo.FourCC       = MFX_FOURCC_I420;

    sts = MFXVideoDECODE_Init(*session, &mfxDecParams);
    if (sts)
        return sts;

    return sts;
}

static mfxStatus InitEncodeBasic(mfxSession* session) {
    mfxVersion ver = {};
    ver.Major      = 2;
    ver.Minor      = 0;

    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, session);
    if (sts)
        return sts;

    // init encode
    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));

    mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    mfxEncParams.mfx.CodecId                 = MFX_CODEC_HEVC;
    mfxEncParams.mfx.TargetUsage             = 7;
    mfxEncParams.mfx.TargetKbps              = 4000;
    mfxEncParams.mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    mfxEncParams.mfx.GopPicSize              = 30;
    mfxEncParams.mfx.GopRefDist              = 2;
    mfxEncParams.mfx.FrameInfo.FrameRateExtN = 30;
    mfxEncParams.mfx.FrameInfo.FrameRateExtD = 1;
    mfxEncParams.mfx.FrameInfo.FourCC        = MFX_FOURCC_I420;
    mfxEncParams.mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxEncParams.mfx.FrameInfo.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxEncParams.mfx.FrameInfo.Width         = 352;
    mfxEncParams.mfx.FrameInfo.Height        = 288;

    mfxEncParams.mfx.FrameInfo.CropX = 0;
    mfxEncParams.mfx.FrameInfo.CropY = 0;
    mfxEncParams.mfx.FrameInfo.CropW = mfxEncParams.mfx.FrameInfo.Width;
    mfxEncParams.mfx.FrameInfo.CropH = mfxEncParams.mfx.FrameInfo.Height;

    sts = MFXVideoENCODE_Init(*session, &mfxEncParams);
    if (sts)
        return sts;

    return sts;
}

static mfxStatus InitVPPBasic(mfxSession* session) {
    mfxVersion ver = {};
    ver.Major      = 2;
    ver.Minor      = 0;

    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, session);
    if (sts)
        return sts;

    // init VPP
    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));
    mfxVPPParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxVPPParams.vpp.In.FourCC        = MFX_FOURCC_I420;
    mfxVPPParams.vpp.In.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxVPPParams.vpp.In.Width         = 352;
    mfxVPPParams.vpp.In.Height        = 288;
    mfxVPPParams.vpp.In.CropH         = mfxVPPParams.vpp.In.Height;
    mfxVPPParams.vpp.In.CropW         = mfxVPPParams.vpp.In.Width;
    mfxVPPParams.vpp.In.CropX         = 0;
    mfxVPPParams.vpp.In.CropY         = 0;
    mfxVPPParams.vpp.In.FrameRateExtN = 30;
    mfxVPPParams.vpp.In.FrameRateExtD = 1;

    mfxVPPParams.vpp.Out = mfxVPPParams.vpp.In;

    sts = MFXVideoVPP_Init(*session, &mfxVPPParams);
    if (sts)
        return sts;

    return sts;
}

static mfxStatus GetFrameDecodeBasic(mfxSession* session, mfxFrameSurface1** decSurfaceIn) {
    mfxStatus sts;

    // init decode
    sts = InitDecodeBasic(session);
    if (sts)
        return sts;

    // get internally allocated frame
    *decSurfaceIn = nullptr;
    sts           = MFXMemory_GetSurfaceForDecode(*session, decSurfaceIn);
    if (sts)
        return sts;

    return sts;
}

//GetSurfaceForVPP
TEST(Memory_GetSurfaceForVPP, InitializedVPPReturnsSurface) {
    mfxStatus sts;
    mfxSession session;

    // init VPP
    sts = InitVPPBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1* vppSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForVPP(session, &vppSurfaceIn);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(vppSurfaceIn->Data.MemType,
              MFX_MEMTYPE_FROM_VPPIN | MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_INTERNAL_FRAME);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForVPP, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    // init VPP
    sts = InitVPPBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    sts = MFXMemory_GetSurfaceForVPP(session, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForVPP, NullSessionReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;

    // init VPP
    sts = InitVPPBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1* vppSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForVPP(nullptr, &vppSurfaceIn);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForVPP, UninitializedVPPReturnsNotInitialized) {
    mfxStatus sts;
    mfxSession session;

    // init session only, not VPP
    mfxVersion ver = { 0, 2 };
    sts            = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1* vppSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForVPP(session, &vppSurfaceIn);
    EXPECT_EQ(sts, MFX_ERR_NOT_INITIALIZED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//GetSurfaceForEncode
TEST(Memory_GetSurfaceForEncode, InitializedEncodeReturnsSurface) {
    mfxStatus sts;
    mfxSession session;

    // init encode
    sts = InitEncodeBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1* encSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForEncode(session, &encSurfaceIn);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(encSurfaceIn->Data.MemType,
              MFX_MEMTYPE_FROM_ENC | MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_INTERNAL_FRAME);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForEncode, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    // init encode
    sts = InitEncodeBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    sts = MFXMemory_GetSurfaceForEncode(session, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForEncode, NullSessionReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;

    // init encode
    sts = InitEncodeBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1* encSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForEncode(nullptr, &encSurfaceIn);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForEncode, UninitializedEncodeReturnsNotInitialized) {
    mfxStatus sts;
    mfxSession session;

    // init session only, not encoder
    mfxVersion ver = { 0, 2 };
    sts            = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1* encSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForEncode(session, &encSurfaceIn);
    EXPECT_EQ(sts, MFX_ERR_NOT_INITIALIZED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//GetSurfaceForDecode
TEST(Memory_GetSurfaceForDecode, InitializedDecodeReturnsSurface) {
    mfxStatus sts;
    mfxSession session;

    // init decode
    sts = InitDecodeBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1* decSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForDecode(session, &decSurfaceIn);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(decSurfaceIn->Data.MemType,
              MFX_MEMTYPE_FROM_DECODE | MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_INTERNAL_FRAME);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForDecode, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    // init decode
    sts = InitDecodeBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    sts = MFXMemory_GetSurfaceForDecode(session, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForDecode, NullSessionReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;

    // init session only, not decoder
    mfxVersion ver = { 0, 2 };
    sts            = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1* decSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForDecode(nullptr, &decSurfaceIn);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForDecode, UninitializedDecodeReturnsNotInitialized) {
    mfxStatus sts;
    mfxSession session;

    // init decode
    sts = InitDecodeBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1* decSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForDecode(session, &decSurfaceIn);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//Simplified decode

TEST(Memory_FrameInterface, NoDecodeHeaderCanDecode) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams = { 0 };
    mfxDecParams.mfx.CodecId   = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern     = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength = mfxBS.DataLength = test_bitstream_96x64_8bit_hevc::getlen();
    mfxBS.Data                         = test_bitstream_96x64_8bit_hevc::getdata();
    mfxBS.CodecId                      = MFX_CODEC_HEVC;

    mfxFrameSurface1* pmfxOutSurface = nullptr;
    mfxSyncPoint syncp               = { 0 };

    mfxBitstream* bsPtr = &mfxBS;
    mfxU32 nFrames      = 0;
    while (1) {
        sts = MFXVideoDECODE_DecodeFrameAsync(session, bsPtr, nullptr, &pmfxOutSurface, &syncp);

        if (sts == MFX_ERR_NONE)
            nFrames++;

        if (bsPtr && (sts == MFX_ERR_MORE_DATA))
            bsPtr = nullptr;
        else if (sts != MFX_ERR_NONE)
            break;
    }

    // expect draining to have finished
    EXPECT_EQ(sts, MFX_ERR_MORE_DATA);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//AddRef
TEST(Memory_FrameInterfaceAddRef, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->AddRef(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_FrameInterfaceAddRef, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->AddRef(nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

TEST(Memory_FrameInterfaceAddRef, NullHandleReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->AddRef(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

//Release
TEST(Memory_FrameInterfaceRelease, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Release(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_FrameInterfaceRelease, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Release(nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

TEST(Memory_FrameInterfaceRelease, NullHandleReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->Release(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(Memory_FrameInterfaceRelease, ZeroRefcountReturnsErrUndefinedBehavior) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Release(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // try to release again
    sts = pmfxWorkSurface->FrameInterface->Release(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_UNDEFINED_BEHAVIOR);
}

//GetRefCounter
TEST(Memory_FrameInterfaceGetRefCounter, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    mfxU32 counter = 0;
    sts            = pmfxWorkSurface->FrameInterface->GetRefCounter(pmfxWorkSurface, &counter);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(counter, 1);
}

TEST(Memory_FrameInterfaceGetRefCounter, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);

    sts = pmfxWorkSurface->FrameInterface->GetRefCounter(pmfxWorkSurface, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

TEST(Memory_FrameInterfaceGetRefCounter, NullHandleReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    mfxU32 counter                           = 0;
    sts = pmfxWorkSurface->FrameInterface->GetRefCounter(pmfxWorkSurface, &counter);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

//Map
TEST(Memory_FrameInterfaceMap, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_FrameInterfaceMap, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(nullptr, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

TEST(Memory_FrameInterfaceMap, NullHandleReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(Memory_FrameInterfaceMap, InvalidFlagValReturnsUnsupported) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, 0xffff);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

TEST(Memory_FrameInterfaceMap, WriteToWriteFlagSurfaceReturnsErrLock) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->Data.Locked = 1;
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_WRITE);
    EXPECT_EQ(sts, MFX_ERR_LOCK_MEMORY);
}

TEST(Memory_FrameInterfaceMap, WriteToReadWriteFlagSurfaceReturnsErrLock) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->Data.Locked = 1;
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ_WRITE);
    EXPECT_EQ(sts, MFX_ERR_LOCK_MEMORY);
}

//Unmap
TEST(Memory_FrameInterfaceUnmap, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = pmfxWorkSurface->FrameInterface->Unmap(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_FrameInterfaceUnmap, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = pmfxWorkSurface->FrameInterface->Unmap(nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

TEST(Memory_FrameInterfaceUnmap, NullHandleReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->Unmap(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(Memory_FrameInterfaceUnmap, AlreadyUnmappedReturnsUnsupported) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = pmfxWorkSurface->FrameInterface->Unmap(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // try to unmap again
    sts = pmfxWorkSurface->FrameInterface->Unmap(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

//GetNativeHandle
TEST(Memory_FrameInterfaceGetNativeHandle, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxHDL resource                   = nullptr;
    mfxResourceType resource_type     = (mfxResourceType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetNativeHandle(nullptr, &resource, &resource_type);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

TEST(Memory_FrameInterfaceGetNativeHandle, NullResourceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxResourceType resource_type     = (mfxResourceType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts =
        pmfxWorkSurface->FrameInterface->GetNativeHandle(pmfxWorkSurface, nullptr, &resource_type);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

TEST(Memory_FrameInterfaceGetNativeHandle, NullResourceTypeReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxHDL resource                   = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetNativeHandle(pmfxWorkSurface, &resource, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

#ifdef VPL_UTEST_MEMORY_TYPE_SYSTEM

TEST(Memory_FrameInterfaceGetNativeHandle, SystemMemoryReturnsUnsupported) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxHDL resource                   = nullptr;
    mfxResourceType resource_type     = (mfxResourceType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetNativeHandle(pmfxWorkSurface,
                                                           &resource,
                                                           &resource_type);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

TEST(Memory_FrameInterfaceGetNativeHandle, NullSurfaceReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxHDL resource                   = nullptr;
    mfxResourceType resource_type     = (mfxResourceType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->GetNativeHandle(pmfxWorkSurface,
                                                           &resource,
                                                           &resource_type);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

#else

TEST(Memory_FrameInterfaceGetNativeHandle, ValidInputReturnsErrNone) {}

TEST(Memory_FrameInterfaceGetNativeHandle, DISABLED_NullSurfaceReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(Memory_FrameInterfaceGetNativeHandle, DISABLED_NullResourceReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(Memory_FrameInterfaceGetNativeHandle, DISABLED_NullResourceTypeReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

#endif

//GetDeviceHandle
TEST(Memory_FrameInterfaceGetDeviceHandle, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxHDL device_handle              = nullptr;
    mfxHandleType device_type         = (mfxHandleType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetDeviceHandle(nullptr, &device_handle, &device_type);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

TEST(Memory_FrameInterfaceGetDeviceHandle, NullHandleReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxHandleType device_type         = (mfxHandleType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetDeviceHandle(pmfxWorkSurface, nullptr, &device_type);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

TEST(Memory_FrameInterfaceGetDeviceHandle, NullDeviceTypeReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxHDL device_handle              = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts =
        pmfxWorkSurface->FrameInterface->GetDeviceHandle(pmfxWorkSurface, &device_handle, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

#ifdef VPL_UTEST_MEMORY_TYPE_SYSTEM

TEST(Memory_FrameInterfaceGetDeviceHandle, SystemMemoryReturnsUnsupported) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxHDL device_handle              = nullptr;
    mfxHandleType device_type         = (mfxHandleType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetDeviceHandle(pmfxWorkSurface,
                                                           &device_handle,
                                                           &device_type);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

#else

TEST(Memory_FrameInterfaceGetDeviceHandle, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(Memory_FrameInterfaceGetDeviceHandle, DISABLED_InvalidSurfaceReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(Memory_FrameInterfaceGetDeviceHandle, DISABLED_InvalidHandleReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(Memory_FrameInterfaceGetDeviceHandle, DISABLED_InvalidDeviceTypeReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(Memory_FrameInterfaceGetDeviceHandle, DISABLED_AsyncDependencyReturnsAborted) {
    FAIL() << "Test not implemented";
}
#endif

//GetDeviceHandle
TEST(Memory_FrameInterfaceSynchronize, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Synchronize(pmfxWorkSurface, 1000);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_FrameInterfaceSynchronize, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Synchronize(nullptr, 1000);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);
}

TEST(Memory_FrameInterfaceSynchronize, InvalidSurfaceReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1* pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->Synchronize(pmfxWorkSurface, 1000);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}
