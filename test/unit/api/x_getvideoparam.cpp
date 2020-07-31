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

TEST(EncodeGetVideoParam, InitializedEncodeReturnsParams) {}

TEST(EncodeGetVideoParam, UninitializedEncodeReturnsNullCodec) {}

TEST(EncodeGetVideoParam, NullSessionReturnsInvalidHandle) {}

TEST(EncodeGetVideoParam, NullParamsOutReturnsErrNull) {}

TEST(DecodeGetVideoParam, InitializedDecodeReturnsParams) {}

TEST(DecodeGetVideoParam, UninitializedDecodeReturnsNullCodec) {}

TEST(DecodeGetVideoParam, NullSessionReturnsInvalidHandle) {}

TEST(DecodeGetVideoParam, NullParamsOutReturnsErrNull) {}

TEST(VPPGetVideoParam, InitializedVPPReturnsParams) {}

TEST(VPPGetVideoParam, UninitializedVPPReturnsNullFourCC) {}

TEST(VPPGetVideoParam, NullSessionReturnsInvalidHandle) {}

TEST(VPPGetVideoParam, NullParamsOutReturnsErrNull) {}