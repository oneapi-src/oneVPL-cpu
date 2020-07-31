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
TEST(EncodeInit, ValidParamsInReturnsErrNone) {}

TEST(EncodeInit, InvalidParamsInReturnsInvalidVideoParam) {}

//profile/level incompatible with resolution
TEST(EncodeInit, IncompatibleParamsInReturnsIncompatibleVideoParam) {}

TEST(EncodeInit, NullParamsInReturnsErrNull) {}

TEST(EncodeInit, NullSessionInReturnsInvalidHandle) {}

TEST(EncodeInit, DoubleInitReturnsUndefinedBehavior) {}

//DecodeInit
TEST(DecodeInit, ValidParamsInReturnsErrNone) {}

TEST(DecodeInit, InvalidParamsInReturnsInvalidVideoParam) {}

//profile/level incompatible with resolution
TEST(DecodeInit, IncompatibleParamsInReturnsIncompatibleVideoParam) {}

TEST(DecodeInit, NullParamsInReturnsErrNull) {}

TEST(DecodeInit, NullSessionInReturnsInvalidHandle) {}

TEST(DecodeInit, DoubleInitReturnsUndefinedBehavior) {}

//VPPInit
TEST(VPPInit, ValidParamsInReturnsErrNone) {}

TEST(VPPInit, InvalidParamsInReturnsInvalidVideoParam) {}

TEST(VPPInit, NullParamsInReturnsErrNull) {}

TEST(VPPInit, NullSessionInReturnsInvalidHandle) {}

TEST(VPPInit, DoubleInitReturnsUndefinedBehavior) {}

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
TEST(EncodeReset, ValidParamsInReturnsErrNone) {}

TEST(EncodeReset, InvalidParamsInReturnsInvalidVideoParam) {}

//profile/level incompatible with resolution
TEST(EncodeReset, IncompatibleParamsInReturnsIncompatibleVideoParam) {}

TEST(EncodeReset, NullParamsInReturnsErrNull) {}

TEST(EncodeReset, NullSessionInReturnsInvalidHandle) {}

TEST(EncodeReset, UninitializedEncodeReturnsErrNotInitialized) {}

//DecodeReset
TEST(DecodeReset, ValidParamsInReturnsErrNone) {}

TEST(DecodeReset, InvalidParamsInReturnsInvalidVideoParam) {}

//profile/level incompatible with resolution
TEST(DecodeReset, IncompatibleParamsInReturnsIncompatibleVideoParam) {}

TEST(DecodeReset, NullParamsInReturnsErrNull) {}

TEST(DecodeReset, NullSessionInReturnsInvalidHandle) {}

TEST(DecodeReset, UninitializedDecodeReturnsErrNotInitialized) {}

//VPPReset
TEST(VPPReset, ValidParamsInReturnsErrNone) {}

TEST(VPPReset, InvalidParamsInReturnsInvalidVideoParam) {}

TEST(VPPReset, NullParamsInReturnsErrNull) {}

TEST(VPPReset, NullSessionInReturnsInvalidHandle) {}

TEST(VPPReset, UninitializedEncodeReturnsErrNotInitialized) {}

/* Close overview

Terminates current operation and de-allocates any internal memory/structures.
MFX_ERR_NONE  The function completed successfully.

mfxStatus MFX_CDECL MFXVideoENCODE_Close(mfxSession session);
mfxStatus MFX_CDECL MFXVideoDECODE_Close(mfxSession session);
mfxStatus MFX_CDECL MFXVideoVPP_Close(mfxSession session);
*/

TEST(EncodeClose, InitializedEncodeReturnsErrNone) {}

TEST(EncodeClose, UninitializedEncodeReturnsErrNotInitialized) {}

TEST(EncodeClose, NullSessionInReturnsInvalidHandle) {}

TEST(DecodeClose, InitializedEncodeReturnsErrNone) {}

TEST(DecodeClose, UninitializedEncodeReturnsErrNotInitialized) {}

TEST(DecodeClose, NullSessionInReturnsInvalidHandle) {}

TEST(VPPClose, InitializedEncodeReturnsErrNone) {}

TEST(VPPClose, UninitializedEncodeReturnsErrNotInitialized) {}

TEST(VPPClose, NullSessionInReturnsInvalidHandle) {}