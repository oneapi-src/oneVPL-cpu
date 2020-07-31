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
TEST(EncodeQueryIOSurf, PopulatedParamsInReturnsRequest) {}

TEST(EncodeQueryIOSurf, InvalidParamsReturnInvalidVideoParam) {}

TEST(EncodeQueryIOSurf, IncompatibleParamsReturnIncompatibleVideoParam) {}

TEST(EncodeQueryIOSurf, NullSessionReturnsInvalidHandle) {}

TEST(EncodeQueryIOSurf, NullParamsInReturnsErrNull) {}

//DecodeQueryIOSurf
TEST(DecodeQueryIOSurf, PopulatedParamsInReturnsRequest) {}

TEST(DecodeQueryIOSurf, InvalidParamsReturnInvalidVideoParam) {}

TEST(DecodeQueryIOSurf, IncompatibleParamsReturnIncompatibleVideoParam) {}

TEST(DecodeQueryIOSurf, NullSessionReturnsInvalidHandle) {}

TEST(DecodeQueryIOSurf, NullParamsInReturnsErrNull) {}

//VPPQueryIOSurf
TEST(VPPQueryIOSurf, PopulatedParamsInReturnsRequest) {}

TEST(VPPQueryIOSurf, InvalidParamsReturnInvalidVideoParam) {}

TEST(VPPQueryIOSurf, NullSessionReturnsInvalidHandle) {}

TEST(VPPQueryIOSurf, NullParamsInReturnsErrNull) {}
