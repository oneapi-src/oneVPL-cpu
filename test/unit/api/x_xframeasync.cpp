/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

/*!
    EncodeFrame overview
    Takes a single input frame in and generates its output bitstream. 

    If the encoder needs to cache the frame, the function locks the frame. 
    
    The BufferSizeInKB in the mfxVideoParam structure specifies maximum size for
    compressed frames. This value can also be obtained from MFXVideoENCODE_GetVideoParam.
       
    To mark the end of the encoding sequence, call this function with a NULL surface pointer. Repeat the call to drain any remaining internally cached bitstreams
    (one frame at a time) until MFX_ERR_MORE_DATA is returned.



   @return 
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_NOT_ENOUGH_BUFFER  The bitstream buffer size is insufficient. \n
   MFX_ERR_MORE_DATA   The function requires more data to generate any output. \n

  mfxStatus MFXVideoENCODE_EncodeFrameAsync(mfxSession session, mfxEncodeCtrl *ctrl, mfxFrameSurface1 *surface, mfxBitstream *bs, mfxSyncPoint *syncp);

*/

TEST(EncodeFrameAsync, ValidInputsReturnsErrNone) {}

TEST(EncodeFrameAsync, InsufficientOutBufferReturnsNotEnoughBuffer) {}

TEST(EncodeFrameAsync, NullSessionReturnsInvalidHandle) {}

TEST(EncodeFrameAsync, NullSurfaceReturnsErrNull) {}

TEST(EncodeFrameAsync, NullBitstreamReturnsErrNull) {}

/*!
   DecodeFrame overview
   Decodes the input bitstream to a single output frame.

   MFX_ERR_NONE The function completed successfully and the output surface is ready for decoding \n
   MFX_ERR_MORE_DATA The function requires more bitstream at input before decoding can proceed. \n
   MFX_ERR_MORE_SURFACE The function requires more frame surface at output before decoding can proceed. \n
   MFX_WRN_VIDEO_PARAM_CHANGED  The decoder detected a new sequence header in the bitstream. Video parameters may have changed. \n

*/
//mfxStatus MFX_CDECL MFXVideoDECODE_DecodeFrameAsync(mfxSession session, mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out, mfxSyncPoint *syncp);

TEST(DecodeFrameAsync, ValidInputsReturnsErrNone) {}

TEST(DecodeFrameAsync, InsufficientInBitstreamReturnsMoreData) {}

TEST(DecodeFrameAsync, InsufficientSurfacesReturnsMoreSurface) {}

TEST(DecodeFrameAsync, NullSessionReturnsInvalidHandle) {}

TEST(DecodeFrameAsync, NullSurfaceWorkReturnsErrNull) {}

TEST(DecodeFrameAsync, NullSurfaceOutReturnsErrNull) {}

/*!
   RunFrameVPPAsync overview
   Processes a single input frame to a single output frame. 

   @param[in] session SDK session handle.
   @param[in] in  Pointer to the input video surface structure
   @param[out] out  Pointer to the output video surface structure
   @param[in] aux  Optional pointer to the auxiliary data structure
   @param[out] syncp  Pointer to the output sync point

   @return 
   MFX_ERR_NONE The output frame is ready after synchronization. \n
   
*/
//mfxStatus MFX_CDECL MFXVideoVPP_RunFrameVPPAsync(mfxSession session, mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, mfxSyncPoint *syncp);

TEST(RunFrameVPPAsync, ValidInputsReturnsErrNone) {}

TEST(RunFrameVPPAsync, NullSessionReturnsInvalidHandle) {}

TEST(RunFrameVPPAsync, NullSurfaceInReturnsErrNull) {}

TEST(RunFrameVPPAsync, NullSurfaceOutReturnsErrNull) {}
