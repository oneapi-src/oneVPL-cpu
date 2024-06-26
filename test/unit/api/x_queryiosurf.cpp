/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

/* QueryIOSurf Overview

   For legacy external allocation.  Returns minimum and suggested numbers
       for encode init
       for decode output
       for vpp input and output
   This function does not validate I/O parameters except those used in calculating the number of surfaces.


   Returns:
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. These parameters may be out of the valid range, or the combination of them 
                                resulted in incompatibility. Incompatibility not resolved. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.

*/

//EncodeQueryIOSurf
TEST(EncodeQueryIOSurf, PopulatedParamsInReturnsRequest) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));

    mfxEncParams.mfx.CodecId          = MFX_CODEC_HEVC;
    mfxEncParams.mfx.FrameInfo.Width  = 320;
    mfxEncParams.mfx.FrameInfo.Height = 240;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoENCODE_Query(session, &mfxEncParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request;
    sts = MFXVideoENCODE_QueryIOSurf(session, &par, &request);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_GE(request.NumFrameSuggested, 1);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(EncodeQueryIOSurf, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoENCODE_QueryIOSurf(0, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(EncodeQueryIOSurf, NullParamsInReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest R;
    sts = MFXVideoENCODE_QueryIOSurf(session, nullptr, &R);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(EncodeQueryIOSurf, NullRequestReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));

    mfxEncParams.mfx.CodecId          = MFX_CODEC_HEVC;
    mfxEncParams.mfx.FrameInfo.Width  = 320;
    mfxEncParams.mfx.FrameInfo.Height = 240;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoENCODE_Query(session, &mfxEncParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoENCODE_QueryIOSurf(session, &par, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//DecodeQueryIOSurf
TEST(DecodeQueryIOSurf, PopulatedParamsInReturnsRequest) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));

    mfxEncParams.mfx.CodecId          = MFX_CODEC_HEVC;
    mfxEncParams.mfx.FrameInfo.Width  = 320;
    mfxEncParams.mfx.FrameInfo.Height = 240;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoDECODE_Query(session, &mfxEncParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request;
    sts = MFXVideoDECODE_QueryIOSurf(session, &par, &request);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_GE(request.NumFrameSuggested, 1);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeQueryIOSurf, InvalidParamsReturnInvalidVideoParam) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    par.mfx.CodecId          = MFX_CODEC_HEVC;
    par.mfx.FrameInfo.Width  = 320;
    par.mfx.FrameInfo.Height = 240;
    par.IOPattern            = MFX_IOPATTERN_OUT_VIDEO_MEMORY;

    mfxFrameAllocRequest request;
    sts = MFXVideoDECODE_QueryIOSurf(session, &par, &request);
    ASSERT_EQ(sts, MFX_ERR_INVALID_VIDEO_PARAM);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeQueryIOSurf, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoDECODE_QueryIOSurf(0, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(DecodeQueryIOSurf, NullParamsInReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest R;
    sts = MFXVideoDECODE_QueryIOSurf(session, nullptr, &R);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeQueryIOSurf, NullRequestReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));

    mfxEncParams.mfx.CodecId          = MFX_CODEC_HEVC;
    mfxEncParams.mfx.FrameInfo.Width  = 320;
    mfxEncParams.mfx.FrameInfo.Height = 240;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoDECODE_Query(session, &mfxEncParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoDECODE_QueryIOSurf(session, &par, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//VPPQueryIOSurf
TEST(VPPQueryIOSurf, PopulatedParamsInReturnsRequest) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));

    mfxVPPParams.mfx.CodecId          = MFX_CODEC_HEVC;
    mfxVPPParams.mfx.FrameInfo.Width  = 320;
    mfxVPPParams.mfx.FrameInfo.Height = 240;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoVPP_Query(session, &mfxVPPParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request[2] = { 0 };
    sts                             = MFXVideoVPP_QueryIOSurf(session, &par, request);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_GE(request[0].NumFrameSuggested, 1);
    ASSERT_GE(request[1].NumFrameSuggested, 1);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(VPPQueryIOSurf, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoVPP_QueryIOSurf(0, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(VPPQueryIOSurf, NullParamsInReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest R;
    sts = MFXVideoVPP_QueryIOSurf(session, nullptr, &R);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(VPPQueryIOSurf, NullRequestReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));

    mfxVPPParams.mfx.CodecId          = MFX_CODEC_HEVC;
    mfxVPPParams.mfx.FrameInfo.Width  = 320;
    mfxVPPParams.mfx.FrameInfo.Height = 240;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoVPP_Query(session, &mfxVPPParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoVPP_QueryIOSurf(session, &par, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}
