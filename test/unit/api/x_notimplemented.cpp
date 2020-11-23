/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

// These optional functions for encode, decode, and VPP are not implemented
// in the CPU reference implementation

// MFXJoinSession not implemented
TEST(JoinSession, AlwaysReturnsNotImplemented) {
    mfxSession session1, session2;
    mfxVersion ver = {};
    mfxStatus sts  = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session1);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session2);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXJoinSession(session1, session2);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// MFXDisjoinSession not implemented
TEST(DisjoinSession, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDisjoinSession(session);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// MFXCloneSession not implemented
TEST(CloneSession, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session, session2;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXCloneSession(session, &session2);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// MFXSetPriority not implemented
TEST(SetPriority, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXSetPriority(session, MFX_PRIORITY_LOW);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// MFXGetPriority not implemented
TEST(GetPriority, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXGetPriority(session, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(GetEncodeStat, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoENCODE_GetEncodeStat(session, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(GetDecodeStat, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoDECODE_GetDecodeStat(session, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(GetVPPStat, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoVPP_GetVPPStat(session, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeSetSkipMode, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxSkipMode M = MFX_SKIPMODE_NOSKIP;
    sts           = MFXVideoDECODE_SetSkipMode(session, M);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeGetPayload, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoDECODE_GetPayload(session, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// API 2.1
TEST(MFXMemory_GetSurfaceForVPPOut, AlwaysReturnsNotImplemented) {
    mfxVersion ver = { 1, 2 };
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameSurface1 *vppSurfaceOut = nullptr;
    sts                             = MFXMemory_GetSurfaceForVPPOut(session, &vppSurfaceOut);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(MFXVideoDECODE_VPP_Init, AlwaysReturnsNotImplemented) {
    mfxVersion ver = { 1, 2 };
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam decode_par               = {};
    mfxVideoChannelParam *vpp_par_array[1] = {};

    sts = MFXVideoDECODE_VPP_Init(session, &decode_par, vpp_par_array, 1);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(MFXVideoDECODE_VPP_DecodeFrameAsync, AlwaysReturnsNotImplemented) {
    mfxVersion ver = { 1, 2 };
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxBitstream bs                 = {};
    mfxSurfaceArray *surf_array_out = nullptr;

    sts = MFXVideoDECODE_VPP_DecodeFrameAsync(session, &bs, nullptr, 0, &surf_array_out);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(MFXVideoDECODE_VPP_Reset, AlwaysReturnsNotImplemented) {
    mfxVersion ver = { 1, 2 };
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam decode_par               = {};
    mfxVideoChannelParam *vpp_par_array[1] = {};

    sts = MFXVideoDECODE_VPP_Reset(session, &decode_par, vpp_par_array, 1);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(MFXVideoDECODE_VPP_GetChannelParam, AlwaysReturnsNotImplemented) {
    mfxVersion ver = { 1, 2 };
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoChannelParam par = {};

    sts = MFXVideoDECODE_VPP_GetChannelParam(session, &par, 0);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(MFXVideoVPP_ProcessFrameAsync, AlwaysReturnsNotImplemented) {
    mfxVersion ver = { 1, 2 };
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameSurface1 in   = {};
    mfxFrameSurface1 *out = nullptr;

    sts = MFXVideoVPP_ProcessFrameAsync(session, &in, &out);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}
