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

#define MAX_LENGTH             260
#define MAX_WIDTH              3840
#define MAX_HEIGHT             2160
#define MAX_BS_BUFFER_SIZE     64 * 1024 * 1024
#define DEFAULT_BS_BUFFER_SIZE 2 * 1024 * 21024

#define IS_ARG_EQ(a, b) (!strcmp((a), (b)))

enum MemoryMode {
    MEM_MODE_UNKNOWN = -1,

    MEM_MODE_EXTERNAL = 0,
    MEM_MODE_INTERNAL,
    MEM_MODE_AUTO,
};

const char* MemoryModeString[] = { "MEM_MODE_EXTERNAL",
                                   "MEM_MODE_INTERNAL",
                                   "MEM_MODE_AUTO" };

typedef struct _Params {
    char* infileName;
    char* outfileName;

    char* infileFormat;
    char* outfileFormat;

    char* targetDeviceType;

    char* gpuCopyMode;

    MemoryMode memoryMode;

    mfxU32 srcFourCC;
    mfxU32 dstFourCC;

    mfxU32 maxFrames;
    mfxU32 srcWidth;
    mfxU32 srcHeight;
    mfxU32 dstWidth;
    mfxU32 dstHeight;
    mfxU32 timeout;
    mfxU32 frameRate;
    mfxU32 enableCinterface;

    mfxU32 srcbsbufSize;
    mfxU32 dstbsbufSize;

    // encoder specific
    mfxU32 bitRate;
    mfxU32 targetUsage;
    mfxU32 brcMode;
    mfxU32 gopSize;
    mfxU32 keyFrameDist;

    // jpeg encoder specific
    mfxU32 quality;

    mfxU32 targetDevice;

    mfxU32 gpuCopy;

    // cropping
    mfxU32 srcCropX;
    mfxU32 srcCropY;
    mfxU32 srcCropW;
    mfxU32 srcCropH;
    mfxU32 dstCropX;
    mfxU32 dstCropY;
    mfxU32 dstCropW;
    mfxU32 dstCropH;

} Params;

mfxStatus ReadEncodedStream(mfxBitstream& bs, mfxU32 codecid, FILE* f);
void WriteRawFrame(mfxFrameSurface1* pSurface, FILE* f);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
int GetFreeSurfaceIndex(mfxFrameSurface1* SurfacesPool, mfxU16 nPoolSize);
char** ValidateInput(int cnt, char* in[]);
void str_upper(char* str, int l);
char* ValidateFileName(char* in);
bool ValidateSize(char* in, mfxU32* vsize, mfxU32 vmax);
bool ValidateParams(Params* params);
bool ParseArgsAndValidate(int argc, char* argv[], Params* params);
void Usage(void);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        Usage();
        return 1; // return 1 as error code
    }

    char** cmd_args;
    cmd_args = ValidateInput(argc, argv);
    if (cmd_args == NULL) {
        Usage();
        return 1; // return 1 as error code
    }

    Params params = { 0 };
    if (ParseArgsAndValidate(argc, cmd_args, &params) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    printf("opening %s\n", params.infileName);

    FILE* fSource = fopen(params.infileName, "rb");
    if (!fSource) {
        printf("could not open input file, %s\n", params.infileName);
        return 1;
    }

    FILE* fSink = fopen(params.outfileName, "wb");
    if (!fSink) {
        fclose(fSource);
        printf("could not create output file, %s\n", params.outfileName);
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
        return sts;
    }

    printf("Memory mode = %s\n", MemoryModeString[params.memoryMode]);
    puts("library initialized");

    // prepare input bitstream
    mfxBitstream mfxBS = { 0 };
    if (params.srcbsbufSize > 0 && params.srcbsbufSize <= MAX_BS_BUFFER_SIZE)
        mfxBS.MaxLength = params.srcbsbufSize;
    else {
        fclose(fSource);
        fclose(fSink);
        puts("Memory allocation error. Bitstream buffersize is not correct");
        return 1;
    }

    std::vector<mfxU8> input_buffer;
    input_buffer.resize(mfxBS.MaxLength);
    mfxBS.Data = input_buffer.data();

    ReadEncodedStream(mfxBS, params.srcFourCC, fSource);

    mfxVideoParam mfxDecParams = { 0 };

    if (params.memoryMode == MEM_MODE_AUTO) {
        // do lazy-init for AUTO mode - only need codec type
        mfxBS.CodecId = params.srcFourCC;
    }
    else {
        // initialize decode parameters from stream header
        memset(&mfxDecParams, 0, sizeof(mfxDecParams));
        mfxDecParams.mfx.CodecId = params.srcFourCC;
        mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
        sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
        if (sts != MFX_ERR_NONE) {
            fclose(fSource);
            fclose(fSink);
            printf("Problem decoding header.  DecodeHeader sts=%d\n", sts);
            return sts;
        }

        // input parameters finished, now initialize decode
        sts = MFXVideoDECODE_Init(session, &mfxDecParams);
        if (sts != MFX_ERR_NONE) {
            fclose(fSource);
            fclose(fSink);
            puts("Could not initialize decode");
            exit(sts);
        }
    }

    // Query number required surfaces for decoder
    mfxFrameAllocRequest DecRequest = { 0 };
    mfxU16 nSurfNumDec              = 0;
    mfxFrameSurface1* decSurfaces   = nullptr;
    int nIndex                      = -1;
    mfxU8* DECoutbuf                = nullptr;

    if (params.memoryMode == MEM_MODE_EXTERNAL) {
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
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxFrameSurface1* pmfxOutSurface  = nullptr;

    puts("start decoding");
    bool isdraining = false;
    for (;;) {
        bool stillgoing = true;

        if (params.memoryMode == MEM_MODE_EXTERNAL) {
            nIndex = GetFreeSurfaceIndex(decSurfaces, nSurfNumDec);
        }

        while (stillgoing) {
            // submit async decode request
            auto t0 = std::chrono::high_resolution_clock::now();
            if (params.memoryMode == MEM_MODE_EXTERNAL) {
                pmfxWorkSurface = &decSurfaces[nIndex];
            }
            else if (params.memoryMode == MEM_MODE_INTERNAL) {
                sts = MFXMemory_GetSurfaceForDecode(session, &pmfxWorkSurface);
                if (sts) {
                    printf("Error in GetSurfaceForDecode: sts=%d\n", sts);
                    exit(1);
                }
            }
            else if (params.memoryMode == MEM_MODE_AUTO) {
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
                    ReadEncodedStream(mfxBS, params.srcFourCC, fSource);
                    if (mfxBS.DataLength == 0)
                        if (isdraining == true) {
                            stillgoing =
                                false; // stop if end of file and all drained
                        }
                        else {
                            isdraining = true;
                        }

                    if (params.memoryMode == MEM_MODE_INTERNAL)
                        pmfxWorkSurface->FrameInterface->Release(
                            pmfxWorkSurface);

                    break;
                case MFX_ERR_MORE_SURFACE: // feed a fresh surface to decode
                    if (params.memoryMode == MEM_MODE_EXTERNAL) {
                        nIndex = GetFreeSurfaceIndex(decSurfaces, nSurfNumDec);
                    }
                    else {
                        printf(
                            "Error - MFX_ERR_MORE_SURFACE returned with internal memory allocation\n");
                        exit(1);
                    }
                    break;
                case MFX_ERR_NONE: // no more steps needed, exit loop
                    stillgoing = false;
                    break;
                default: // state is not one of the cases above
                    printf("Error in DecodeFrameAsync: and exit sts=%d\n", sts);
                    exit(sts);
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

        if (params.memoryMode == MEM_MODE_INTERNAL ||
            params.memoryMode == MEM_MODE_AUTO) {
            pmfxOutSurface->FrameInterface->Map(pmfxOutSurface, MFX_MAP_READ);
        }

        // write output if output file specified
        if (fSink)
            WriteRawFrame(pmfxOutSurface, fSink);

        if (params.memoryMode == MEM_MODE_INTERNAL ||
            params.memoryMode == MEM_MODE_AUTO) {
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
    if (params.memoryMode == MEM_MODE_EXTERNAL) {
        delete[] DECoutbuf;
        delete[] decSurfaces;
    }

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

int GetFreeSurfaceIndex(mfxFrameSurface1* SurfacesPool, mfxU16 nPoolSize) {
    for (mfxU16 i = 0; i < nPoolSize; i++) {
        if (0 == SurfacesPool[i].Data.Locked)
            return i;
    }
    return MFX_ERR_NOT_FOUND;
}

mfxStatus ReadEncodedStream(mfxBitstream& bs, mfxU32 codecid, FILE* f) {
    memmove(bs.Data, bs.Data + bs.DataOffset, bs.DataLength);
    bs.DataOffset = 0;

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
        bs.DataLength += static_cast<mfxU32>(
            fread(bs.Data + bs.DataLength, 1, bs.MaxLength - bs.DataLength, f));
    }
    return MFX_ERR_NONE;
}

void WriteRawFrame(mfxFrameSurface1* pSurface, FILE* f) {
    mfxU16 w, h, i, pitch;
    mfxFrameInfo* pInfo = &pSurface->Info;
    mfxFrameData* pData = &pSurface->Data;

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

char** ValidateInput(int cnt, char* in[]) {
    if (in) {
        for (int i = 0; i < cnt; i++) {
            if (strlen(in[i]) > MAX_LENGTH)
                return NULL;
        }
    }

    return in;
}

void str_upper(char* str, int l) {
    for (int i = 0; i < l; i++) {
        str[i] = static_cast<char>(toupper(str[i]));
    }
}

char* ValidateFileName(char* in) {
    if (in) {
        if (strlen(in) > MAX_LENGTH)
            return NULL;
    }

    return in;
}

bool ValidateSize(char* in, mfxU32* vsize, mfxU32 vmax) {
    if (in) {
        *vsize = static_cast<mfxU32>(strtol(in, NULL, 10));
        if (*vsize > vmax)
            return false;
        else
            return true;
    }

    return false;
}

// perform basic parameter validation and setup
bool ValidateParams(Params* params) {
    // input file (required)
    if (!params->infileName) {
        printf("ERROR - input file name (-i) is required\n");
        return false;
    }

    // output file (required)
    if (!params->outfileName) {
        printf("ERROR - output file name (-i) is required\n");
        return false;
    }

    // input format (required)
    if (!params->infileFormat) {
        printf("ERROR - input format (-if) is required\n");
        return false;
    }

    // input format (required)
    if (!strncmp(params->infileFormat, "H264", strlen("H264"))) {
        params->srcFourCC = MFX_CODEC_AVC;
    }
    else if (!strncmp(params->infileFormat, "H265", strlen("H265"))) {
        params->srcFourCC = MFX_CODEC_HEVC;
    }
    else if (!strncmp(params->infileFormat, "AV1", strlen("AV1"))) {
        params->srcFourCC = MFX_CODEC_AV1;
    }
    else if (!strncmp(params->infileFormat, "JPEG", strlen("JPEG"))) {
        params->srcFourCC = MFX_CODEC_JPEG;
    }
    else {
        printf("ERROR - unsupported input format %s\n", params->infileFormat);
        return false;
    }

    // default bitstream buffer size (input)
    if (params->srcbsbufSize == 0) {
        params->srcbsbufSize = DEFAULT_BS_BUFFER_SIZE;
    }

    return true;
}

bool ParseArgsAndValidate(int argc, char* argv[], Params* params) {
    int idx;
    char* s;

    // init all params to 0
    memset(params, 0, sizeof(Params));

    // set any non-zero defaults
    params->memoryMode = MEM_MODE_EXTERNAL;

    if (argc < 2)
        return false;

    for (idx = 1; idx < argc;) {
        // all switches must start with '-'
        if (argv[idx][0] != '-') {
            printf("ERROR - invalid argument: %s\n", argv[idx]);
            return false;
        }

        // switch string, starting after the '-'
        s = &argv[idx][1];
        idx++;

        // search for match
        if (IS_ARG_EQ(s, "i")) {
            params->infileName = ValidateFileName(argv[idx++]);
            if (!params->infileName) {
                return false;
            }
        }
        else if (IS_ARG_EQ(s, "o")) {
            params->outfileName = ValidateFileName(argv[idx++]);
            if (!params->outfileName) {
                return false;
            }
        }
        else if (IS_ARG_EQ(s, "n")) {
            params->maxFrames = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "if")) {
            params->infileFormat = argv[idx++];
            str_upper(params->infileFormat,
                      static_cast<int>(
                          strlen(params->infileFormat))); // to upper case
        }
        else if (IS_ARG_EQ(s, "of")) {
            params->outfileFormat = argv[idx++];
            str_upper(params->outfileFormat,
                      static_cast<int>(
                          strlen(params->outfileFormat))); // to upper case
        }
        else if (IS_ARG_EQ(s, "sw")) {
            if (!ValidateSize(argv[idx++], &params->srcWidth, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "sh")) {
            if (!ValidateSize(argv[idx++], &params->srcHeight, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "dw")) {
            if (!ValidateSize(argv[idx++], &params->dstWidth, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "dh")) {
            if (!ValidateSize(argv[idx++], &params->dstHeight, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "td")) {
            params->targetDeviceType = argv[idx++];
        }
        else if (IS_ARG_EQ(s, "sbs")) {
            params->srcbsbufSize = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "dbs")) {
            params->dstbsbufSize = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "to")) {
            params->timeout = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "fr")) {
            params->frameRate = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "br")) {
            params->bitRate = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "tu")) {
            params->targetUsage = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "qu")) {
            params->quality = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "bm")) {
            params->brcMode = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "gs")) {
            params->gopSize = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "kd")) {
            params->keyFrameDist = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "ci")) {
            params->enableCinterface = 1;
        }
        else if (IS_ARG_EQ(s, "gcm")) {
            params->gpuCopyMode = argv[idx++];
        }
        else if (IS_ARG_EQ(s, "ext")) {
            params->memoryMode = MEM_MODE_EXTERNAL;
        }
        else if (IS_ARG_EQ(s, "int")) {
            params->memoryMode = MEM_MODE_INTERNAL;
        }
        else if (IS_ARG_EQ(s, "auto")) {
            params->memoryMode = MEM_MODE_AUTO;
        }
        else if (IS_ARG_EQ(s, "scrx")) {
            if (!ValidateSize(argv[idx++], &params->srcCropX, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "scry")) {
            if (!ValidateSize(argv[idx++], &params->srcCropY, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "scrw")) {
            if (!ValidateSize(argv[idx++], &params->srcCropW, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "scrh")) {
            if (!ValidateSize(argv[idx++], &params->srcCropH, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "dcrx")) {
            if (!ValidateSize(argv[idx++], &params->dstCropX, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "dcry")) {
            if (!ValidateSize(argv[idx++], &params->dstCropY, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "dcrw")) {
            if (!ValidateSize(argv[idx++], &params->dstCropW, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "dcrh")) {
            if (!ValidateSize(argv[idx++], &params->dstCropH, MAX_HEIGHT))
                return false;
        }
        else {
            printf("ERROR - invalid argument: %s\n", argv[idx]);
            return false;
        }
    }

    // run basic parameter validation
    return ValidateParams(params);
}

void Usage(void) {
    printf("\nOptions - Decode:\n");
    printf("  -i     inputFile     ... input file name\n");
    printf("  -o     outputFile    ... output file name\n");
    printf("  -n     maxFrames     ... max frames to decode\n");
    printf("  -if    inputFormat   ... [h264, h265, av1, jpeg]\n");
    printf("  -sbs   bsbufSize     ... source bitstream buffer size (bytes)\n");

    printf("\nMemory model (default = -ext)\n");
    printf("  -ext  = external memory (1.0 style)\n");
    printf("  -int  = internal memory with MFXMemory_GetSurfaceForDecode\n");
    printf(
        "  -auto = internal memory with NULL working surface + simplified decode path\n");

    printf("\nTo view:\n");
    printf(
        " ffplay -video_size [width]x[height] -pixel_format [pixel format] -f rawvideo [out filename]\n");
    return;
}
