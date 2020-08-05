/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

/* Query function overview:
     If the input parameter set is a null pointer: T
     The function returns configurability info in the output parameter set. 
     Parameters set to non zero values are configurable with init.
     For decode and encode, the CodecID must be set in the output parameter set to identify the coding standard.

     If the in parameter set is non-zero, the function checks the validity of the fields in the input structure. 
     Corrected values are returned in the output structure. 
     The function will zero fields if there is insufficient information to determine validity or
     correction is not possible. 

     This feature can verify whether the SDK implementation supports certain profiles, levels or bitrates.
     For decode and encode, the CodecID must be set in the input parameter set to identify the coding standard.

     The application can call this function before or after it initializes the operation.

    Return states:
     MFX_ERR_NONE  The function completed successfully. \n
     MFX_ERR_UNSUPPORTED  The function failed to identify a specific implementation for the required features. \n
     MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others;
        incompatibility resolved.
*/

//EncodeQuery
TEST(EncodeQuery, DISABLED_NullParamsInReturnsConfigurable) {
    FAIL() << "Test not implemented";
}

TEST(EncodeQuery, DISABLED_PopulatedParamsInReturnsCorrected) {
    FAIL() << "Test not implemented";
}

TEST(EncodeQuery, DISABLED_UnsupportedParamsReturnUnsupported) {
    FAIL() << "Test not implemented";
}

TEST(EncodeQuery, DISABLED_IncompatibleParamsReturnIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(EncodeQuery, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(EncodeQuery, DISABLED_NullParamsOutReturnsErrNull) {
    FAIL() << "Test not implemented";
}

//DecodeQuery
TEST(DecodeQuery, DISABLED_NullParamsInReturnsConfigurable) {
    FAIL() << "Test not implemented";
}

TEST(DecodeQuery, DISABLED_PopulatedParamsInReturnsCorrected) {
    FAIL() << "Test not implemented";
}

TEST(DecodeQuery, DISABLED_UnsupportedParamsReturnUnsupported) {
    FAIL() << "Test not implemented";
}

TEST(DecodeQuery, DISABLED_IncompatibleParamsReturnIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(DecodeQuery, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(DecodeQuery, DISABLED_NullParamsOutReturnsErrNull) {
    FAIL() << "Test not implemented";
}

//VPPQuery
TEST(VPPQuery, DISABLED_NullParamsInReturnsConfigurable) {
    FAIL() << "Test not implemented";
}

TEST(VPPQuery, DISABLED_PopulatedParamsInReturnsCorrected) {
    FAIL() << "Test not implemented";
}

TEST(VPPQuery, DISABLED_UnsupportedParamsReturnUnsupported) {
    FAIL() << "Test not implemented";
}

TEST(VPPQuery, DISABLED_IncompatibleParamsReturnIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(VPPQuery, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(VPPQuery, DISABLED_NullParamsOutReturnsErrNull) {
    FAIL() << "Test not implemented";
}
