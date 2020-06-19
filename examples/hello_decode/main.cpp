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

int GetFreeSurfaceIndex(mfxFrameSurface1 *SurfacesPool, mfxU16 nPoolSize) {
    for (mfxU16 i = 0; i < nPoolSize; i++) {
        if (0 == SurfacesPool[i].Data.Locked)
            return i;
    }
    return MFX_ERR_NOT_FOUND;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: hello_decode in.h265\n\n");
        printf("decodes an h265/hevc file to out.raw\n");
        printf("to view:\n");
        printf(" ffplay -s 1280x720 -pix_fmt yuv420p -f rawvideo out.raw\n");
        exit(1);
    }

    FILE *fSink = fopen("out.raw", "wb");
    printf("opening %s\n", argv[1]);
    FILE *fSource = fopen(argv[1], "rb");
    if (!fSource) {
        puts("could not open input file");
        exit(1);
    }

    // init bitstream
    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 2000000;
    mfxBS.Data         = new mfxU8[mfxBS.MaxLength];

    // initialize  session
    mfxInitParam initPar   = { 0 };
    initPar.Version.Major  = 1;
    initPar.Version.Minor  = 1;
    initPar.Implementation = MFX_IMPL_SOFTWARE_VPL;

    mfxSession session;
    mfxStatus sts = MFXInitEx(initPar, &session);
    if (sts != MFX_ERR_NONE) {
        puts("MFXInitEx error.  Could not initialize session");
        exit(1);
    }
    puts("oneVPL initialized");

    // set up input bitstream
    memmove(mfxBS.Data, mfxBS.Data + mfxBS.DataOffset, mfxBS.DataLength);
    mfxBS.DataLength =
        static_cast<mfxU32>(fread(mfxBS.Data, 1, mfxBS.MaxLength, fSource));

    // initialize decode parameters from stream header
    mfxVideoParam mfxDecParams = { 0 };
    mfxDecParams.mfx.CodecId   = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern     = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        printf("Problem decoding header.  DecodeHeader sts=%d\n", sts);
        exit(1);
    }

    // Query number required surfaces for decoder
    mfxFrameAllocRequest DecRequest = { 0 };
    MFXVideoDECODE_QueryIOSurf(session, &mfxDecParams, &DecRequest);

    // Determine the required number of surfaces for decoder output
    mfxU16 nSurfNumDec = DecRequest.NumFrameSuggested;

    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];

    // initialize surface pool for decode (I420 format)
    size_t framePoolBufSize = static_cast<size_t>(
        mfxDecParams.mfx.FrameInfo.Width * mfxDecParams.mfx.FrameInfo.Height *
        1.5 * nSurfNumDec);
    mfxU8 *DECoutbuf = new mfxU8[framePoolBufSize];

    for (int i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]      = { 0 };
        decSurfaces[i].Info = mfxDecParams.mfx.FrameInfo;
        size_t buf_offset =
            static_cast<size_t>(i * mfxDecParams.mfx.FrameInfo.Height *
                                mfxDecParams.mfx.FrameInfo.Width * 1.5);
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset +
                                mfxDecParams.mfx.FrameInfo.Height *
                                    mfxDecParams.mfx.FrameInfo.Width;
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + mfxDecParams.mfx.FrameInfo.Height / 2 *
                                        mfxDecParams.mfx.FrameInfo.Width / 2;
        decSurfaces[i].Data.Pitch = mfxDecParams.mfx.FrameInfo.Width;
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
    int framenum       = 0;
    double decode_time = 0;
    double sync_time   = 0;

    for (;;) {
        mfxSyncPoint syncp               = { 0 };
        mfxFrameSurface1 *pmfxOutSurface = nullptr;

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
        if (fSink) {
            // write the output to disk
            //Y
            for (int r = 0; r < pmfxOutSurface->Info.CropH; r++) {
                fwrite(pmfxOutSurface->Data.Y + r * pmfxOutSurface->Data.Pitch,
                       1,
                       pmfxOutSurface->Info.CropW,
                       fSink);
            }
            //U
            for (int r = 0; r < pmfxOutSurface->Info.CropH / 2; r++) {
                fwrite(
                    pmfxOutSurface->Data.U + r * pmfxOutSurface->Data.Pitch / 2,
                    1,
                    pmfxOutSurface->Info.CropW / 2,
                    fSink);
            }
            //V
            for (int r = 0; r < pmfxOutSurface->Info.CropH / 2; r++) {
                fwrite(
                    pmfxOutSurface->Data.V + r * pmfxOutSurface->Data.Pitch / 2,
                    1,
                    pmfxOutSurface->Info.CropW / 2,
                    fSink);
            }
        }
        framenum++;
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
