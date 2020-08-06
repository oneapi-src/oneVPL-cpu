/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

/* Init overview
    This function allocates memory and prepares structures for encode, decode, and VPP.
    Parameters are validated to ensure they are supported. 

   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. 
           These parameters may be out of the valid range, or the combination resulted in incompatibility. 
           Incompatibility not resolved. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; 
           incompatibility resolved. \n
   MFX_ERR_UNDEFINED_BEHAVIOR  The function is called twice without a close;
*/

//EncodeInit
TEST(EncodeInit, DISABLED_ValidParamsInReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(EncodeInit, DISABLED_InvalidParamsInReturnsInvalidVideoParam) {
    FAIL() << "Test not implemented";
}

//profile/level incompatible with resolution
TEST(EncodeInit, DISABLED_IncompatibleParamsInReturnsIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(EncodeInit, DISABLED_NullParamsInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(EncodeInit, DISABLED_NullSessionInReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(EncodeInit, DISABLED_DoubleInitReturnsUndefinedBehavior) {
    FAIL() << "Test not implemented";
}

//DecodeInit
TEST(DecodeInit, DISABLED_ValidParamsInReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(DecodeInit, DISABLED_InvalidParamsInReturnsInvalidVideoParam) {
    FAIL() << "Test not implemented";
}

//profile/level incompatible with resolution
TEST(DecodeInit, DISABLED_IncompatibleParamsInReturnsIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(DecodeInit, DISABLED_NullParamsInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(DecodeInit, DISABLED_NullSessionInReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(DecodeInit, DISABLED_DoubleInitReturnsUndefinedBehavior) {
    FAIL() << "Test not implemented";
}

//VPPInit
TEST(VPPInit, DISABLED_ValidParamsInReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(VPPInit, DISABLED_InvalidParamsInReturnsInvalidVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(VPPInit, DISABLED_NullParamsInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(VPPInit, DISABLED_NullSessionInReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(VPPInit, DISABLED_DoubleInitReturnsUndefinedBehavior) {
    FAIL() << "Test not implemented";
}

/* Reset overview

    This function stops the current operation and restores internal structures for a new operation, 
    possibly with new parameters.
    
    For decode:
     * It recovers the decoder from errors.
    * It restarts decoding from a new position
    The function resets the old sequence header (sequence parameter set in H.264, or sequence header in MPEG-2 and VC-1). The decoder will expect a new sequence header 
    before it decodes the next frame and will skip any bitstream before encountering the new sequence header.

   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. These parameters may be out of the valid range, or the combination of them
                                resulted in incompatibility. Incompatibility not resolved. \n

   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.

mfxStatus MFX_CDECL MFXVideoENCODE_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoDECODE_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoVPP_Reset(mfxSession session, mfxVideoParam *par);
*/

//EncodeReset
TEST(EncodeReset, DISABLED_ValidParamsInReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(EncodeReset, DISABLED_InvalidParamsInReturnsInvalidVideoParam) {
    FAIL() << "Test not implemented";
}

//profile/level incompatible with resolution
TEST(EncodeReset, DISABLED_IncompatibleParamsInReturnsIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(EncodeReset, DISABLED_NullParamsInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(EncodeReset, DISABLED_NullSessionInReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(EncodeReset, DISABLED_UninitializedEncodeReturnsErrNotInitialized) {
    FAIL() << "Test not implemented";
}

//DecodeReset
TEST(DecodeReset, DISABLED_ValidParamsInReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(DecodeReset, DISABLED_InvalidParamsInReturnsInvalidVideoParam) {
    FAIL() << "Test not implemented";
}

//profile/level incompatible with resolution
TEST(DecodeReset, DISABLED_IncompatibleParamsInReturnsIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(DecodeReset, DISABLED_NullParamsInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(DecodeReset, DISABLED_NullSessionInReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(DecodeReset, DISABLED_UninitializedDecodeReturnsErrNotInitialized) {
    FAIL() << "Test not implemented";
}

//VPPReset
TEST(VPPReset, DISABLED_ValidParamsInReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(VPPReset, DISABLED_InvalidParamsInReturnsInvalidVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(VPPReset, DISABLED_NullParamsInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(VPPReset, DISABLED_NullSessionInReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(VPPReset, DISABLED_UninitializedEncodeReturnsErrNotInitialized) {
    FAIL() << "Test not implemented";
}

/* Close overview

Terminates current operation and de-allocates any internal memory/structures.
MFX_ERR_NONE  The function completed successfully.

*/

TEST(EncodeClose, DISABLED_InitializedEncodeReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(EncodeClose, DISABLED_UninitializedEncodeReturnsErrNotInitialized) {
    FAIL() << "Test not implemented";
}

TEST(EncodeClose, DISABLED_NullSessionInReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(EncodeClose, DISABLED_DoubleCloseReturnsErrNotInitialized) {
    FAIL() << "Test not implemented";
}

TEST(DecodeClose, DISABLED_InitializedEncodeReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(DecodeClose, DISABLED_UninitializedEncodeReturnsErrNotInitialized) {
    FAIL() << "Test not implemented";
}

TEST(DecodeClose, DISABLED_NullSessionInReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(DecodeClose, DISABLED_DoubleCloseReturnsErrNotInitialized) {
    FAIL() << "Test not implemented";
}

TEST(VPPClose, DISABLED_InitializedEncodeReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(VPPClose, DISABLED_UninitializedEncodeReturnsErrNotInitialized) {
    FAIL() << "Test not implemented";
}

TEST(VPPClose, DISABLED_NullSessionInReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(VPPClose, DISABLED_DoubleCloseReturnsErrNotInitialized) {
    FAIL() << "Test not implemented";
}