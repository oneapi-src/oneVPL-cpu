//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) encode application,
/// using oneVPL internal memory management
///
/// @file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#define MAX_PATH              260
#define MAX_WIDTH             3840
#define MAX_HEIGHT            2160
#define TARGETKBPS            4000
#define FRAMERATE             30
#define OUTPUT_FILE           "out.h265"
#define WAIT_100_MILLSECONDS  100
#define BITSTREAM_BUFFER_SIZE 2000000

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        goto end;          \
    }

#define ALIGN16(value) (((value + 15) >> 4) << 4)

mfxStatus LoadRawFrame(mfxFrameSurface1 *surface, FILE *f);
void WriteEncodedStream(mfxBitstream &bs, FILE *f);
char *ValidateFileName(char *in);
mfxU16 ValidateSize(char *in, mfxU16 max);

// Print usage message
void Usage(void) {
    printf("Usage: hello-encode SOURCE width height\n\n"
           "Encode SOURCE i420 raw frames "
           "to HEVC/H265 elementary stream in %s\n\n"
           "To view:\n"
           " ffplay %s\n",
           OUTPUT_FILE,
           OUTPUT_FILE);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        Usage();
        return 1;
    }

    char *in_filename                = NULL;
    FILE *source                     = NULL;
    FILE *sink                       = NULL;
    mfxLoader loader                 = NULL;
    mfxConfig cfg                    = NULL;
    mfxVariant ImplValue             = { 0 };
    mfxSession session               = NULL;
    mfxStatus sts                    = MFX_ERR_NONE;
    mfxBitstream bitstream           = { 0 };
    mfxVideoParam encode_params      = { 0 };
    int framenum                     = 0;
    mfxSyncPoint syncp               = { 0 };
    mfxFrameSurface1 *pmfxOutSurface = NULL;
    bool isdraining                  = false;
    bool stillgoing                  = true;
    mfxU32 codec_id                  = MFX_CODEC_HEVC;
    mfxU16 input_width               = 0;
    mfxU16 input_height              = 0;

    // Setup input and output files
    in_filename = ValidateFileName(argv[1]);
    VERIFY(in_filename, "Input filename is not valid");

    source = fopen(in_filename, "rb");
    VERIFY(source, "Could not open input file");

    sink = fopen(OUTPUT_FILE, "wb");
    VERIFY(sink, "Could not create output file");

    input_width = ValidateSize(argv[2], MAX_WIDTH);
    VERIFY(input_width, "Input width is not valid");

    input_height = ValidateSize(argv[3], MAX_HEIGHT);
    VERIFY(input_height, "Input height is not valid");

    // Initialize VPL session for any implementation of HEVC encode
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_HEVC;
    sts                = MFXSetConfigFilterProperty(
        cfg,
        (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
        ImplValue);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session supporting HEVC encode");

    // Initialize encode parameters
    encode_params.mfx.CodecId                 = MFX_CODEC_HEVC;
    encode_params.mfx.TargetUsage             = MFX_TARGETUSAGE_BALANCED;
    encode_params.mfx.TargetKbps              = TARGETKBPS;
    encode_params.mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    encode_params.mfx.FrameInfo.FrameRateExtN = FRAMERATE;
    encode_params.mfx.FrameInfo.FrameRateExtD = 1;
    encode_params.mfx.FrameInfo.FourCC        = MFX_FOURCC_I420;
    encode_params.mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    encode_params.mfx.FrameInfo.CropW         = input_width;
    encode_params.mfx.FrameInfo.CropH         = input_height;
    encode_params.mfx.FrameInfo.Width         = ALIGN16(input_width);
    encode_params.mfx.FrameInfo.Height        = ALIGN16(input_height);

    encode_params.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    // Initialize the encoder
    VERIFY(MFX_ERR_NONE == MFXVideoENCODE_Init(session, &encode_params), "Encode init failed");

    // Prepare output bitstream
    bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    bitstream.Data      = (mfxU8 *)malloc(bitstream.MaxLength * sizeof(mfxU8));

    printf("Encoding %s -> %s\n", in_filename, OUTPUT_FILE);
    while (stillgoing) {
        // Load a new frame if not draining
        if (!isdraining) {
            VERIFY(MFX_ERR_NONE == MFXMemory_GetSurfaceForEncode(session, &pmfxOutSurface),
                   "Could not get encode surface");

            // Map makes surface writable by CPU for all implementations
            sts = pmfxOutSurface->FrameInterface->Map(pmfxOutSurface, MFX_MAP_WRITE);
            VERIFY(MFX_ERR_NONE == sts, "mfxFrameSurfaceInterface->Map failed");

            sts = LoadRawFrame(pmfxOutSurface, source);
            if (sts != MFX_ERR_NONE)
                isdraining = true;

            // Unmap/release returns local device access for all implementations
            sts = pmfxOutSurface->FrameInterface->Unmap(pmfxOutSurface);
            VERIFY(MFX_ERR_NONE == sts, "mfxFrameSurfaceInterface->Unmap failed");

            pmfxOutSurface->FrameInterface->Release(pmfxOutSurface);
            VERIFY(MFX_ERR_NONE == sts, "mfxFrameSurfaceInterface->Release failed");
        }

        sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                              NULL,
                                              (isdraining ? NULL : pmfxOutSurface),
                                              &bitstream,
                                              &syncp);

        switch (sts) {
            case MFX_ERR_NONE:
                // ERR_NONE and syncp indicate output is available
                if (syncp) {
                    // encode output is not available on CPU until
                    // sync operation completes
                    sts = MFXVideoCORE_SyncOperation(session, syncp, 60000);
                    VERIFY(MFX_ERR_NONE == sts, "MFXVideoCORE_SyncOperation error");

                    WriteEncodedStream(bitstream, sink);
                    framenum++;
                }
                break;
            case MFX_ERR_NOT_ENOUGH_BUFFER:
                // This example deliberatly uses a large output buffer with
                // immediate write to disk for simplicity.
                // Handle when frame size exceeds available buffer here
                break;
            case MFX_ERR_MORE_DATA:
                if (isdraining)
                    stillgoing = false;
                break;
            case MFX_ERR_DEVICE_LOST:
                // For non-CPU implementations
                // Cleanup if device is lost
                break;
            case MFX_WRN_DEVICE_BUSY:
                // For non-CPU implementations
                // Wait a few milliseconds then try again
                break;
            case MFX_ERR_INCOMPATIBLE_VIDEO_PARAM:
                // The CPU reference implementation does not include
                // mfxEncodeCtrl, but for other implementations issues
                // with mfxEncodeCtrl parameters can be handled here
                break;
            default:
                printf("unknown status %d\n", sts);
                stillgoing = false;
        }
    }

end:
    printf("Encoded %d frames\n", framenum);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.

    if (loader)
        MFXUnload(loader);

    if (bitstream.Data)
        free(bitstream.Data);

    if (source)
        fclose(source);

    if (sink)
        fclose(sink);

    return 0;
}

void WriteEncodedStream(mfxBitstream &bs, FILE *f) {
    fwrite(bs.Data + bs.DataOffset, 1, bs.DataLength, f);
    bs.DataLength = 0;
    return;
}

mfxStatus LoadRawFrame(mfxFrameSurface1 *surface, FILE *f) {
    mfxU16 w, h, i, pitch;
    mfxU32 bytes;
    mfxU8 *ptr;
    mfxFrameInfo *info = &surface->Info;
    mfxFrameData *data = &surface->Data;

    w = info->Width;
    h = info->Height;

    switch (info->FourCC) {
        case MFX_FOURCC_I420:
            // read luminance plane (Y)
            pitch = data->Pitch;
            ptr   = data->Y;
            for (i = 0; i < h; i++) {
                bytes = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != bytes)
                    return MFX_ERR_MORE_DATA;
            }

            // read chrominance (U, V)
            pitch /= 2;
            h /= 2;
            w /= 2;
            ptr = data->U;
            for (i = 0; i < h; i++) {
                bytes = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != bytes)
                    return MFX_ERR_MORE_DATA;
            }

            ptr = data->V;
            for (i = 0; i < h; i++) {
                bytes = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != bytes)
                    return MFX_ERR_MORE_DATA;
            }
            break;
        default:
            printf("Unsupported FourCC code, skip LoadRawFrame\n");
            break;
    }

    return MFX_ERR_NONE;
}

char *ValidateFileName(char *in) {
    if (in) {
        if (strlen(in) > MAX_PATH)
            return NULL;
    }

    return in;
}

mfxU16 ValidateSize(char *in, mfxU16 max) {
    mfxI32 isize = strtol(in, NULL, 10);
    if (isize <= 0 || isize > max) {
        return 0;
    }

    return isize;
}
