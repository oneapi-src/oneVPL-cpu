//==============================================================================
// Copyright (C) 2020 Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) encode application.
///
/// @file

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "vpl/mfxvideo.h"

#define OUTPUT_FILE "out.h265"

#define ALIGN_UP(addr, size) \
    (((addr) + ((size)-1)) & (~((decltype(addr))(size)-1)))

mfxStatus LoadRawFrame(mfxFrameSurface1 *pSurface, FILE *fSource);
void WriteEncodedStream(mfxU8 *data, mfxU32 length, FILE *f);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
int GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1> &pSurfacesPool);
void Usage(void);

int main(int argc, char *argv[]) {
    mfxU32 codecID = MFX_CODEC_HEVC;
    mfxU32 fourCC  = MFX_FOURCC_IYUV;
    if (argc != 4) {
        Usage();
        return 1;
    }

    FILE *fSource = fopen(argv[1], "rb");
    if (!fSource) {
        printf("could not open input file, %s\n", argv[1]);
        return 1;
    }
    FILE *fSink = fopen(OUTPUT_FILE, "wb");
    if (!fSink) {
        printf("could not open output file, %s\n", OUTPUT_FILE);
        return 1;
    }
    mfxU16 inputWidth  = atoi(argv[2]);
    mfxU16 inputHeight = atoi(argv[3]);

    // initialize  session
    mfxInitParam initPar   = { 0 };
    initPar.Version.Major  = 1;
    initPar.Version.Minor  = 35;
    initPar.Implementation = MFX_IMPL_SOFTWARE;

    mfxSession session;
    mfxStatus sts = MFXInitEx(initPar, &session);
    if (sts != MFX_ERR_NONE) {
        puts("MFXInitEx error.  Could not initialize session");
        return 1;
    }

    // Initialize encoder parameters
    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));
    mfxEncParams.mfx.CodecId                 = codecID;
    mfxEncParams.mfx.TargetUsage             = MFX_TARGETUSAGE_BALANCED;
    mfxEncParams.mfx.TargetKbps              = 4000;
    mfxEncParams.mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    mfxEncParams.mfx.FrameInfo.FrameRateExtN = 30;
    mfxEncParams.mfx.FrameInfo.FrameRateExtD = 1;
    mfxEncParams.mfx.FrameInfo.FourCC        = fourCC;
    mfxEncParams.mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxEncParams.mfx.FrameInfo.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxEncParams.mfx.FrameInfo.CropX         = 0;
    mfxEncParams.mfx.FrameInfo.CropY         = 0;
    mfxEncParams.mfx.FrameInfo.CropW         = inputWidth;
    mfxEncParams.mfx.FrameInfo.CropH         = inputHeight;
    // Width must be a multiple of 16
    mfxEncParams.mfx.FrameInfo.Width = ALIGN_UP(inputWidth, 16);
    // Height must be a multiple of 16 in case of frame picture and a multiple
    // of 32 in case of field picture
    mfxEncParams.mfx.FrameInfo.Height = ALIGN_UP(inputHeight, 16);

    mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    // Query number required surfaces for encoder
    mfxFrameAllocRequest EncRequest = { 0 };
    sts = MFXVideoENCODE_QueryIOSurf(session, &mfxEncParams, &EncRequest);

    if (sts != MFX_ERR_NONE) {
        puts("QueryIOSurf error");
        return 1;
    }

    // Determine the required number of surfaces for encoder
    mfxU16 nEncSurfNum = EncRequest.NumFrameSuggested;

    // Allocate surfaces for encoder - Frame surface array keeps pointers all
    // surface planes and general frame info
    mfxU32 surfaceSize = GetSurfaceSize(fourCC, inputWidth, inputHeight);
    if (surfaceSize == 0) {
        puts("Surface size is wrong");
        return 1;
    }

    std::vector<mfxU8> surfaceBuffersData(surfaceSize * nEncSurfNum);
    mfxU8 *surfaceBuffers = surfaceBuffersData.data();

    mfxU16 surfW = inputWidth;
    mfxU16 surfH = inputHeight;

    // Allocate surface headers (mfxFrameSurface1) for encoder
    std::vector<mfxFrameSurface1> pEncSurfaces(nEncSurfNum);
    for (int i = 0; i < nEncSurfNum; i++) {
        memset(&pEncSurfaces[i], 0, sizeof(mfxFrameSurface1));
        pEncSurfaces[i].Info   = mfxEncParams.mfx.FrameInfo;
        pEncSurfaces[i].Data.Y = &surfaceBuffers[surfaceSize * i];

        pEncSurfaces[i].Data.U = pEncSurfaces[i].Data.Y + surfW * surfH;
        pEncSurfaces[i].Data.V =
            pEncSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        pEncSurfaces[i].Data.Pitch = surfW;
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

    double encode_time = 0;
    double sync_time   = 0;

    // Start encoding the frames
    mfxU16 nEncSurfIdx = 0;
    mfxSyncPoint syncp;
    mfxU32 framenum = 0;

    printf("Encoding %s -> %s\n", argv[1], OUTPUT_FILE);

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
            // Encode a frame asynchronously (returns immediately)
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
            ++framenum;
            WriteEncodedStream(mfxBS.Data + mfxBS.DataOffset,
                               mfxBS.DataLength,

                               fSink);
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

            ++framenum;
            WriteEncodedStream(mfxBS.Data + mfxBS.DataOffset,
                               mfxBS.DataLength,

                               fSink);

            mfxBS.DataLength = 0;
        }
    }

    printf("Encoded %d frames\n", framenum);

    // Clean up resources - It is recommended to close Media SDK components
    // first, before releasing allocated surfaces, since some surfaces may still
    // be locked by internal Media SDK resources.
    MFXVideoENCODE_Close(session);

    fclose(fSource);
    fclose(fSink);

    return 0;
}

mfxStatus LoadRawFrame(mfxFrameSurface1 *pSurface, FILE *fSource) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxU16 w, h, i, pitch;
    mfxU32 nBytesRead;
    mfxU8 *ptr;
    mfxFrameInfo *pInfo = &pSurface->Info;
    mfxFrameData *pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    switch (pInfo->FourCC) {
        case MFX_FOURCC_IYUV:
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
            break;
        default:
            break;
    }

    return MFX_ERR_NONE;
}

// Write encoded stream to file
void WriteEncodedStream(mfxU8 *data, mfxU32 length, FILE *f) {
    fwrite(data, 1, length, f);
}

// Return the surface size in bytes given format and dimensions
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
    mfxU32 nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_IYUV:
            nbytes = width * height + (width >> 1) * (height >> 1) +
                     (width >> 1) * (height >> 1);
            break;
        default:
            break;
    }

    return nbytes;
}

// Return index of free surface in given pool
int GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1> &pSurfacesPool) {
    auto it = std::find_if(pSurfacesPool.begin(),
                           pSurfacesPool.end(),
                           [](const mfxFrameSurface1 &surface) {
                               return 0 == surface.Data.Locked;
                           });

    if (it == pSurfacesPool.end())
        return MFX_ERR_NOT_FOUND;
    else
        return static_cast<int>(it - pSurfacesPool.begin());
}

// Print usage message
void Usage(void) {
    printf("Usage: hello-encode SOURCE WIDTH HEIGHT\n\n"
           "Encode raw I420 video in SOURCE having dimensions WIDTH x HEIGHT "
           "to H265 in %s\n\n"
           "To view:\n"
           " ffplay %s\n",
           OUTPUT_FILE,
           OUTPUT_FILE);
}
