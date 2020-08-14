//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) vpp application.
///
/// @file

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "vpl/mfxvideo.h"

#define MAX_PATH   260
#define MAX_WIDTH  3840
#define MAX_HEIGHT 2160

#define OUTPUT_FILE   "out.i420"
#define OUTPUT_WIDTH  640
#define OUTPUT_HEIGHT 480

mfxStatus LoadRawFrame(mfxFrameSurface1 *pSurface, FILE *fSource);
void WriteRawFrame(mfxFrameSurface1 *pSurface, FILE *f);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
mfxI32 GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1> &pSurfacesPool);
char *ValidateFileName(char *in);
void Usage(void);

int main(int argc, char *argv[]) {
    mfxU32 fourCC;
    mfxU32 width;
    mfxU32 height;

    if (argc != 4) {
        Usage();
        return 1;
    }

    char *in_filename = NULL;

    in_filename = ValidateFileName(argv[1]);
    if (!in_filename) {
        printf("Input filename is not valid\n");
        Usage();
        return 1;
    }

    FILE *fSource = fopen(in_filename, "rb");
    if (!fSource) {
        printf("Could not open input file, \"%s\"\n", in_filename);
        return 1;
    }
    FILE *fSink = fopen(OUTPUT_FILE, "wb");
    if (!fSink) {
        fclose(fSource);
        printf("Could not open output file, %s\n", OUTPUT_FILE);
        return 1;
    }
    mfxI32 isize = strtol(argv[2], NULL, 10);
    if (isize <= 0 || isize > MAX_WIDTH) {
        fclose(fSource);
        fclose(fSink);
        puts("Input size is not valid\n");
        return 1;
    }
    mfxI32 inputWidth = isize;

    isize = strtol(argv[3], NULL, 10);
    if (isize <= 0 || isize > MAX_HEIGHT) {
        fclose(fSource);
        fclose(fSink);
        puts("Input size is not valid\n");
        return 1;
    }
    mfxI32 inputHeight = isize;

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

    // Initialize VPP parameters
    // - For simplistic memory management, system memory surfaces are used to store the raw frames
    //   (Note that when using HW acceleration video surfaces are prefered, for better performance)
    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));
    // Input data
    mfxVPPParams.vpp.In.FourCC        = MFX_FOURCC_I420;
    mfxVPPParams.vpp.In.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxVPPParams.vpp.In.CropX         = 0;
    mfxVPPParams.vpp.In.CropY         = 0;
    mfxVPPParams.vpp.In.CropW         = inputWidth;
    mfxVPPParams.vpp.In.CropH         = inputHeight;
    mfxVPPParams.vpp.In.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxVPPParams.vpp.In.FrameRateExtN = 30;
    mfxVPPParams.vpp.In.FrameRateExtD = 1;
    mfxVPPParams.vpp.In.Width         = mfxVPPParams.vpp.In.CropW;
    mfxVPPParams.vpp.In.Height        = mfxVPPParams.vpp.In.CropH;
    // Output data
    mfxVPPParams.vpp.Out.FourCC        = MFX_FOURCC_I420;
    mfxVPPParams.vpp.Out.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxVPPParams.vpp.Out.CropX         = 0;
    mfxVPPParams.vpp.Out.CropY         = 0;
    mfxVPPParams.vpp.Out.CropW         = OUTPUT_WIDTH;
    mfxVPPParams.vpp.Out.CropH         = OUTPUT_HEIGHT;
    mfxVPPParams.vpp.Out.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxVPPParams.vpp.Out.FrameRateExtN = 30;
    mfxVPPParams.vpp.Out.FrameRateExtD = 1;
    mfxVPPParams.vpp.Out.Width         = mfxVPPParams.vpp.Out.CropW;
    mfxVPPParams.vpp.Out.Height        = mfxVPPParams.vpp.Out.CropH;

    mfxVPPParams.IOPattern =
        MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    // Query number of required surfaces for VPP
    mfxFrameAllocRequest VPPRequest[2]; // [0] - in, [1] - out
    memset(&VPPRequest, 0, sizeof(mfxFrameAllocRequest) * 2);
    sts = MFXVideoVPP_QueryIOSurf(session, &mfxVPPParams, VPPRequest);
    if (sts != MFX_ERR_NONE) {
        fclose(fSource);
        fclose(fSink);
        puts("QueryIOSurf error");
        return 1;
    }

    mfxU16 nVPPSurfNumIn  = VPPRequest[0].NumFrameSuggested;
    mfxU16 nVPPSurfNumOut = VPPRequest[1].NumFrameSuggested;

    // Allocate surfaces for VPP: In
    // - Frame surface array keeps pointers all surface planes and general frame info
    fourCC = mfxVPPParams.vpp.In.FourCC;
    width  = mfxVPPParams.vpp.In.Width;
    height = mfxVPPParams.vpp.In.Height;

    mfxU32 surfaceSize = GetSurfaceSize(fourCC, width, height);
    if (surfaceSize == 0) {
        fclose(fSource);
        fclose(fSink);
        puts("VPP-in surface size is wrong");
        return 1;
    }

    std::vector<mfxU8> surfDataIn(surfaceSize * nVPPSurfNumIn);
    mfxU8 *surfaceBuffersIn = surfDataIn.data();

    std::vector<mfxFrameSurface1> pVPPSurfacesIn(nVPPSurfNumIn);
    for (mfxI32 i = 0; i < nVPPSurfNumIn; i++) {
        memset(&pVPPSurfacesIn[i], 0, sizeof(mfxFrameSurface1));
        pVPPSurfacesIn[i].Info   = mfxVPPParams.vpp.In;
        pVPPSurfacesIn[i].Data.Y = &surfaceBuffersIn[surfaceSize * i];
        pVPPSurfacesIn[i].Data.U = pVPPSurfacesIn[i].Data.Y + width * height;
        pVPPSurfacesIn[i].Data.V =
            pVPPSurfacesIn[i].Data.U + ((width / 2) * (height / 2));
        pVPPSurfacesIn[i].Data.Pitch = width;
    }

    // Allocate surfaces for VPP: Out
    // - Frame surface array keeps pointers all surface planes and general frame info
    fourCC = mfxVPPParams.vpp.Out.FourCC;
    width  = mfxVPPParams.vpp.Out.Width;
    height = mfxVPPParams.vpp.Out.Height;

    surfaceSize = GetSurfaceSize(fourCC, width, height);
    if (surfaceSize == 0) {
        fclose(fSource);
        fclose(fSink);
        puts("VPP-out surface size is wrong");
        return 1;
    }

    std::vector<mfxU8> surfDataOut(surfaceSize * nVPPSurfNumOut);
    mfxU8 *surfaceBuffersOut = surfDataOut.data();

    std::vector<mfxFrameSurface1> pVPPSurfacesOut(nVPPSurfNumOut);
    for (mfxI32 i = 0; i < nVPPSurfNumOut; i++) {
        memset(&pVPPSurfacesOut[i], 0, sizeof(mfxFrameSurface1));
        pVPPSurfacesOut[i].Info   = mfxVPPParams.vpp.Out;
        pVPPSurfacesOut[i].Data.Y = &surfaceBuffersOut[surfaceSize * i];
        pVPPSurfacesOut[i].Data.U = pVPPSurfacesOut[i].Data.Y + width * height;
        pVPPSurfacesOut[i].Data.V =
            pVPPSurfacesOut[i].Data.U + ((width / 2) * (height / 2));
        pVPPSurfacesOut[i].Data.Pitch = width;
    }

    // Initialize Media SDK VPP
    sts = MFXVideoVPP_Init(session, &mfxVPPParams);
    if (sts != MFX_ERR_NONE) {
        fclose(fSource);
        fclose(fSink);
        puts("Could not initialize vpp");
        return 1;
    }

    // Prepare Media SDK bit stream buffer
    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 2000000;
    std::vector<mfxU8> bstData(mfxBS.MaxLength);
    mfxBS.Data = bstData.data();

    // Start processing the frames
    int nSurfIdxIn = 0, nSurfIdxOut = 0;
    mfxSyncPoint syncp;
    mfxU32 framenum = 0;

    printf("Processing %s -> %s\n", in_filename, OUTPUT_FILE);

    // Stage 1: Main processing loop
    while (MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts) {
        nSurfIdxIn =
            GetFreeSurfaceIndex(pVPPSurfacesIn); // Find free frame surface
        if (nSurfIdxIn == MFX_ERR_NOT_FOUND) {
            fclose(fSource);
            fclose(fSink);
            puts("no available surface");
            return 1;
        }

        sts = LoadRawFrame(&pVPPSurfacesIn[nSurfIdxIn], fSource);
        if (sts != MFX_ERR_NONE)
            break;

        nSurfIdxOut = GetFreeSurfaceIndex(
            pVPPSurfacesOut); // Find free output frame surface
        if (nSurfIdxOut == MFX_ERR_NOT_FOUND) {
            fclose(fSource);
            fclose(fSink);
            puts("no available surface");
            return 1;
        }

        for (;;) {
            // Process a frame asychronously (returns immediately)
            sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                               &pVPPSurfacesIn[nSurfIdxIn],
                                               &pVPPSurfacesOut[nSurfIdxOut],
                                               NULL,
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
                60000); // Synchronize. Wait until a frame is ready
            ++framenum;
            WriteRawFrame(&pVPPSurfacesOut[nSurfIdxOut], fSink);
            mfxBS.DataLength = 0;
        }
    }

    sts = MFX_ERR_NONE;

    // Stage 2: Retrieve the buffered processed frames
    while (MFX_ERR_NONE <= sts) {
        nSurfIdxOut =
            GetFreeSurfaceIndex(pVPPSurfacesOut); // Find free frame surface
        if (nSurfIdxOut == MFX_ERR_NOT_FOUND) {
            fclose(fSource);
            fclose(fSink);
            puts("no available surface");
            return 1;
        }

        for (;;) {
            // Process a frame asychronously (returns immediately)
            sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                               NULL,
                                               &pVPPSurfacesOut[nSurfIdxOut],
                                               NULL,
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
                60000); // Synchronize. Wait until a frame is ready
            ++framenum;
            WriteRawFrame(&pVPPSurfacesOut[nSurfIdxOut], fSink);
            mfxBS.DataLength = 0;
        }
    }

    printf("Processed %d frames\n", framenum);

    // Clean up resources - It is recommended to close Media SDK components
    // first, before releasing allocated surfaces, since some surfaces may still
    // be locked by internal Media SDK resources.
    MFXVideoVPP_Close(session);

    fclose(fSource);
    fclose(fSink);

    return 0;
}

mfxStatus LoadRawFrame(mfxFrameSurface1 *pSurface, FILE *fSource) {
    mfxU16 w, h, i, pitch;
    mfxU32 nBytesRead;
    mfxU8 *ptr;
    mfxFrameInfo *pInfo = &pSurface->Info;
    mfxFrameData *pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    switch (pInfo->FourCC) {
        case MFX_FOURCC_I420:
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

// Write raw I420 frame to file
void WriteRawFrame(mfxFrameSurface1 *pSurface, FILE *f) {
    mfxU16 w, h, i, pitch;
    mfxFrameInfo *pInfo = &pSurface->Info;
    mfxFrameData *pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    // write the output to disk
    switch (pInfo->FourCC) {
        case MFX_FOURCC_I420:
            // Y
            pitch = pData->Pitch;
            for (i = 0; i < h; i++) {
                fwrite(pData->Y + i * pitch, 1, w, f);
            }
            // U
            pitch /= 2;
            h /= 2;
            w /= 2;
            for (i = 0; i < h; i++) {
                fwrite(pData->U + i * pitch, 1, w, f);
            }
            // V
            for (i = 0; i < h; i++) {
                fwrite(pData->V + i * pitch, 1, w, f);
            }
            break;
        default:
            break;
    }

    return;
}

// Return the surface size in bytes given format and dimensions
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
    mfxU32 nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_I420:
            nbytes = width * height + (width >> 1) * (height >> 1) +
                     (width >> 1) * (height >> 1);
            break;
        default:
            break;
    }

    return nbytes;
}

// Return index of free surface in given pool
mfxI32 GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1> &pSurfacesPool) {
    auto it = std::find_if(pSurfacesPool.begin(),
                           pSurfacesPool.end(),
                           [](const mfxFrameSurface1 &surface) {
                               return 0 == surface.Data.Locked;
                           });

    if (it == pSurfacesPool.end())
        return MFX_ERR_NOT_FOUND;
    else
        return static_cast<mfxI32>(it - pSurfacesPool.begin());
}

char *ValidateFileName(char *in) {
    if (in) {
        if (strlen(in) > MAX_PATH)
            return NULL;
    }

    return in;
}

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
