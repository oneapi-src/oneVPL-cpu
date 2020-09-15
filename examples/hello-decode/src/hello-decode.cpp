//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) decode application,
/// using oneVPL internal memory management
///
/// @file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#define MAX_PATH              260
#define OUTPUT_FILE           "out.i420"
#define WAIT_100_MILLSECONDS  100
#define BITSTREAM_BUFFER_SIZE 2000000

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        goto end;          \
    }

mfxStatus ReadEncodedStream(mfxBitstream &bs, FILE *f);
void WriteRawFrame(mfxFrameSurface1 *surface, FILE *f);
char *ValidateFileName(char *in);

// Print usage message
void Usage(void) {
    printf("Usage: hello-decode SOURCE\n\n"
           "Decode H265/HEVC video in SOURCE "
           "to I420 raw video in %s\n\n"
           "To view:\n"
           " ffplay -video_size [width]x[height] "
           "-pixel_format yuv420p -f rawvideo %s\n",
           OUTPUT_FILE,
           OUTPUT_FILE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        Usage();
        return 1;
    }

    char *in_filename                 = NULL;
    FILE *source                      = NULL;
    FILE *sink                        = NULL;
    mfxLoader loader                  = NULL;
    mfxConfig cfg                     = NULL;
    mfxVariant ImplValue              = { 0 };
    mfxSession session                = NULL;
    mfxStatus sts                     = MFX_ERR_NONE;
    mfxBitstream bitstream            = { 0 };
    int framenum                      = 0;
    mfxSyncPoint syncp                = { 0 };
    mfxFrameSurface1 *pmfxWorkSurface = NULL;
    mfxFrameSurface1 *pmfxOutSurface  = NULL;
    bool isdraining                   = false;
    bool stillgoing                   = true;
    mfxU32 codec_id                   = MFX_CODEC_HEVC;

    // Setup input and output files
    in_filename = ValidateFileName(argv[1]);
    VERIFY(in_filename, "Input filename is not valid");

    source = fopen(in_filename, "rb");
    VERIFY(source, "Could not open input file");

    sink = fopen(OUTPUT_FILE, "wb");
    VERIFY(sink, "Could not create output file");

    // Initialize VPL session for any implementation of HEVC
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = codec_id;
    sts                = MFXSetConfigFilterProperty(
        cfg,
        (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session supporting HEVC");

    // Prepare input bitstream and start decoding
    bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    bitstream.Data      = (mfxU8 *)malloc(bitstream.MaxLength * sizeof(mfxU8));
    bitstream.CodecId   = codec_id;
    ReadEncodedStream(bitstream, source);

    printf("Decoding %s -> %s\n", in_filename, OUTPUT_FILE);
    while (stillgoing) {
        sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                              (isdraining) ? NULL : &bitstream,
                                              NULL,
                                              &pmfxOutSurface,
                                              &syncp);

        switch (sts) {
            case MFX_ERR_MORE_DATA:
                if (isdraining)
                    stillgoing = false;
                else {
                    ReadEncodedStream(bitstream, source);
                    if (bitstream.DataLength == 0) {
                        isdraining = true;
                    }
                }
                break;

            case MFX_ERR_NONE:
                do {
                    sts = pmfxOutSurface->FrameInterface->Synchronize(pmfxOutSurface,
                                                                      WAIT_100_MILLSECONDS);
                    if (MFX_ERR_NONE == sts) {
                        sts = pmfxOutSurface->FrameInterface->Map(pmfxOutSurface, MFX_MAP_READ);
                        VERIFY(MFX_ERR_NONE == sts, "mfxFrameSurfaceInterface->Map failed");

                        WriteRawFrame(pmfxOutSurface, sink);

                        sts = pmfxOutSurface->FrameInterface->Unmap(pmfxOutSurface);
                        VERIFY(MFX_ERR_NONE == sts, "mfxFrameSurfaceInterface->Unmap failed");

                        sts = pmfxOutSurface->FrameInterface->Release(pmfxOutSurface);
                        VERIFY(MFX_ERR_NONE == sts, "mfxFrameSurfaceInterface->Release failed");

                        framenum++;
                    }
                } while (sts == MFX_WRN_IN_EXECUTION);
                break;

            default:
                printf("Error in DecodeFrameAsync: sts=%d\n", sts);
                stillgoing = false;
                break;
        }
    }

end:
    printf("Decoded %d frames\n", framenum);

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

// Read encoded stream from file
mfxStatus ReadEncodedStream(mfxBitstream &bs, FILE *f) {
    memmove(bs.Data, bs.Data + bs.DataOffset, bs.DataLength);
    bs.DataOffset = 0;
    bs.DataLength += (mfxU32)fread(bs.Data + bs.DataLength, 1, bs.MaxLength - bs.DataLength, f);
    return MFX_ERR_NONE;
}

// Write raw I420 frame to file
void WriteRawFrame(mfxFrameSurface1 *surface, FILE *f) {
    mfxU16 w, h, i, pitch;
    mfxFrameInfo *info = &surface->Info;
    mfxFrameData *data = &surface->Data;

    w = info->Width;
    h = info->Height;

    // write the output to disk
    switch (info->FourCC) {
        case MFX_FOURCC_I420:
            // Y
            pitch = data->Pitch;
            for (i = 0; i < h; i++) {
                fwrite(data->Y + i * pitch, 1, w, f);
            }
            // U
            pitch /= 2;
            h /= 2;
            w /= 2;
            for (i = 0; i < h; i++) {
                fwrite(data->U + i * pitch, 1, w, f);
            }
            // V
            for (i = 0; i < h; i++) {
                fwrite(data->V + i * pitch, 1, w, f);
            }
            break;

        default:
            printf("Unsupported FourCC code, skip WriteRawFrame\n");
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
