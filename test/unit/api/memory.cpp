/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

/*
   Memory functions have the same states
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_NULL_PTR If surface is NULL. \n
   MFX_ERR_INVALID_HANDLE If session was not initialized. \n
   MFX_ERR_NOT_INITIALIZED If VPP wasn't initialized
*/

//GetSurfaceForVPP
TEST(GetSurfaceForVPP, DISABLED_InitializedVPPReturnsSurface) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForVPP, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForVPP, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForVPP, DISABLED_UninitializedVPPReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}

//GetSurfaceForEncode
TEST(GetSurfaceForEncode, DISABLED_InitializedEncodeReturnsSurface) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForEncode, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForEncode, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForEncode, DISABLED_UninitializedEncodeReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}

//GetSurfaceForDecode
TEST(GetSurfaceForDecode, DISABLED_InitializedDecodeReturnsSurface) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForDecode, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForDecode, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForDecode, DISABLED_UninitializedDecodeReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}
