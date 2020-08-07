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
TEST(EncodeQueryIOSurf, DISABLED_PopulatedParamsInReturnsRequest) {
    FAIL() << "Test not implemented";
}

TEST(EncodeQueryIOSurf, DISABLED_InvalidParamsReturnInvalidVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(EncodeQueryIOSurf,
     DISABLED_IncompatibleParamsReturnIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
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

//DecodeQueryIOSurf
TEST(DecodeQueryIOSurf, DISABLED_PopulatedParamsInReturnsRequest) {
    FAIL() << "Test not implemented";
}

TEST(DecodeQueryIOSurf, DISABLED_InvalidParamsReturnInvalidVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(DecodeQueryIOSurf,
     DISABLED_IncompatibleParamsReturnIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
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

//VPPQueryIOSurf
TEST(VPPQueryIOSurf, DISABLED_PopulatedParamsInReturnsRequest) {
    FAIL() << "Test not implemented";
}

TEST(VPPQueryIOSurf, DISABLED_InvalidParamsReturnInvalidVideoParam) {
    FAIL() << "Test not implemented";
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
