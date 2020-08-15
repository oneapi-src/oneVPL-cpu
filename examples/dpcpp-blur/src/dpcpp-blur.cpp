//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// Minimal oneAPI Video Processing Library (oneVPL) dpc++ interop application.
///
/// @file

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <vector>

#include "vpl/mfxvideo.h"

#ifdef BUILD_DPCPP
    #include "CL/sycl.hpp"
#endif

#define MAX_PATH   260
#define MAX_WIDTH  3840
#define MAX_HEIGHT 2160

#define OUTPUT_FILE "out.rgba"

#define BLUR_RADIUS 5
#define BLUR_SIZE   (float)((BLUR_RADIUS << 1) + 1)

#ifdef BUILD_DPCPP
namespace dpc_common {
// this exception handler with catch async exceptions
static auto exception_handler = [](cl::sycl::exception_list eList) {
    for (std::exception_ptr const &e : eList) {
        try {
            std::rethrow_exception(e);
        }
        catch (std::exception const &e) {
    #if _DEBUG
            std::cout << "Failure" << std::endl;
    #endif
            std::terminate();
        }
    }
};
}; // namespace dpc_common

// Select device on which to run kernel.
class MyDeviceSelector : public cl::sycl::device_selector {
public:
    MyDeviceSelector() {}

    int operator()(const cl::sycl::device &device) const override {
        const std::string name =
            device.get_info<cl::sycl::info::device::name>();

        std::cout << "Trying device: " << name << "..." << std::endl;
        std::cout << "  Vendor: "
                  << device.get_info<cl::sycl::info::device::vendor>()
                  << std::endl;

        if (device.is_cpu())
            return 500; // We give higher merit for CPU
        //if (device.is_accelerator()) return 400;
        //if (device.is_gpu()) return 300;
        //if (device.is_host()) return 100;
        return -1;
    }
};

void BlurFrame(sycl::queue q,
               mfxFrameSurface1 *inSurface,
               mfxFrameSurface1 *bluredSurface);
#endif

mfxStatus LoadRawFrame(mfxFrameSurface1 *pSurface, FILE *fSource);
void WriteRawFrame(mfxFrameSurface1 *pSurface, FILE *f);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
mfxI32 GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1> &pSurfacesPool);
char *ValidateFileName(char *in);

// Print usage message
void Usage(void) {
    printf("Usage: dpcpp-blur SOURCE WIDTH HEIGHT\n\n"
           "Process raw I420 video in SOURCE having dimensions WIDTH x HEIGHT "
           "to blurred raw RGB32 video in %s\n"
           "Default blur kernel is [%d]x[%d]\n\n"
           "To view:\n"
           " ffplay -video_size [width]x[height] "
           "-pixel_format rgb32 -f rawvideo %s\n",
           OUTPUT_FILE,
           2 * BLUR_RADIUS + 1,
           2 * BLUR_RADIUS + 1,
           OUTPUT_FILE);
}

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
    // Output data in the same size but with RGB32 color format
    mfxVPPParams.vpp.Out.FourCC        = MFX_FOURCC_RGB4;
    mfxVPPParams.vpp.Out.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxVPPParams.vpp.Out.CropX         = 0;
    mfxVPPParams.vpp.Out.CropY         = 0;
    mfxVPPParams.vpp.Out.CropW         = inputWidth;
    mfxVPPParams.vpp.Out.CropH         = inputHeight;
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

    // Frame surface array keeps pointers to all surface planes and general
    // frame info
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

    // Frame surface array keeps pointers to all surface planes and general
    // frame info
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

    // we need one more surface for the blured image
    std::vector<mfxU8> surfDataOut(surfaceSize * (nVPPSurfNumOut + 1));
    mfxU8 *surfaceBuffersOut = surfDataOut.data();

    std::vector<mfxFrameSurface1> pVPPSurfacesOut(nVPPSurfNumOut);
    for (mfxI32 i = 0; i < nVPPSurfNumOut; i++) {
        memset(&pVPPSurfacesOut[i], 0, sizeof(mfxFrameSurface1));
        pVPPSurfacesOut[i].Info       = mfxVPPParams.vpp.Out;
        pVPPSurfacesOut[i].Data.B     = &surfaceBuffersOut[surfaceSize * i];
        pVPPSurfacesOut[i].Data.G     = pVPPSurfacesOut[i].Data.B + 1;
        pVPPSurfacesOut[i].Data.R     = pVPPSurfacesOut[i].Data.G + 1;
        pVPPSurfacesOut[i].Data.A     = pVPPSurfacesOut[i].Data.R + 1;
        pVPPSurfacesOut[i].Data.Pitch = width * 4;
    }

    // Initialize surface for blured frame
    mfxFrameSurface1 bluredSurface;
    std::vector<mfxU8> blurDataOut(surfaceSize);

    memset(&bluredSurface, 1, sizeof(bluredSurface));
    bluredSurface.Info       = mfxVPPParams.vpp.Out;
    bluredSurface.Data.B     = &blurDataOut[0];
    bluredSurface.Data.G     = bluredSurface.Data.B + 1;
    bluredSurface.Data.R     = bluredSurface.Data.G + 1;
    bluredSurface.Data.A     = bluredSurface.Data.R + 1;
    bluredSurface.Data.Pitch = width * 4;

    // Initialize VPP
    sts = MFXVideoVPP_Init(session, &mfxVPPParams);
    if (sts != MFX_ERR_NONE) {
        fclose(fSource);
        fclose(fSink);
        puts("Could not initialize vpp");
        return 1;
    }

    // Prepare bit stream buffer
    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 2000000;
    std::vector<mfxU8> bstData(mfxBS.MaxLength);
    mfxBS.Data = bstData.data();

    // Start processing the frames
    int nSurfIdxIn = 0, nSurfIdxOut = 0;
    mfxSyncPoint syncp;
    mfxU32 framenum = 0;

#ifdef BUILD_DPCPP
    // Initialize DPC++
    MyDeviceSelector sel;

    // Create SYCL execution queue
    sycl::queue q(sel, dpc_common::exception_handler);

    // See what device was actually selected for this queue.
    // CPU is preferrable for this time.
    std::cout << "Running on "
              << q.get_device().get_info<sycl::info::device::name>() << "\n";
#endif

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
            // Blur and store processed frame
#ifdef BUILD_DPCPP
            BlurFrame(q, &pVPPSurfacesOut[nSurfIdxOut], &bluredSurface);
            WriteRawFrame(&bluredSurface, fSink);
#else
            WriteRawFrame(&pVPPSurfacesOut[nSurfIdxOut], fSink);
#endif
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
            // Blur and store processed frame
#ifdef BUILD_DPCPP
            BlurFrame(q, &pVPPSurfacesOut[nSurfIdxOut], &bluredSurface);
            WriteRawFrame(&bluredSurface, fSink);
#else
            WriteRawFrame(&pVPPSurfacesOut[nSurfIdxOut], fSink);
#endif

            mfxBS.DataLength = 0;
        }
    }

    printf("Processed %d frames\n", framenum);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.
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

// Write raw rgb32 frame to file
void WriteRawFrame(mfxFrameSurface1 *pSurface, FILE *f) {
    mfxU16 w, h, i, pitch;
    mfxFrameInfo *pInfo = &pSurface->Info;
    mfxFrameData *pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    // write the output to disk
    switch (pInfo->FourCC) {
        case MFX_FOURCC_RGB4:
            pitch = pData->Pitch;
            for (i = 0; i < h; i++) {
                fwrite(pData->B + i * pitch, 1, w * 4, f);
            }
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
        case MFX_FOURCC_RGB4:
            nbytes = 4 * width * height;
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

#ifdef BUILD_DPCPP

// Few useful acronyms.
constexpr auto sycl_read  = sycl::access::mode::read;
constexpr auto sycl_write = sycl::access::mode::write;

// SYCL kernel scheduler
// Blur frame by using SYCL kernel
void BlurFrame(sycl::queue q,
               mfxFrameSurface1 *inSurface,
               mfxFrameSurface1 *bluredSurface) {
    int img_width, img_height;

    img_width  = inSurface->Info.Width;
    img_height = inSurface->Info.Height;

    // Wrap mfx surfaces into SYCL image by using host ptr for zero copy of data
    sycl::image<2> image_buf_src(inSurface->Data.B,
                                 sycl::image_channel_order::rgba,
                                 sycl::image_channel_type::unsigned_int8,
                                 sycl::range<2>(img_width, img_height));

    sycl::image<2> image_buf_dst(bluredSurface->Data.B,
                                 sycl::image_channel_order::rgba,
                                 sycl::image_channel_type::unsigned_int8,
                                 sycl::range<2>(img_width, img_height));

    try {
        q.submit([&](cl::sycl::handler &cgh) {
            // Src image accessor
            sycl::accessor<cl::sycl::uint4,
                           2,
                           sycl_read,
                           sycl::access::target::image>
                accessorSrc(image_buf_src, cgh);
            // Dst image accessor
            auto accessorDst =
                image_buf_dst.get_access<cl::sycl::uint4, sycl_write>(cgh);
            cl::sycl::uint4 black = (cl::sycl::uint4)(0);
            // Parallel execution of the kerner for each pixel. Kernel
            // implemented as a lambda function.

            // Important: this is naive implementation of the blur kernel. For
            // further optimization it is better to use range_nd iterator and
            // apply moving average technique to reduce # of MAC operations per
            // pixel.
            cgh.parallel_for<class NaiveBlur_rgba>(
                sycl::range<2>(img_width, img_height),
                [=](sycl::item<2> item) {
                    auto coords = cl::sycl::int2(item[0], item[1]);

                    // Let's add horizontal black border
                    if (item[0] <= BLUR_RADIUS ||
                        item[0] >= img_width - 1 - BLUR_RADIUS) {
                        accessorDst.write(coords, black);
                        return;
                    }

                    // Let's add vertical black border
                    if (item[1] <= BLUR_RADIUS ||
                        item[1] >= img_height - 1 - BLUR_RADIUS) {
                        accessorDst.write(coords, black);
                        return;
                    }

                    cl::sycl::float4 tmp = (cl::sycl::float4)(0.f);
                    cl::sycl::uint4 rgba;

                    for (int i = item[0] - BLUR_RADIUS;
                         i < item[0] + BLUR_RADIUS;
                         i++) {
                        for (int j = item[1] - BLUR_RADIUS;
                             j < item[1] + BLUR_RADIUS;
                             j++) {
                            rgba = accessorSrc.read(cl::sycl::int2(i, j));
                            // Sum over the square mask
                            tmp[0] += rgba.x();
                            tmp[1] += rgba.y();
                            tmp[2] += rgba.z();
                            // Keep alpha channel from anchor pixel
                            if (i == item[0] && j == item[1])
                                tmp[3] = rgba.w();
                        }
                    }
                    // Compute average intensity
                    tmp[0] /= BLUR_SIZE * BLUR_SIZE;
                    tmp[1] /= BLUR_SIZE * BLUR_SIZE;
                    tmp[2] /= BLUR_SIZE * BLUR_SIZE;

                    // Convert and write blur pixel
                    cl::sycl::uint4 tmp_u;
                    tmp_u[0] = tmp[0];
                    tmp_u[1] = tmp[1];
                    tmp_u[2] = tmp[2];
                    tmp_u[3] = tmp[3];

                    accessorDst.write(coords, tmp_u);
                });
        });

        // Since we are in blocking execution mode for this sample simplicity,
        // we need to wait for the execution completeness.
        q.wait_and_throw();
    }
    catch (std::exception e) {
        std::cout << "SYCL exception caught: " << e.what() << "\n";
        return;
    }
    return;
}
#endif // BUILD_DPCPP
