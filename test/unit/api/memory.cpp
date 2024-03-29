/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "api/test_bitstreams.h"
#include "vpl/mfxjpeg.h"
#include "vpl/mfxsurfacepool.h"
#include "vpl/mfxvideo.h"

/*
   Memory functions have the same states
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_NULL_PTR If surface is NULL. \n
   MFX_ERR_INVALID_HANDLE If session was not initialized. \n
   MFX_ERR_NOT_INITIALIZED If VPP wasn't initialized
*/

static mfxStatus InitDecodeBasic(mfxSession *session) {
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

static mfxStatus InitEncodeBasic(mfxSession *session) {
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

    mfxEncParams.mfx.CodecId                 = MFX_CODEC_JPEG;
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

    // Required for JPEG Encode
    mfxEncParams.mfx.Interleaved     = 1;
    mfxEncParams.mfx.Quality         = 50;
    mfxEncParams.mfx.RestartInterval = 0;
    memset(&mfxEncParams.mfx.reserved5, 0, sizeof(mfxEncParams.mfx.reserved5));

    sts = MFXVideoENCODE_Init(*session, &mfxEncParams);
    if (sts)
        return sts;

    return sts;
}

static mfxStatus InitVPPBasic(mfxSession *session) {
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

static mfxStatus GetFrameDecodeBasic(mfxSession *session, mfxFrameSurface1 **decSurfaceIn) {
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

static void CloseDecodeBasic(mfxSession session) {
    MFXVideoDECODE_Close(session);
    MFXClose(session);
}

//GetSurfaceForVPPIn
TEST(Memory_GetSurfaceForVPPIn, InitializedVPPReturnsSurface) {
    mfxStatus sts;
    mfxSession session;

    // init VPP
    sts = InitVPPBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1 *vppSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForVPPIn(session, &vppSurfaceIn);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(vppSurfaceIn->Data.MemType,
              MFX_MEMTYPE_FROM_VPPIN | MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_INTERNAL_FRAME);

    vppSurfaceIn->FrameInterface->Release(vppSurfaceIn);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForVPPIn, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    // init VPP
    sts = InitVPPBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    sts = MFXMemory_GetSurfaceForVPPIn(session, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForVPPIn, NullSessionReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;

    // init VPP
    sts = InitVPPBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1 *vppSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForVPPIn(nullptr, &vppSurfaceIn);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForVPPIn, UninitializedVPPReturnsNotInitialized) {
    mfxStatus sts;
    mfxSession session;

    // init session only, not VPP
    mfxVersion ver = { 0, 2 };
    sts            = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1 *vppSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForVPPIn(session, &vppSurfaceIn);
    ASSERT_EQ(sts, MFX_ERR_NOT_INITIALIZED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//GetSurfaceForVPPOut
TEST(Memory_GetSurfaceForVPPOut, InitializedVPPReturnsSurface) {
    mfxStatus sts;
    mfxSession session;

    // init VPP
    sts = InitVPPBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1 *vppSurfaceOut = nullptr;
    sts                             = MFXMemory_GetSurfaceForVPPOut(session, &vppSurfaceOut);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(vppSurfaceOut->Data.MemType,
              MFX_MEMTYPE_FROM_VPPOUT | MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_INTERNAL_FRAME);

    vppSurfaceOut->FrameInterface->Release(vppSurfaceOut);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForVPPOut, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    // init VPP
    sts = InitVPPBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    sts = MFXMemory_GetSurfaceForVPPOut(session, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForVPPOut, NullSessionReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;

    // init VPP
    sts = InitVPPBasic(&session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1 *vppSurfaceOut = nullptr;
    sts                             = MFXMemory_GetSurfaceForVPPOut(nullptr, &vppSurfaceOut);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Memory_GetSurfaceForVPPOut, UninitializedVPPReturnsNotInitialized) {
    mfxStatus sts;
    mfxSession session;

    // init session only, not VPP
    mfxVersion ver = { 0, 2 };
    sts            = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1 *vppSurfaceOut = nullptr;
    sts                             = MFXMemory_GetSurfaceForVPPOut(session, &vppSurfaceOut);
    ASSERT_EQ(sts, MFX_ERR_NOT_INITIALIZED);

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
    ASSERT_EQ(sts, MFX_ERR_NONE);

    // get internally allocated frame
    mfxFrameSurface1 *encSurfaceIn = nullptr;
    sts                            = MFXMemory_GetSurfaceForEncode(session, &encSurfaceIn);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(encSurfaceIn->Data.MemType,
              MFX_MEMTYPE_FROM_ENC | MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_INTERNAL_FRAME);

    encSurfaceIn->FrameInterface->Release(encSurfaceIn);

    sts = MFXVideoENCODE_Close(session);

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
    mfxFrameSurface1 *encSurfaceIn = nullptr;
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
    mfxFrameSurface1 *encSurfaceIn = nullptr;
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
    mfxFrameSurface1 *decSurfaceIn = nullptr;
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
    mfxFrameSurface1 *decSurfaceIn = nullptr;
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
    mfxFrameSurface1 *decSurfaceIn = nullptr;
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

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength = mfxBS.DataLength = test_bitstream_96x64_8bit_hevc::getlen();
    mfxBS.Data                         = test_bitstream_96x64_8bit_hevc::getdata();
    mfxBS.CodecId                      = MFX_CODEC_HEVC;

    mfxFrameSurface1 *pmfxOutSurface = nullptr;
    mfxSyncPoint syncp               = { 0 };

    mfxBitstream *bsPtr = &mfxBS;
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
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->AddRef(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceAddRef, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->AddRef(nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceAddRef, NullHandleReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->AddRef(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    // free internal resources
    CloseDecodeBasic(session);
}

//Release
TEST(Memory_FrameInterfaceRelease, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Release(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceRelease, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Release(nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceRelease, NullHandleReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->Release(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceRelease, ZeroRefcountReturnsErrUndefinedBehavior) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Release(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // try to release again
    sts = pmfxWorkSurface->FrameInterface->Release(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_UNDEFINED_BEHAVIOR);

    // free internal resources
    CloseDecodeBasic(session);
}

//GetRefCounter
TEST(Memory_FrameInterfaceGetRefCounter, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    mfxU32 counter = 0;
    sts            = pmfxWorkSurface->FrameInterface->GetRefCounter(pmfxWorkSurface, &counter);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(counter, 1);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceGetRefCounter, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);

    sts = pmfxWorkSurface->FrameInterface->GetRefCounter(pmfxWorkSurface, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceGetRefCounter, NullHandleReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    mfxU32 counter                           = 0;
    sts = pmfxWorkSurface->FrameInterface->GetRefCounter(pmfxWorkSurface, &counter);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    // free internal resources
    CloseDecodeBasic(session);
}

//Map
TEST(Memory_FrameInterfaceMap, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceMap, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;

    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(nullptr, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceMap, NullHandleReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceMap, InvalidFlagValReturnsUnsupported) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, 0xffff);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceMap, WriteToWriteFlagSurfaceReturnsErrLock) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->Data.Locked = 1;
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_WRITE);
    EXPECT_EQ(sts, MFX_ERR_LOCK_MEMORY);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceMap, WriteToReadWriteFlagSurfaceReturnsErrLock) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->Data.Locked = 1;
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ_WRITE);
    EXPECT_EQ(sts, MFX_ERR_LOCK_MEMORY);

    // free internal resources
    CloseDecodeBasic(session);
}

//Unmap
TEST(Memory_FrameInterfaceUnmap, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = pmfxWorkSurface->FrameInterface->Unmap(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceUnmap, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = pmfxWorkSurface->FrameInterface->Unmap(nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceUnmap, NullHandleReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface, MFX_MAP_READ);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->Unmap(pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceUnmap, AlreadyUnmappedReturnsUnsupported) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

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

    // free internal resources
    CloseDecodeBasic(session);
}

//GetNativeHandle
TEST(Memory_FrameInterfaceGetNativeHandle, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxHDL resource                   = nullptr;
    mfxResourceType resource_type     = (mfxResourceType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetNativeHandle(nullptr, &resource, &resource_type);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceGetNativeHandle, NullResourceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxResourceType resource_type     = (mfxResourceType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts =
        pmfxWorkSurface->FrameInterface->GetNativeHandle(pmfxWorkSurface, nullptr, &resource_type);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceGetNativeHandle, NullResourceTypeReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxHDL resource                   = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetNativeHandle(pmfxWorkSurface, &resource, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceGetNativeHandle, SystemMemoryReturnsUnsupported) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxHDL resource                   = nullptr;
    mfxResourceType resource_type     = (mfxResourceType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetNativeHandle(pmfxWorkSurface,
                                                           &resource,
                                                           &resource_type);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceGetNativeHandle, NullSurfaceReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
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

    // free internal resources
    CloseDecodeBasic(session);
}

//GetDeviceHandle
TEST(Memory_FrameInterfaceGetDeviceHandle, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxHDL device_handle              = nullptr;
    mfxHandleType device_type         = (mfxHandleType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetDeviceHandle(nullptr, &device_handle, &device_type);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceGetDeviceHandle, NullHandleReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxHandleType device_type         = (mfxHandleType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetDeviceHandle(pmfxWorkSurface, nullptr, &device_type);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceGetDeviceHandle, NullDeviceTypeReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxHDL device_handle              = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts =
        pmfxWorkSurface->FrameInterface->GetDeviceHandle(pmfxWorkSurface, &device_handle, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceGetDeviceHandle, SystemMemoryReturnsUnsupported) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxHDL device_handle              = nullptr;
    mfxHandleType device_type         = (mfxHandleType)0;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->GetDeviceHandle(pmfxWorkSurface,
                                                           &device_handle,
                                                           &device_type);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    CloseDecodeBasic(session);
}

//GetDeviceHandle
TEST(Memory_FrameInterfaceSynchronize, ValidInputReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Synchronize(pmfxWorkSurface, 1000);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceSynchronize, NullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->Synchronize(nullptr, 1000);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceSynchronize, InvalidSurfaceReturnsInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    pmfxWorkSurface->FrameInterface->Context = nullptr;
    sts = pmfxWorkSurface->FrameInterface->Synchronize(pmfxWorkSurface, 1000);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    // free internal resources
    CloseDecodeBasic(session);
}

// QueryInterface tests - GUID = unknown

TEST(Memory_FrameInterfaceQueryInterface, GUIDUnknownReturnsErrNotImplemented) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = {};
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDUnknownNullSurfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = {};
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(nullptr, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDUnknownNullInterfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = {};

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

// QueryInterface tests - GUID = MFX_GUID_SURFACE_POOL

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolAddRefValidReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    sts = surfacePoolInterface->AddRef(surfacePoolInterface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolAddRefNullInterfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    sts = surfacePoolInterface->AddRef(nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolReleaseValidReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // initial QueryInterface call caused refcount++
    sts = surfacePoolInterface->Release(surfacePoolInterface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolReleaseNullInterfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    sts = surfacePoolInterface->Release(nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolReleaseTwiceReturnsErrInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    sts = surfacePoolInterface->Release(surfacePoolInterface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // call again - refcount already 0 so error (interface was destroyed)
    sts = surfacePoolInterface->Release(surfacePoolInterface);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolGetRefCounterValidReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // initial QueryInterface call caused refcount++
    mfxU32 counter = 0;
    sts            = surfacePoolInterface->GetRefCounter(surfacePoolInterface, &counter);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(counter, 1);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolGetRefCounterIncrementIsCorrect) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // initial QueryInterface call caused refcount++, then call again
    sts = surfacePoolInterface->AddRef(surfacePoolInterface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxU32 counter = 0;
    sts            = surfacePoolInterface->GetRefCounter(surfacePoolInterface, &counter);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(counter, 2);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolGetRefCounterIncDecIsCorrect) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // initial QueryInterface call caused refcount++, then call again, then call release
    sts = surfacePoolInterface->AddRef(surfacePoolInterface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = surfacePoolInterface->Release(surfacePoolInterface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxU32 counter = 0;
    sts            = surfacePoolInterface->GetRefCounter(surfacePoolInterface, &counter);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(counter, 1);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolGetRefCounterNullInterfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    mfxU32 counter = 0;
    sts            = surfacePoolInterface->GetRefCounter(nullptr, &counter);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolGetRefCounterNullCounterReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    sts = surfacePoolInterface->GetRefCounter(surfacePoolInterface, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface,
     GUIDSurfacePoolSetNumSurfacesValidReturnsWrnIncompatible) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // CPU RT only supports MFX_ALLOCATION_UNLIMITED, so this should return warning (input is ignored)
    sts = surfacePoolInterface->SetNumSurfaces(surfacePoolInterface, 5);
    EXPECT_EQ(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface,
     GUIDSurfacePoolSetNumSurfacesNullInterfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    sts = surfacePoolInterface->SetNumSurfaces(nullptr, 5);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface,
     GUIDSurfacePoolRevokeSurfacesValidReturnsWrnIncompatible) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // CPU RT only supports MFX_ALLOCATION_UNLIMITED, so this should return warning (input is ignored)
    sts = surfacePoolInterface->SetNumSurfaces(surfacePoolInterface, 5);
    EXPECT_EQ(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);

    // same for RevokeSurfaces
    sts = surfacePoolInterface->RevokeSurfaces(surfacePoolInterface, 5);
    EXPECT_EQ(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface,
     GUIDSurfacePoolRevokeSurfacesNullInterfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // CPU RT only supports MFX_ALLOCATION_UNLIMITED, so this should return warning (input is ignored)
    sts = surfacePoolInterface->SetNumSurfaces(surfacePoolInterface, 5);
    EXPECT_EQ(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);

    sts = surfacePoolInterface->RevokeSurfaces(nullptr, 5);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolGetAllocationPolicyValidReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // CPU RT only supports MFX_ALLOCATION_UNLIMITED
    mfxPoolAllocationPolicy policy = (mfxPoolAllocationPolicy)0xFFFFFFFF;
    sts = surfacePoolInterface->GetAllocationPolicy(surfacePoolInterface, &policy);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(policy, MFX_ALLOCATION_UNLIMITED);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface,
     GUIDSurfacePoolGetAllocationPolicyNullInterfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // CPU RT only supports MFX_ALLOCATION_UNLIMITED
    mfxPoolAllocationPolicy policy = (mfxPoolAllocationPolicy)0xFFFFFFFF;
    sts                            = surfacePoolInterface->GetAllocationPolicy(nullptr, &policy);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolGetMaximumPoolSizeValidReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // CPU RT only supports MFX_ALLOCATION_UNLIMITED
    mfxU32 size = 0;
    sts         = surfacePoolInterface->GetMaximumPoolSize(surfacePoolInterface, &size);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(size, 0xFFFFFFFF);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface,
     GUIDSurfacePoolGetMaximumPoolSizeNullInterfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // CPU RT only supports MFX_ALLOCATION_UNLIMITED
    mfxU32 size = 0;
    sts         = surfacePoolInterface->GetMaximumPoolSize(nullptr, &size);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePool_GetCurrentPoolSizeValidReturnsErrNone) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // by default CPU RT initializes surfaces pool of size NumFrameSuggested
    mfxVideoParam currParams = { 0 };
    sts                      = MFXVideoDECODE_GetVideoParam(session, &currParams);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request = {};
    sts                          = MFXVideoDECODE_QueryIOSurf(session, &currParams, &request);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get current pool size
    mfxU32 size = 0;
    sts         = surfacePoolInterface->GetCurrentPoolSize(surfacePoolInterface, &size);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(size, request.NumFrameSuggested);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface,
     GUIDSurfacePool_GetCurrentPoolNullInterfaceReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // by default CPU RT initializes surfaces pool of size NumFrameSuggested
    mfxVideoParam currParams = { 0 };
    sts                      = MFXVideoDECODE_GetVideoParam(session, &currParams);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request = {};
    sts                          = MFXVideoDECODE_QueryIOSurf(session, &currParams, &request);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get current pool size
    mfxU32 size = 0;
    sts         = surfacePoolInterface->GetCurrentPoolSize(nullptr, &size);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePool_GetCurrentPoolNullSizeReturnsErrNull) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // by default CPU RT initializes surfaces pool of size NumFrameSuggested
    mfxVideoParam currParams = { 0 };
    sts                      = MFXVideoDECODE_GetVideoParam(session, &currParams);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request = {};
    sts                          = MFXVideoDECODE_QueryIOSurf(session, &currParams, &request);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get current pool size
    sts = surfacePoolInterface->GetCurrentPoolSize(surfacePoolInterface, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePool_GetCurrentPoolSizeMaxSurfacesIsCorrect) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // by default CPU RT initializes surfaces pool of size NumFrameSuggested
    mfxVideoParam currParams = { 0 };
    sts                      = MFXVideoDECODE_GetVideoParam(session, &currParams);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request = {};
    sts                          = MFXVideoDECODE_QueryIOSurf(session, &currParams, &request);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get current pool size
    mfxU32 size = 0;
    sts         = surfacePoolInterface->GetCurrentPoolSize(surfacePoolInterface, &size);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(size, request.NumFrameSuggested);

    // use up all of the free surfaces (one was already requested in GetFrameDecodeBasic)
    for (mfxU16 i = 1; i < request.NumFrameSuggested; i++) {
        mfxFrameSurface1 *decSurfaceIn = nullptr;
        sts                            = MFXMemory_GetSurfaceForDecode(session, &decSurfaceIn);
    }

    // size should still be original pool size
    size = 0;
    sts  = surfacePoolInterface->GetCurrentPoolSize(surfacePoolInterface, &size);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(size, request.NumFrameSuggested);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface,
     GUIDSurfacePool_GetCurrentPoolSizeReqMoreSurfacesIsCorrect) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // by default CPU RT initializes surfaces pool of size NumFrameSuggested
    mfxVideoParam currParams = { 0 };
    sts                      = MFXVideoDECODE_GetVideoParam(session, &currParams);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request = {};
    sts                          = MFXVideoDECODE_QueryIOSurf(session, &currParams, &request);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get current pool size
    mfxU32 size = 0;
    sts         = surfacePoolInterface->GetCurrentPoolSize(surfacePoolInterface, &size);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(size, request.NumFrameSuggested);

    // use up all of the initial free surfaces and then add one more
    for (mfxU16 i = 1; i < request.NumFrameSuggested + 1; i++) {
        mfxFrameSurface1 *decSurfaceIn = nullptr;
        sts                            = MFXMemory_GetSurfaceForDecode(session, &decSurfaceIn);
    }

    // size should be original pool size + 1
    size = 0;
    sts  = surfacePoolInterface->GetCurrentPoolSize(surfacePoolInterface, &size);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(size, request.NumFrameSuggested + 1);

    // free internal resources
    CloseDecodeBasic(session);
}

TEST(Memory_FrameInterfaceQueryInterface, GUIDSurfacePoolCallsAfterReleaseReturnErrInvalidHandle) {
    mfxStatus sts;
    mfxSession session;
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxGUID guid                      = { MFX_GUID_SURFACE_POOL };
    mfxHDL interface;

    sts = GetFrameDecodeBasic(&session, &pmfxWorkSurface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    ASSERT_NE(nullptr, pmfxWorkSurface);
    sts = pmfxWorkSurface->FrameInterface->QueryInterface(pmfxWorkSurface, guid, &interface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // get pointer to surfacePoolInterface
    mfxSurfacePoolInterface *surfacePoolInterface =
        reinterpret_cast<mfxSurfacePoolInterface *>(interface);
    EXPECT_NE(surfacePoolInterface, nullptr);

    // initial QueryInterface call caused refcount++
    sts = surfacePoolInterface->Release(surfacePoolInterface);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // call each of the functions after release - should return MFX_ERR_INVALID_HANDLE
    //   because the interface has been destroyed
    sts = surfacePoolInterface->AddRef(surfacePoolInterface);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    sts = surfacePoolInterface->Release(surfacePoolInterface);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    mfxU32 counter = 0;
    sts            = surfacePoolInterface->GetRefCounter(surfacePoolInterface, &counter);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    sts = surfacePoolInterface->SetNumSurfaces(surfacePoolInterface, 5);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    sts = surfacePoolInterface->RevokeSurfaces(surfacePoolInterface, 5);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    mfxPoolAllocationPolicy policy = (mfxPoolAllocationPolicy)0xFFFFFFFF;
    sts = surfacePoolInterface->GetAllocationPolicy(surfacePoolInterface, &policy);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    mfxU32 size = 0;
    sts         = surfacePoolInterface->GetMaximumPoolSize(surfacePoolInterface, &size);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    sts = surfacePoolInterface->GetCurrentPoolSize(surfacePoolInterface, &size);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    // free internal resources
    CloseDecodeBasic(session);
}
