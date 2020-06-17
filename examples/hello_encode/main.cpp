/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <algorithm>
#include "onevpl/mfxvideo.h"

#define ALIGN_UP(addr, size) \
    (((addr) + ((size)-1)) & (~((decltype(addr))(size)-1)))

mfxStatus LoadRawFrame(mfxFrameSurface1* pSurface, FILE* fSource) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxU16 w, h, i, pitch;
    mfxU32 nBytesRead;
    mfxU8* ptr;
    mfxFrameInfo* pInfo = &pSurface->Info;
    mfxFrameData* pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    // read luminance plane (Y)
    pitch = pData->Pitch;
    ptr   = pData->Y;
    for (i = 0; i < h; i++) {
        nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
        if (w != nBytesRead)
            return MFX_ERR_MORE_DATA;
    }

    // read chrominance (U, V)
    pitch /= 2;
    h /= 2;
    w /= 2;
    ptr = pData->U;
    for (i = 0; i < h; i++) {
        nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
        if (w != nBytesRead)
            return MFX_ERR_MORE_DATA;
    }

    ptr = pData->V;
    for (i = 0; i < h; i++) {
        nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
        if (w != nBytesRead)
            return MFX_ERR_MORE_DATA;
    }

    return MFX_ERR_NONE;
}

int GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1>& pSurfacesPool) {
    auto it = std::find_if(pSurfacesPool.begin(),
                           pSurfacesPool.end(),
                           [](const mfxFrameSurface1& surface) {
                               return 0 == surface.Data.Locked;
                           });

    if (it == pSurfacesPool.end())
        return MFX_ERR_NOT_FOUND;
    else
        return it - pSurfacesPool.begin();
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf(
            "Usage: hello_encode [encoder] [input filename] [out filename] [width] [height]\n\n");
        printf("\t[encoder]:        h265\n");
        printf("\t[input filename]: raw video file (i420 only)\n");
        printf("\t[out filename]:   filename to store the output\n");
        printf("\t[width]:          width of input video\n");
        printf("\t[height]:         height of input video\n\n");
        printf("To view:\n");
        printf(" ffplay out\n");
        return 1; // return 1 as error code
    }

    FILE* fSink = fopen(argv[3], "wb");
    printf("opening %s\n", argv[1]);
    FILE* fSource = fopen(argv[2], "rb");
    if (!fSource) {
        printf("could not open input file\n");
        return 1;
    }

    mfxU16 inputWidth  = atoi(argv[4]);
    mfxU16 inputHeight = atoi(argv[5]);

    // Initialize Media SDK session
    mfxInitParam initPar   = { 0 };
    initPar.Version.Major  = 1;
    initPar.Version.Minor  = 1;
    initPar.Implementation = MFX_IMPL_SOFTWARE_VPL;

    mfxSession session;
    mfxStatus sts = MFXInitEx(initPar, &session);
    if (sts != MFX_ERR_NONE) {
        puts("MFXInitEx error. could not initialize session");
        return 1;
    }

    puts("initialized");

    // Initialize encoder parameters
    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));
    mfxEncParams.mfx.CodecId                 = MFX_CODEC_HEVC;
    mfxEncParams.mfx.TargetUsage             = MFX_TARGETUSAGE_BALANCED;
    mfxEncParams.mfx.TargetKbps              = 4000;
    mfxEncParams.mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    mfxEncParams.mfx.FrameInfo.FrameRateExtN = 30;
    mfxEncParams.mfx.FrameInfo.FrameRateExtD = 1;
    mfxEncParams.mfx.FrameInfo.FourCC        = MFX_FOURCC_YV12;
    mfxEncParams.mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxEncParams.mfx.FrameInfo.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxEncParams.mfx.FrameInfo.CropX         = 0;
    mfxEncParams.mfx.FrameInfo.CropY         = 0;
    mfxEncParams.mfx.FrameInfo.CropW         = inputWidth;
    mfxEncParams.mfx.FrameInfo.CropH         = inputHeight;
    // Width must be a multiple of 16
    // Height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
    mfxEncParams.mfx.FrameInfo.Width  = ALIGN_UP(inputWidth, 16);
    mfxEncParams.mfx.FrameInfo.Height = ALIGN_UP(inputHeight, 16);

    mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    // Query number required surfaces for encoder
    mfxFrameAllocRequest EncRequest = { 0 };
    MFXVideoENCODE_QueryIOSurf(session, &mfxEncParams, &EncRequest);

    // Determine the required number of surfaces for encoder
    mfxU16 nEncSurfNum = EncRequest.NumFrameSuggested;

    // Allocate surfaces for encoder
    // - Frame surface array keeps pointers all surface planes and general frame info
    mfxU8 bitsPerPixel = 12; // I420 format is a 12 bits per pixel format
    mfxU32 surfaceSize = inputWidth * inputHeight * bitsPerPixel / 8;
    std::vector<mfxU8> surfaceBuffersData(surfaceSize * nEncSurfNum);
    mfxU8* surfaceBuffers = surfaceBuffersData.data();

    // Allocate surface headers (mfxFrameSurface1) for encoder
    std::vector<mfxFrameSurface1> pEncSurfaces(nEncSurfNum);
    for (int i = 0; i < nEncSurfNum; i++) {
        memset(&pEncSurfaces[i], 0, sizeof(mfxFrameSurface1));
        pEncSurfaces[i].Info   = mfxEncParams.mfx.FrameInfo;
        pEncSurfaces[i].Data.Y = &surfaceBuffers[surfaceSize * i];
        pEncSurfaces[i].Data.U =
            pEncSurfaces[i].Data.Y + inputWidth * inputHeight;
        pEncSurfaces[i].Data.V =
            pEncSurfaces[i].Data.U + ((inputWidth / 2) * (inputHeight / 2));
        pEncSurfaces[i].Data.Pitch = inputWidth;
    }

    // Initialize the Media SDK encoder
    sts = MFXVideoENCODE_Init(session, &mfxEncParams);
    if (sts != MFX_ERR_NONE) {
        puts("could not initialize encode");
        return 1;
    }

    // Prepare Media SDK bit stream buffer
    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 2000000;
    std::vector<mfxU8> bstData(mfxBS.MaxLength);
    mfxBS.Data = bstData.data();

    // Start encoding the frames
    int nEncSurfIdx = 0;
    mfxSyncPoint syncp;
    mfxU32 nFrame = 0;

    puts("start encoding");

    // Stage 1: Main encoding loop
    while (MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts) {
        nEncSurfIdx =
            GetFreeSurfaceIndex(pEncSurfaces); // Find free frame surface
        if (nEncSurfIdx == MFX_ERR_NOT_FOUND) {
            puts("no available surface");
            return 1;
        }

        sts = LoadRawFrame(&pEncSurfaces[nEncSurfIdx], fSource);
        if (sts != MFX_ERR_NONE)
            break;

        for (;;) {
            // Encode a frame asychronously (returns immediately)
            sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                                  NULL,
                                                  &pEncSurfaces[nEncSurfIdx],
                                                  &mfxBS,
                                                  &syncp);
            if (MFX_ERR_NONE < sts && syncp) {
                sts = MFX_ERR_NONE; // Ignore warnings if output is available
                break;
            }
            else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts) {
                // Allocate more bitstream buffer memory here if needed...
                break;
            }
            else {
                break;
            }
        }

        if (MFX_ERR_NONE == sts) {
            sts = MFXVideoCORE_SyncOperation(
                session,
                syncp,
                60000); // Synchronize. Wait until encoded frame is ready
            fwrite(mfxBS.Data + mfxBS.DataOffset, 1, mfxBS.DataLength, fSink);
            ++nFrame;
            mfxBS.DataLength = 0;
        }
    }

    sts = MFX_ERR_NONE;

    // Stage 2: Retrieve the buffered encoded frames
    while (MFX_ERR_NONE <= sts) {
        for (;;) {
            // Encode a frame asychronously (returns immediately)
            sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                                  NULL,
                                                  NULL,
                                                  &mfxBS,
                                                  &syncp);
            if (MFX_ERR_NONE < sts && syncp) {
                sts = MFX_ERR_NONE; // Ignore warnings if output is available
                break;
            }
            else {
                break;
            }
        }

        if (MFX_ERR_NONE == sts) {
            sts = MFXVideoCORE_SyncOperation(
                session,
                syncp,
                60000); // Synchronize. Wait until encoded frame is ready
            fwrite(mfxBS.Data + mfxBS.DataOffset, 1, mfxBS.DataLength, fSink);
            ++nFrame;
            mfxBS.DataLength = 0;
        }
    }

    // Clean up resources
    //  - It is recommended to close Media SDK components first, before releasing allocated surfaces, since
    //    some surfaces may still be locked by internal Media SDK resources.
    MFXVideoENCODE_Close(session);

    fclose(fSource);
    fclose(fSink);

    puts("done!");
    return 0;
}
