/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxjpeg.h"
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

TEST(EncodeFrameAsync, ValidInputsReturnsErrNone) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams              = { 0 };
    mfxEncParams.mfx.CodecId                = MFX_CODEC_JPEG;
    mfxEncParams.mfx.FrameInfo.FourCC       = MFX_FOURCC_I420;
    mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    mfxEncParams.mfx.FrameInfo.CropW        = 128;
    mfxEncParams.mfx.FrameInfo.CropH        = 96;
    mfxEncParams.mfx.FrameInfo.Width        = 128;
    mfxEncParams.mfx.FrameInfo.Height       = 96;
    mfxEncParams.IOPattern                  = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    mfxU16 nEncSurfNum = 16;
    mfxU32 lumaSize =
        mfxEncParams.mfx.FrameInfo.Width * mfxEncParams.mfx.FrameInfo.Height;

    mfxU8* surfaceBuffers = new mfxU8[(mfxU32)(lumaSize * 1.5 * nEncSurfNum)];
    memset(surfaceBuffers, 0, lumaSize * 1.5 * nEncSurfNum);

    mfxFrameSurface1* encSurfaces = new mfxFrameSurface1[nEncSurfNum];
    for (mfxI32 i = 0; i < nEncSurfNum; i++) {
        encSurfaces[i]        = { 0 };
        encSurfaces[i].Info   = mfxEncParams.mfx.FrameInfo;
        encSurfaces[i].Data.Y = &surfaceBuffers[(mfxU32)(lumaSize * 1.5 * i)];
        encSurfaces[i].Data.U = encSurfaces[i].Data.Y + lumaSize;
        encSurfaces[i].Data.V = encSurfaces[i].Data.U + lumaSize / 4;
        encSurfaces[i].Data.Pitch = mfxEncParams.mfx.FrameInfo.Width;
    }

    sts = MFXVideoENCODE_Init(session, &mfxEncParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 20000;
    mfxBS.Data         = new mfxU8[mfxBS.MaxLength];

    mfxI32 nEncSurfIdx = 0;
    mfxSyncPoint syncp;

    while (true) {
        // Encode a frame asynchronously (returns immediately)
        sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                              NULL,
                                              &encSurfaces[nEncSurfIdx],
                                              &mfxBS,
                                              &syncp);

        if (sts != MFX_ERR_MORE_DATA)
            break;
        nEncSurfIdx++;
    }
    ASSERT_GT(mfxBS.DataLength, 0);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    delete[] surfaceBuffers;
    delete[] encSurfaces;
    delete[] mfxBS.Data;
}

TEST(EncodeFrameAsync, InsufficientOutBufferReturnsNotEnoughBuffer) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams              = { 0 };
    mfxEncParams.mfx.CodecId                = MFX_CODEC_JPEG;
    mfxEncParams.mfx.FrameInfo.FourCC       = MFX_FOURCC_I420;
    mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    mfxEncParams.mfx.FrameInfo.CropW        = 128;
    mfxEncParams.mfx.FrameInfo.CropH        = 96;
    mfxEncParams.mfx.FrameInfo.Width        = 128;
    mfxEncParams.mfx.FrameInfo.Height       = 96;
    mfxEncParams.IOPattern                  = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    mfxU16 nEncSurfNum = 16;
    mfxU32 lumaSize =
        mfxEncParams.mfx.FrameInfo.Width * mfxEncParams.mfx.FrameInfo.Height;

    mfxU8* surfaceBuffers = new mfxU8[(mfxU32)(lumaSize * 1.5 * nEncSurfNum)];
    memset(surfaceBuffers, 0, lumaSize * 1.5 * nEncSurfNum);

    mfxFrameSurface1* encSurfaces = new mfxFrameSurface1[nEncSurfNum];
    for (mfxI32 i = 0; i < nEncSurfNum; i++) {
        encSurfaces[i]        = { 0 };
        encSurfaces[i].Info   = mfxEncParams.mfx.FrameInfo;
        encSurfaces[i].Data.Y = &surfaceBuffers[(mfxU32)(lumaSize * 1.5 * i)];
        encSurfaces[i].Data.U = encSurfaces[i].Data.Y + lumaSize;
        encSurfaces[i].Data.V = encSurfaces[i].Data.U + lumaSize / 4;
        encSurfaces[i].Data.Pitch = mfxEncParams.mfx.FrameInfo.Width;
    }

    sts = MFXVideoENCODE_Init(session, &mfxEncParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 20;
    mfxBS.Data         = new mfxU8[mfxBS.MaxLength];

    mfxI32 nEncSurfIdx = 0;
    mfxSyncPoint syncp;

    while (true) {
        // Encode a frame asynchronously (returns immediately)
        sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                              NULL,
                                              &encSurfaces[nEncSurfIdx],
                                              &mfxBS,
                                              &syncp);

        if (sts != MFX_ERR_MORE_DATA)
            break;
        nEncSurfIdx++;
    }
    ASSERT_EQ(sts, MFX_ERR_NOT_ENOUGH_BUFFER);

    delete[] surfaceBuffers;
    delete[] encSurfaces;
    delete[] mfxBS.Data;
}

TEST(EncodeFrameAsync, NullSessionReturnsInvalidHandle) {
    mfxStatus sts =
        MFXVideoENCODE_EncodeFrameAsync(0, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(EncodeFrameAsync, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(EncodeFrameAsync, DISABLED_NullBitstreamReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(EncodeFrameAsync, DISABLED_EncodeUninitializedReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}

/*!
   DecodeFrame overview
   Decodes the input bitstream to a single output frame.

   MFX_ERR_NONE The function completed successfully and the output surface is ready for decoding \n
   MFX_ERR_MORE_DATA The function requires more bitstream at input before decoding can proceed. \n
   MFX_ERR_MORE_SURFACE The function requires more frame surface at output before decoding can proceed. \n
   MFX_WRN_VIDEO_PARAM_CHANGED  The decoder detected a new sequence header in the bitstream. Video parameters may have changed. \n

*/
//mfxStatus MFX_CDECL MFXVideoDECODE_DecodeFrameAsync(mfxSession session, mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out, mfxSyncPoint *syncp);

TEST(DecodeFrameAsync, DISABLED_ValidInputsReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(DecodeFrameAsync, DISABLED_InsufficientInBitstreamReturnsMoreData) {
    FAIL() << "Test not implemented";
}

TEST(DecodeFrameAsync, DISABLED_InsufficientSurfacesReturnsMoreSurface) {
    FAIL() << "Test not implemented";
}

TEST(DecodeFrameAsync, NullSessionReturnsInvalidHandle) {
    mfxStatus sts =
        MFXVideoDECODE_DecodeFrameAsync(0, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(DecodeFrameAsync, DISABLED_NullSurfaceWorkReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(DecodeFrameAsync, DISABLED_NullSurfaceOutReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(DecodeFrameAsync, DISABLED_DecodeUninitializedReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}
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

TEST(RunFrameVPPAsync, DISABLED_ValidInputsReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(RunFrameVPPAsync, NullSessionReturnsInvalidHandle) {
    mfxStatus sts =
        MFXVideoVPP_RunFrameVPPAsync(0, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(RunFrameVPPAsync, DISABLED_NullSurfaceInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(RunFrameVPPAsync, DISABLED_NullSurfaceOutReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(RunFrameVPPAsync, DISABLED_VPPUninitializedReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}
