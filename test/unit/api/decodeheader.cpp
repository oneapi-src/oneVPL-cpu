
/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

/* DecodeHeader overview
  Parses the input bitstream and fills the mfxVideoParam structure with appropriate values, 
  such as resolution and frame rate, for the Init function. 
  The application can then pass the resulting structure to the MFXVideoDECODE_Init function for decoder initialization.

  An application can call this function at any time before or after decoder initialization. 
  If the SDK finds a sequence header in the bitstream, the function moves the bitstream pointer to 
  the first bit of the sequence header. 
   MFX_ERR_NONE The function successfully filled structure. It does not mean that the stream can be decoded by SDK. 
                The application should call MFXVideoDECODE_Query function to check if decoding of the stream is supported. \n
   MFX_ERR_MORE_DATA   The function requires more bitstream data \n
   MFX_ERR_UNSUPPORTED  CodecId field of the mfxVideoParam structure indicates some unsupported codec. \n
   MFX_ERR_INVALID_HANDLE  session is not initialized \n
   MFX_ERR_NULL_PTR  bs or par pointer is NULL.


mfxStatus MFX_CDECL MFXVideoDECODE_DecodeHeader(mfxSession session, mfxBitstream *bs, mfxVideoParam *par);
*/

TEST(DecodeHeader, EightBitInReturnsCorrectMetadata) {}

TEST(DecodeHeader, TenBitInReturnsCorrectMetadata) {}

TEST(DecodeHeader, NullSessionReturnsInvalidHandle) {}

TEST(DecodeHeader, NullBitstreamInReturnsErrNull) {}

TEST(DecodeHeader, NullParamsInReturnsErrNull) {}
