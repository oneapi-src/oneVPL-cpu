/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

/* GetVideoParam overview
   Retrieves current working parameters to the specified output structure.

MFX_ERR_NONE The function completed successfully. 

*/

TEST(EncodeGetVideoParam, DISABLED_InitializedEncodeReturnsParams) {
    FAIL() << "Test not implemented";
}

TEST(EncodeGetVideoParam, DISABLED_EncodeUninitializedReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}

TEST(EncodeGetVideoParam, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(EncodeGetVideoParam, DISABLED_NullParamsOutReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(DecodeGetVideoParam, DISABLED_InitializedDecodeReturnsParams) {
    FAIL() << "Test not implemented";
}

TEST(DecodeGetVideoParam, DISABLED_DecodeUninitializedReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}

TEST(DecodeGetVideoParam, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(DecodeGetVideoParam, DISABLED_NullParamsOutReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(VPPGetVideoParam, DISABLED_InitializedVPPReturnsParams) {
    FAIL() << "Test not implemented";
}

TEST(VPPGetVideoParam, DISABLED_VPPUninitializedReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}

TEST(VPPGetVideoParam, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(VPPGetVideoParam, DISABLED_NullParamsOutReturnsErrNull) {
    FAIL() << "Test not implemented";
}
