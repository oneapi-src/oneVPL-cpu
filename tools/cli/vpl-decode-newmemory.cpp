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
#include <vector>
#include "vpl/mfxjpeg.h"
#include "vpl/mfxvideo.h"

#define MAX_PATH 260

mfxStatus ReadEncodedStream(mfxBitstream &bs, mfxU32 codecid, FILE *f);
void WriteRawFrame(mfxFrameSurface1 *pSurface, FILE *f);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
int GetFreeSurfaceIndex(mfxFrameSurface1 *SurfacesPool, mfxU16 nPoolSize);
char *ValidateFileName(char *in);
void Usage(void);

enum MemoryMode {
    MEM_MODE_UNKNOWN = -1,

    MEM_MODE_EXTERNAL = 0,
    MEM_MODE_INTERNAL,
    MEM_MODE_AUTO,
};

int main(int argc, char *argv[]) {
    if (argc != 5) {
        Usage();
        return 1; // return 1 as error code
    }

    char *in_filename  = NULL;
    char *out_filename = NULL;
    mfxU32 codecID;
    MemoryMode memoryMode = MEM_MODE_UNKNOWN;

    if (strncmp("h265", argv[1], 4) == 0) {
        codecID = MFX_CODEC_HEVC;
        puts("h265 decoding");
    }
    else if (strncmp("h264", argv[1], 4) == 0) {
        codecID = MFX_CODEC_AVC;
        puts("h264 decoding");
    }
    else if (strncmp("jpeg", argv[1], 4) == 0) {
        codecID = MFX_CODEC_JPEG;
        puts("jpeg decoding");
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

    in_filename = ValidateFileName(argv[2]);
    if (!in_filename) {
        printf("Input filename is not valid\n");
        Usage();
        return 1;
    }

    out_filename = ValidateFileName(argv[3]);
    if (!out_filename) {
        printf("Output filename is not valid\n");
        Usage();
        return 1;
    }

    if (strncmp("ext", argv[4], 3) == 0) {
        memoryMode = MEM_MODE_EXTERNAL;
        puts("Memory mode = MEM_MODE_EXTERNAL\n");
    }
    else if (strncmp("int", argv[4], 3) == 0) {
        memoryMode = MEM_MODE_INTERNAL;
        puts("Memory mode = MEM_MODE_INTERNAL\n");
    }
    else if (strncmp("auto", argv[4], 4) == 0) {
        memoryMode = MEM_MODE_AUTO;
        puts("Memory mode = MEM_MODE_AUTO\n");
    }
    else {
        printf("Memory mode is not valid\n");
        Usage();
        return 1;
    }

    printf("opening %s\n", in_filename);
    FILE *fSource = fopen(in_filename, "rb");
    if (!fSource) {
        printf("could not open input file, %s\n", in_filename);
        return 1;
    }
    FILE *fSink = fopen(out_filename, "wb");
    if (!fSink) {
        fclose(fSource);
        printf("could not open output file, %s\n", out_filename);
        return 1;
    }

    // initialize  session
    mfxInitParam initPar   = { 0 };
    initPar.Version.Major  = 2;
    initPar.Version.Minor  = 0;
    initPar.Implementation = MFX_IMPL_SOFTWARE;

    mfxSession session;
    mfxStatus sts = MFXInitEx(initPar, &session);
    if (sts != MFX_ERR_NONE) {
        fclose(fSource);
        fclose(fSink);
        puts("MFXInitEx error.  Could not initialize session");
        return 1;
    }

    puts("library initialized");

    // prepare input bitstream
    mfxBitstream mfxBS  = { 0 };
    mfxBS.MaxLength     = 2000000;
    mfxU8 *input_buffer = new mfxU8[mfxBS.MaxLength];
    mfxBS.Data          = input_buffer;

    ReadEncodedStream(mfxBS, codecID, fSource);

    mfxVideoParam mfxDecParams = { 0 };

    // do lazy-init for AUTO mode
    if (memoryMode == MEM_MODE_AUTO) {
        mfxBS.CodecId = codecID;
    }
    else {
        // initialize decode parameters from stream header
        mfxDecParams;
        memset(&mfxDecParams, 0, sizeof(mfxDecParams));
        mfxDecParams.mfx.CodecId = codecID;
        mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
        sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
        if (sts != MFX_ERR_NONE) {
            fclose(fSource);
            fclose(fSink);
            printf("Problem decoding header.  DecodeHeader sts=%d\n", sts);
            return 1;
        }

        // input parameters finished, now initialize decode
        sts = MFXVideoDECODE_Init(session, &mfxDecParams);
        if (sts != MFX_ERR_NONE) {
            fclose(fSource);
            fclose(fSink);
            puts("Could not initialize decode");
            exit(1);
        }
    }

    mfxFrameAllocRequest DecRequest = { 0 };
    mfxU16 nSurfNumDec              = 0;
    mfxFrameSurface1 *decSurfaces   = nullptr;
    int nIndex                      = -1;
    mfxU8 *DECoutbuf                = nullptr;

    if (memoryMode == MEM_MODE_EXTERNAL) {
        // Query number required surfaces for decoder
        DecRequest = { 0 };
        MFXVideoDECODE_QueryIOSurf(session, &mfxDecParams, &DecRequest);

        // Determine the required number of surfaces for decoder output
        nSurfNumDec = DecRequest.NumFrameSuggested;

        decSurfaces = new mfxFrameSurface1[nSurfNumDec];

        // initialize surface pool for decode (I420 format)
        mfxU32 surfaceSize = GetSurfaceSize(mfxDecParams.mfx.FrameInfo.FourCC,
                                            mfxDecParams.mfx.FrameInfo.Width,
                                            mfxDecParams.mfx.FrameInfo.Height);
        if (surfaceSize == 0) {
            if (decSurfaces)
                delete[] decSurfaces;
            fclose(fSource);
            fclose(fSink);
            puts("Surface size is wrong");
            return 1;
        }
        size_t framePoolBufSize =
            static_cast<size_t>(surfaceSize) * nSurfNumDec;
        DECoutbuf = new mfxU8[framePoolBufSize];

        mfxU16 surfW = (mfxDecParams.mfx.FrameInfo.FourCC == MFX_FOURCC_I010)
                           ? mfxDecParams.mfx.FrameInfo.Width * 2
                           : mfxDecParams.mfx.FrameInfo.Width;
        mfxU16 surfH = mfxDecParams.mfx.FrameInfo.Height;

        for (mfxU32 i = 0; i < nSurfNumDec; i++) {
            decSurfaces[i]        = { 0 };
            decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
            size_t buf_offset     = static_cast<size_t>(i) * surfaceSize;
            decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
            decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
            decSurfaces[i].Data.V =
                decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
            decSurfaces[i].Data.Pitch = surfW;
        }
    }

    // ------------------
    // main loop
    // ------------------
    int framenum                      = 0;
    double decode_time                = 0;
    double sync_time                  = 0;
    mfxSyncPoint syncp                = { 0 };
    mfxFrameSurface1 *pmfxWorkSurface = nullptr;
    mfxFrameSurface1 *pmfxOutSurface  = nullptr;

    puts("start decoding");
    bool isdraining = false;
    for (;;) {
        bool stillgoing = true;

        if (memoryMode == MEM_MODE_EXTERNAL) {
            nIndex = GetFreeSurfaceIndex(decSurfaces, nSurfNumDec);
        }

        while (stillgoing) {
            // submit async decode request
            auto t0 = std::chrono::high_resolution_clock::now();

            if (memoryMode == MEM_MODE_EXTERNAL) {
                pmfxWorkSurface = &decSurfaces[nIndex];
            }
            else if (memoryMode == MEM_MODE_INTERNAL) {
                sts = MFXMemory_GetSurfaceForDecode(session, &pmfxWorkSurface);
                if (sts) {
                    printf("Error in GetSurfaceForDecode: sts=%d\n", sts);
                    exit(1);
                }
            }
            else if (memoryMode == MEM_MODE_AUTO) {
                pmfxWorkSurface = nullptr;
            }

            sts =
                MFXVideoDECODE_DecodeFrameAsync(session,
                                                (isdraining ? nullptr : &mfxBS),
                                                pmfxWorkSurface,
                                                &pmfxOutSurface,
                                                &syncp);

            auto t1 = std::chrono::high_resolution_clock::now();
            decode_time +=
                std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
                    .count();

            // next step actions provided by application
            switch (sts) {
                case MFX_ERR_MORE_DATA: // more data is needed to decode
                    ReadEncodedStream(mfxBS, codecID, fSource);
                    if (mfxBS.DataLength == 0) {
                        if (isdraining == true)
                            stillgoing =
                                false; // stop if end of file and all drained
                        else
                            isdraining = true;
                    }
                    if (memoryMode == MEM_MODE_INTERNAL)
                        pmfxWorkSurface->FrameInterface->Release(
                            pmfxWorkSurface);

                    break;
                case MFX_ERR_MORE_SURFACE: // feed a fresh surface to decode
                    if (memoryMode == MEM_MODE_EXTERNAL) {
                        nIndex = GetFreeSurfaceIndex(decSurfaces, nSurfNumDec);
                    }
                    else {
                        printf("Error - MORE_SURFACE with internal memory\n");
                    }
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

        if (memoryMode == MEM_MODE_INTERNAL || memoryMode == MEM_MODE_AUTO) {
            pmfxOutSurface->FrameInterface->Map(pmfxOutSurface, MFX_MAP_READ);
        }

        // write output if output file specified
        if (fSink)
            WriteRawFrame(pmfxOutSurface, fSink);

        if (memoryMode == MEM_MODE_INTERNAL || memoryMode == MEM_MODE_AUTO) {
            pmfxOutSurface->FrameInterface->Unmap(pmfxOutSurface);
            pmfxOutSurface->FrameInterface->Release(pmfxOutSurface);
        }

        framenum++;
    }

    printf("read %d frames\n", framenum);
    if (framenum) {
        printf("decode avg=%f usec, sync avg=%f usec\n",
               decode_time / framenum,
               sync_time / framenum);
    }

    fclose(fSink);
    fclose(fSource);
    MFXVideoDECODE_Close(session);
    MFXClose(session);

    if (memoryMode == MEM_MODE_EXTERNAL) {
        delete[] DECoutbuf;
        delete[] decSurfaces;
    }
    delete[] input_buffer;

    return 0;
}

mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
    mfxU32 nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_I420:
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

mfxStatus ReadEncodedStream(mfxBitstream &bs, mfxU32 codecid, FILE *f) {
    memmove(bs.Data, bs.Data + bs.DataOffset, bs.DataLength);

    if (codecid == MFX_CODEC_AV1) {
        // spec for IVF headers
        // https://wiki.multimedia.cx/index.php/IVF

        // extract AV1 elementary stream from IVF file.
        // should remove stream header and frame header.
        // and use the frame size information to read AV1 frame.
        mfxU32 nBytesInFrame = 0;
        mfxU64 nTimeStamp    = 0;
        mfxU32 nBytesRead    = 0;
        static bool bread_streamheader;

        // read stream header, only once at the beginning, 32 bytes
        if (bread_streamheader == false) {
            mfxU8 header[32] = { 0 };
            nBytesRead       = (mfxU32)fread(header, 1, 32, f);
            if (nBytesRead == 0)
                return MFX_ERR_MORE_DATA;
            bread_streamheader = true;
        }

        bs.DataLength = 0;

        // read frame header and parse frame data
        while (!feof(f)) {
            nBytesRead = (mfxU32)fread(&nBytesInFrame, 1, 4, f);
            if (nBytesInFrame == 0 || nBytesInFrame > bs.MaxLength)
                return MFX_ERR_ABORTED;
            if (nBytesRead == 0)
                return MFX_ERR_MORE_DATA;

            // check whether buffer is over if we read this frame or not
            if (bs.DataLength + nBytesInFrame > bs.MaxLength) {
                // set file pointer location back to -4
                // so, the access pointer for next frame will be the "bytesinframe" location and break
                fseek(f, -4, SEEK_CUR);
                break;
            }
            nBytesRead = (mfxU32)fread(&nTimeStamp, 1, 8, f);
            if (nBytesRead == 0)
                return MFX_ERR_MORE_DATA;

            nBytesRead =
                (mfxU32)fread(bs.Data + bs.DataLength, 1, nBytesInFrame, f);
            if (nBytesRead == 0)
                return MFX_ERR_MORE_DATA;

            bs.DataLength += nBytesRead;
        }
    }
    else {
        bs.DataLength = static_cast<mfxU32>(fread(bs.Data, 1, bs.MaxLength, f));
    }
    return MFX_ERR_NONE;
}

void WriteRawFrame(mfxFrameSurface1 *pSurface, FILE *f) {
    mfxU16 w, h, i, pitch;
    mfxFrameInfo *pInfo = &pSurface->Info;
    mfxFrameData *pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    // write the output to disk
    switch (pInfo->FourCC) {
        case MFX_FOURCC_I420:
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

char *ValidateFileName(char *in) {
    if (in) {
        if (strlen(in) > MAX_PATH)
            return NULL;
    }

    return in;
}

void Usage(void) {
    printf(
        "Usage: vpl-decode-newmemory [decoder] [input filename] [out filename] [memory mode]\n\n");
    printf("\t[decoder]         : h265|av1|jpeg|h264\n");
    printf("\t[input filename]  : encoded stream\n");
    printf("\t[out filename]    : filename to store the output\n");
    printf("\t[memory mode]     : ext|int|auto\n");
    printf("\t  ext  = external memory (1.0 style)\n");
    printf("\t  int  = internal memory with MFXMemory_GetSurfaceForDecode\n");
    printf("\t  auto = internal memory with NULL working surface\n");
    return;
}
