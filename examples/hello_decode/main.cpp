/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <iostream>
#include "onevpl/mfxvideo.h"

void WriteRawFrame(mfxFrameSurface1 *pSurface, FILE *f);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
int GetFreeSurfaceIndex(mfxFrameSurface1 *SurfacesPool, mfxU16 nPoolSize);
void Usage(void);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        Usage();
        return 1; // return 1 as error code
    }

    mfxU32 codecID;
    if (strncmp("h265", argv[1], 4) == 0) {
        codecID = MFX_CODEC_HEVC;
        puts("h265 decoding");
    }
    else if (strncmp("av1", argv[1], 3) == 0) {
        codecID = MFX_CODEC_AV1;
        puts("av1 decoding");
    }
    else {
        printf("%s is not supported\n", argv[1]);
        Usage();
        return 1;
    }

    printf("opening %s\n", argv[2]);
    FILE *fSource = fopen(argv[2], "rb");
    if (!fSource) {
        printf("could not open input file, %s\n", argv[2]);
        return 1;
    }
    FILE *fSink = fopen(argv[3], "wb");

    // init bitstream
    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 2000000;
    mfxBS.Data         = new mfxU8[mfxBS.MaxLength];

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

    puts("oneVPL initialized");

    // set up input bitstream
    memmove(mfxBS.Data, mfxBS.Data + mfxBS.DataOffset, mfxBS.DataLength);
    mfxBS.DataLength =
        static_cast<mfxU32>(fread(mfxBS.Data, 1, mfxBS.MaxLength, fSource));

    // initialize decode parameters from stream header
    mfxVideoParam mfxDecParams = { 0 };
    mfxDecParams.mfx.CodecId   = codecID;
    mfxDecParams.IOPattern     = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        printf("Problem decoding header.  DecodeHeader sts=%d\n", sts);
        return 1;
    }

    // Query number required surfaces for decoder
    mfxFrameAllocRequest DecRequest = { 0 };
    MFXVideoDECODE_QueryIOSurf(session, &mfxDecParams, &DecRequest);

    // Determine the required number of surfaces for decoder output
    mfxU16 nSurfNumDec = DecRequest.NumFrameSuggested;

    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];

    // initialize surface pool for decode (I420 format)
    mfxU32 surfaceSize = GetSurfaceSize(mfxDecParams.mfx.FrameInfo.FourCC,
                                        mfxDecParams.mfx.FrameInfo.Width,
                                        mfxDecParams.mfx.FrameInfo.Height);
    if (surfaceSize == 0) {
        puts("Surface size is wrong");
        return 1;
    }
    size_t framePoolBufSize = static_cast<size_t>(surfaceSize * nSurfNumDec);
    mfxU8 *DECoutbuf        = new mfxU8[framePoolBufSize];

    mfxU16 surfW = (mfxDecParams.mfx.FrameInfo.FourCC == MFX_FOURCC_I010)
                       ? mfxDecParams.mfx.FrameInfo.Width * 2
                       : mfxDecParams.mfx.FrameInfo.Width;
    mfxU16 surfH = mfxDecParams.mfx.FrameInfo.Height;

    for (int i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]        = { 0 };
        decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
        size_t buf_offset     = static_cast<size_t>(i * surfaceSize);
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        decSurfaces[i].Data.Pitch = surfW;
    }

    // input parameters finished, now initialize decode
    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        puts("Could not initialize decode");
        exit(1);
    }
    // ------------------
    // main loop
    // ------------------
    int framenum                     = 0;
    double decode_time               = 0;
    double sync_time                 = 0;
    mfxSyncPoint syncp               = { 0 };
    mfxFrameSurface1 *pmfxOutSurface = nullptr;

    puts("start decoding");
    for (;;) {
        bool stillgoing = true;
        int nIndex      = GetFreeSurfaceIndex(decSurfaces, nSurfNumDec);
        while (stillgoing) {
            // submit async decode request
            auto t0 = std::chrono::high_resolution_clock::now();
            sts     = MFXVideoDECODE_DecodeFrameAsync(session,
                                                  &mfxBS,
                                                  &decSurfaces[nIndex],
                                                  &pmfxOutSurface,
                                                  &syncp);
            auto t1 = std::chrono::high_resolution_clock::now();
            decode_time +=
                std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
                    .count();

            // next step actions provided by application
            switch (sts) {
                case MFX_ERR_MORE_DATA: // more data is needed to decode
                {
                    memmove(mfxBS.Data,
                            mfxBS.Data + mfxBS.DataOffset,
                            mfxBS.DataLength);
                    mfxBS.DataOffset = 0;
                    mfxBS.DataLength = static_cast<mfxU32>(
                        fread(mfxBS.Data + mfxBS.DataLength,
                              1,
                              mfxBS.MaxLength - mfxBS.DataLength,
                              fSource));
                    if (mfxBS.DataLength == 0)
                        stillgoing = false; // stop if end of file
                } break;
                case MFX_ERR_MORE_SURFACE: // feed a fresh surface to decode
                    nIndex = GetFreeSurfaceIndex(decSurfaces, nSurfNumDec);
                    break;
                case MFX_ERR_NONE: // no more steps needed, exit loop
                    stillgoing = false;
                    break;
                default: // state is not one of the cases above
                    printf("Error in DecodeFrameAsync: sts=%d\n", sts);
                    exit(1);
                    break;
            }
        }

        if (sts < 0)
            break;

        // data available to app only after sync
        auto t0 = std::chrono::high_resolution_clock::now();
        MFXVideoCORE_SyncOperation(session, syncp, 60000);
        auto t1 = std::chrono::high_resolution_clock::now();
        sync_time +=
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
                .count();

        // write output if output file specified
        if (fSink)
            WriteRawFrame(pmfxOutSurface, fSink);

        framenum++;
    }

    sts = MFX_ERR_NONE;
    memset(&syncp, 0, sizeof(mfxSyncPoint));
    pmfxOutSurface = nullptr;

    // retrieve the buffered decoded frames
    while (MFX_ERR_NONE <= sts || MFX_ERR_MORE_SURFACE == sts) {
        int nIndex =
            GetFreeSurfaceIndex(decSurfaces,
                                nSurfNumDec); // Find free frame surface

        // Decode a frame asychronously (returns immediately)
        auto t0 = std::chrono::high_resolution_clock::now();
        sts     = MFXVideoDECODE_DecodeFrameAsync(session,
                                              NULL,
                                              &decSurfaces[nIndex],
                                              &pmfxOutSurface,
                                              &syncp);
        auto t1 = std::chrono::high_resolution_clock::now();
        decode_time +=
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
                .count();

        // Ignore warnings if output is available,
        // if no output and no action required just repeat the DecodeFrameAsync call
        if (MFX_ERR_NONE < sts && syncp)
            sts = MFX_ERR_NONE;

        if (sts == MFX_ERR_NONE) {
            t0  = std::chrono::high_resolution_clock::now();
            sts = MFXVideoCORE_SyncOperation(
                session,
                syncp,
                60000); // Synchronize. Waits until decoded frame is ready
            t1 = std::chrono::high_resolution_clock::now();
            sync_time +=
                std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
                    .count();

            // write output if output file specified
            if (fSink)
                WriteRawFrame(pmfxOutSurface, fSink);

            framenum++;
        }
    }

    printf("read %d frames\n", framenum);
    printf("decode avg=%f usec, sync avg=%f usec\n",
           decode_time / framenum,
           sync_time / framenum);

    if (fSink)
        fclose(fSink);
    fclose(fSource);
    MFXVideoDECODE_Close(session);
    delete[] mfxBS.Data;
    delete[] DECoutbuf;
    delete[] decSurfaces;

    return 0;
}

mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
    mfxU32 nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_IYUV:
            nbytes = width * height + (width >> 1) * (height >> 1) +
                     (width >> 1) * (height >> 1);
            break;
        case MFX_FOURCC_I010:
            nbytes = width * height + (width >> 1) * (height >> 1) +
                     (width >> 1) * (height >> 1);
            nbytes *= 2;
            break;
        default:
            break;
    }

    return nbytes;
}

int GetFreeSurfaceIndex(mfxFrameSurface1 *SurfacesPool, mfxU16 nPoolSize) {
    for (mfxU16 i = 0; i < nPoolSize; i++) {
        if (0 == SurfacesPool[i].Data.Locked)
            return i;
    }
    return MFX_ERR_NOT_FOUND;
}

void WriteRawFrame(mfxFrameSurface1 *pSurface, FILE *f) {
    mfxU16 w, h, i, pitch;
    mfxFrameInfo *pInfo = &pSurface->Info;
    mfxFrameData *pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    // write the output to disk
    switch (pInfo->FourCC) {
        case MFX_FOURCC_IYUV:
            //Y
            pitch = pData->Pitch;
            for (i = 0; i < h; i++) {
                fwrite(pData->Y + i * pitch, 1, w, f);
            }

            //U
            pitch /= 2;
            h /= 2;
            w /= 2;
            for (i = 0; i < h; i++) {
                fwrite(pData->U + i * pitch, 1, w, f);
            }
            //V
            for (i = 0; i < h; i++) {
                fwrite(pData->V + i * pitch, 1, w, f);
            }
            break;

        case MFX_FOURCC_I010:
            //Y
            pitch = pData->Pitch;
            w *= 2;
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.Y + i * pitch, 1, w, f);
            }

            //U
            pitch /= 2;
            w /= 2;
            h /= 2;
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.U + i * pitch, 1, w, f);
            }
            //V
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.V + i * pitch, 1, w, f);
            }
            break;
        default:
            break;
    }

    return;
}

void Usage(void) {
    printf("Usage: hello_decode [decoder] [input filename] [out filename]\n\n");
    printf("\t[decoder]         : h265|av1\n");
    printf("\t[input filename]  : encoded stream\n");
    printf("\t[out filename]    : filename to store the output\n");
    printf("To view:\n");
    printf(
        " ffplay -video_size [width]x[height] -pixel_format [pixel format] -f rawvideo [out filename]\n");
    return;
}