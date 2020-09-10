//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) vpp application,
/// using oneVPL internal memory management
///
/// @file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#define MAX_PATH   260
#define MAX_WIDTH  3840
#define MAX_HEIGHT 2160

#define OUTPUT_FILE   "out.i420"
#define OUTPUT_WIDTH  640
#define OUTPUT_HEIGHT 480

#define WAIT_100_MILLSECONDS 100

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        goto end;          \
    }

mfxStatus LoadRawFrame(mfxFrameSurface1 *surface, FILE *f);
void WriteRawFrame(mfxFrameSurface1 *surface, FILE *f);
mfxU32 GetSurfaceSize(mfxU32 fourcc, mfxU32 width, mfxU32 height);
mfxI32 GetFreeSurfaceIndex(mfxFrameSurface1 **surface_pool, mfxU16 pool_size);
char *ValidateFileName(char *in);
mfxI32 ValidateSize(char *in, mfxI32 max);

// Print usage message
void Usage(void) {
    printf("Usage: hello-vpp SOURCE WIDTH HEIGHT\n\n"
           "Process raw I420 video in SOURCE having dimensions WIDTH x HEIGHT "
           "to resized I420 raw video in %s\n\n"
           "To view:\n"
           " ffplay -video_size [width]x[height] "
           "-pixel_format yuv420p -f rawvideo %s\n",
           OUTPUT_FILE,
           OUTPUT_FILE);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        Usage();
        return 1;
    }

    char *in_filename                   = NULL;
    FILE *source                        = NULL;
    FILE *sink                          = NULL;
    mfxLoader loader                    = NULL;
    mfxConfig cfg                       = NULL;
    mfxVariant ImplValue                = { 0 };
    mfxSession session                  = NULL;
    mfxStatus sts                       = MFX_ERR_NONE;
    mfxVideoParam vpp_params            = { 0 };
    mfxFrameAllocRequest vpp_request[2] = { 0 };
    mfxU16 num_surfaces_in;
    mfxU16 num_surfaces_out;
    mfxU8 *buffers_out = NULL;
    mfxU32 fourcc;
    mfxU32 width;
    mfxU32 height;
    mfxI32 input_width;
    mfxI32 input_height;
    mfxU32 surface_size;
    mfxFrameSurface1 *vpp_surfaces_in   = NULL;
    mfxFrameSurface1 **vpp_surfaces_out = NULL;
    bool isdraining                     = false;
    bool stillgoing                     = true;
    mfxI32 i, j;
    mfxU32 index_out  = 0;
    mfxU32 index_impl = 0;
    mfxSyncPoint syncp;
    mfxU32 framenum           = 0;
    bool vpp_support_scaling  = false;
    mfxImplDescription *idesc = NULL;

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

    // Initialize VPL session for video processing
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_EXTBUFF_VPP_SCALING;
    sts                = MFXSetConfigFilterProperty(
        cfg,
        (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC",
        ImplValue);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session");

    // Initialize VPP parameters
    // For simplistic memory management, system memory surfaces are used to
    // store the raw frames (Note that when using HW acceleration video surfaces
    // are prefered, for better performance)
    // Input data
    vpp_params.vpp.In.FourCC        = MFX_FOURCC_I420;
    vpp_params.vpp.In.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    vpp_params.vpp.In.CropX         = 0;
    vpp_params.vpp.In.CropY         = 0;
    vpp_params.vpp.In.CropW         = input_width;
    vpp_params.vpp.In.CropH         = input_height;
    vpp_params.vpp.In.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    vpp_params.vpp.In.FrameRateExtN = 30;
    vpp_params.vpp.In.FrameRateExtD = 1;
    vpp_params.vpp.In.Width         = vpp_params.vpp.In.CropW;
    vpp_params.vpp.In.Height        = vpp_params.vpp.In.CropH;
    // Output data
    vpp_params.vpp.Out.FourCC        = MFX_FOURCC_I420;
    vpp_params.vpp.Out.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    vpp_params.vpp.Out.CropX         = 0;
    vpp_params.vpp.Out.CropY         = 0;
    vpp_params.vpp.Out.CropW         = OUTPUT_WIDTH;
    vpp_params.vpp.Out.CropH         = OUTPUT_HEIGHT;
    vpp_params.vpp.Out.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    vpp_params.vpp.Out.FrameRateExtN = 30;
    vpp_params.vpp.Out.FrameRateExtD = 1;
    vpp_params.vpp.Out.Width         = vpp_params.vpp.Out.CropW;
    vpp_params.vpp.Out.Height        = vpp_params.vpp.Out.CropH;

    vpp_params.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    // Query number of required surfaces for VPP
    sts = MFXVideoVPP_QueryIOSurf(session, &vpp_params, vpp_request);
    VERIFY(MFX_ERR_NONE == sts, "QueryIOSurf error");

    num_surfaces_in  = vpp_request[0].NumFrameSuggested;
    num_surfaces_out = vpp_request[1].NumFrameSuggested;

    // Allocate surfaces for VPP: Out
    // Frame surface array keeps pointers to all surface planes and general
    // frame info
    fourcc = vpp_params.vpp.Out.FourCC;
    width  = vpp_params.vpp.Out.Width;
    height = vpp_params.vpp.Out.Height;

    surface_size = GetSurfaceSize(fourcc, width, height);
    VERIFY(surface_size, "VPP-out surface size is wrong");

    buffers_out      = (mfxU8 *)malloc(surface_size * num_surfaces_out);
    vpp_surfaces_out = (mfxFrameSurface1 **)malloc(sizeof(mfxFrameSurface1) * num_surfaces_out);

    for (i = 0; i < num_surfaces_out; i++) {
        vpp_surfaces_out[i] = (mfxFrameSurface1 *)malloc(sizeof(mfxFrameSurface1));
        memset(vpp_surfaces_out[i], 0, sizeof(mfxFrameSurface1));
        vpp_surfaces_out[i]->Info   = vpp_params.vpp.Out;
        vpp_surfaces_out[i]->Data.Y = &buffers_out[surface_size * i];
        vpp_surfaces_out[i]->Data.U = vpp_surfaces_out[i]->Data.Y + width * height;
        vpp_surfaces_out[i]->Data.V = vpp_surfaces_out[i]->Data.U + ((width / 2) * (height / 2));
        vpp_surfaces_out[i]->Data.Pitch = width;
    }

    // Initialize VPP
    sts = MFXVideoVPP_Init(session, &vpp_params);
    VERIFY(MFX_ERR_NONE == sts, "Could not initialize vpp");

    // Start processing the frames
    printf("Processing %s -> %s\n", in_filename, OUTPUT_FILE);

    // Main processing loop
    while (stillgoing) {
        vpp_surfaces_in = NULL;

        sts = MFXMemory_GetSurfaceForVPP(session, &vpp_surfaces_in);
        VERIFY(MFX_ERR_NONE == sts, "Unknown error in MFXMemory_GetSurfaceForVPP");

        sts = vpp_surfaces_in->FrameInterface->Map(vpp_surfaces_in, MFX_MAP_READ);
        VERIFY(MFX_ERR_NONE == sts, "mfxFrameSurfaceInterface->Map failed");

        index_out = GetFreeSurfaceIndex(vpp_surfaces_out,
                                        num_surfaces_out); // Find free frame surface
        VERIFY(index_out != MFX_ERR_NOT_FOUND, "No available surface");

        sts = LoadRawFrame(vpp_surfaces_in, source);
        if (sts == MFX_ERR_MORE_DATA)
            isdraining = true;

        // Process a frame asynchronously (returns immediately)
        sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                           (isdraining) ? NULL : vpp_surfaces_in,
                                           vpp_surfaces_out[index_out],
                                           NULL,
                                           &syncp);

        switch (sts) {
            case MFX_ERR_NONE:
                sts = MFXVideoCORE_SyncOperation(
                    session,
                    syncp,
                    WAIT_100_MILLSECONDS); // Synchronize. Wait until a frame is ready
                ++framenum;
                VERIFY(MFX_ERR_NONE == sts, "MFXVideoCORE_SyncOperation error");

                WriteRawFrame(vpp_surfaces_out[index_out], sink);

                if (!isdraining) {
                    vpp_surfaces_in->FrameInterface->Unmap(vpp_surfaces_in);
                    vpp_surfaces_in->FrameInterface->Release(vpp_surfaces_in);
                }
                break;

            case MFX_ERR_MORE_DATA:
                if (isdraining)
                    stillgoing = false;
                break;

            default:
                if (sts < 0) // error
                    stillgoing = false;
                break;
        }
    }

end:
    printf("Processed %d frames\n", framenum);

    if (loader)
        MFXUnload(loader);

    if (vpp_surfaces_out) {
        for (i = 0; i < num_surfaces_out; i++)
            free(vpp_surfaces_out[i]);

        free(vpp_surfaces_out);
    }

    if (source)
        fclose(source);

    if (sink)
        fclose(sink);

    return 0;
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
            break;
    }

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
            break;
    }

    return;
}

// Return the surface size in bytes given format and dimensions
mfxU32 GetSurfaceSize(mfxU32 fourcc, mfxU32 width, mfxU32 height) {
    mfxU32 bytes = 0;

    switch (fourcc) {
        case MFX_FOURCC_I420:
            bytes = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
            break;
        default:
            break;
    }

    return bytes;
}

// Return index of free surface in given pool
mfxI32 GetFreeSurfaceIndex(mfxFrameSurface1 **surface_pool, mfxU16 pool_size) {
    if (surface_pool)
        for (mfxU16 i = 0; i < pool_size; i++)
            if (0 == surface_pool[i]->Data.Locked)
                return i;
    return MFX_ERR_NOT_FOUND;
}

char *ValidateFileName(char *in) {
    if (in) {
        if (strlen(in) > MAX_PATH)
            return NULL;
    }

    return in;
}

mfxI32 ValidateSize(char *in, mfxI32 max) {
    mfxI32 isize = strtol(in, NULL, 10);
    if (isize <= 0 || isize > max) {
        return 0;
    }

    return isize;
}