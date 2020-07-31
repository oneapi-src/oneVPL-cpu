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
TEST(GetSurfaceForVPP, InitializedVPPReturnsSurface) {}

TEST(GetSurfaceForVPP, NullSurfaceReturnsErrNull) {}

TEST(GetSurfaceForVPP, NullSessionReturnsInvalidHandle) {}

TEST(GetSurfaceForVPP, UninitializedVPPReturnsNotInitialized) {}

//GetSurfaceForEncode
TEST(GetSurfaceForEncode, InitializedEncodeReturnsSurface) {}

TEST(GetSurfaceForEncode, NullSurfaceReturnsErrNull) {}

TEST(GetSurfaceForEncode, NullSessionReturnsInvalidHandle) {}

TEST(GetSurfaceForEncode, UninitializedEncodeReturnsNotInitialized) {}

//GetSurfaceForDecode
TEST(GetSurfaceForDecode, InitializedDecodeReturnsSurface) {}

TEST(GetSurfaceForDecode, NullSurfaceReturnsErrNull) {}

TEST(GetSurfaceForDecode, NullSessionReturnsInvalidHandle) {}

TEST(GetSurfaceForDecode, UninitializedDecodeReturnsNotInitialized) {}
