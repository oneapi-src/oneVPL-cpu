#include "video_decoder.h"
#include "frame_alloc.h"

VideoDecoder::VideoDecoder()
        : m_session(),
          m_impl(0),
          m_version(),
          m_bUseExtAlloc(0),

          m_mfxVideoDecode(0),
          m_mfxVideoParams(),
          m_mfxBitstream(),
          m_codecId(0),
          m_inbuf(0),
          m_bDrainingBuffer(0),

          m_mfxFrameAllocator(),
          m_mfxFrameAllocRequest(),
          m_mfxFrameAllocResponse(),
#ifdef LIBVA_SUPPORT
          m_vaDisp(0),
          m_drmFD(0),
#endif

          m_surfBuf(0),
          m_mfxSurfacePool(0),
          m_mfxOutSurface(0),
          m_nSurfaces(0),
          m_infile(0) {
}

VideoDecoder::~VideoDecoder() {}

// platform-specific display initialization (only needed for Linux currently)
mfxStatus VideoDecoder::InitDisplay(void) {
#ifdef LIBVA_SUPPORT
    mfxStatus sts = MFX_ERR_NONE;
    VAStatus res;
    int verMajor, verMinor;

    // open default card
    m_drmFD = open("/dev/dri/card0", O_RDWR);

    if (m_drmFD < 0) {
        return MFX_ERR_NOT_INITIALIZED;
    }

    // get handle to display from DRM connection
    m_vaDisp = vaGetDisplayDRM(m_drmFD);

    if (!m_vaDisp) {
        close(m_drmFD);
        return MFX_ERR_NULL_PTR;
    }

    // init libva
    res = vaInitialize(m_vaDisp, &verMajor, &verMinor);

    if (res != VA_STATUS_SUCCESS) {
        close(m_drmFD);
        m_drmFD = -1;
        return MFX_ERR_NOT_INITIALIZED;
    }

    // pass handle to MSDK session
    sts = m_session.SetHandle(static_cast<mfxHandleType>(MFX_HANDLE_VA_DISPLAY),
                              m_vaDisp);

    return sts;
#else
    return MFX_ERR_NONE;
#endif
}

// platform-specific display teardown
mfxStatus VideoDecoder::CloseDisplay(void) {
#ifdef LIBVA_SUPPORT
    if (m_vaDisp) {
        vaTerminate(m_vaDisp);
        m_vaDisp = 0;
    }

    if (m_drmFD >= 0) {
        close(m_drmFD);
        m_drmFD = -1;
    }

    return MFX_ERR_NONE;
#else
    return MFX_ERR_NONE;
#endif
}

// initialize new MSDK session
mfxStatus VideoDecoder::InitSession(mfxU32 bUseExtAlloc) {
    mfxStatus sts = MFX_ERR_NONE;

    // require HW library, API >= 1.0
    // query later for actual loaded API version
    m_impl          = MFX_IMPL_SOFTWARE_VPL;
    m_version.Major = 1;
    m_version.Minor = 0;
    m_bUseExtAlloc  = bUseExtAlloc;

    printf(
        "MSDK Init - requested API version ... %d.%02d, requested implementation ... 0x%04x\n",
        m_version.Major,
        m_version.Minor,
        m_impl);

    // initialize new MSDK session
    sts = m_session.Init(m_impl, &m_version);
    if (sts) {
        ERR_AND_EXIT("session Init() failed");
    }

    // initialize display if needed (i.e. Linux)
    sts = InitDisplay();
    if (sts) {
        ERR_AND_EXIT("InitDisplay() failed");
    }

    // set callbacks for external frame allocator if required
    if (m_bUseExtAlloc) {
        m_mfxFrameAllocator.pthis  = m_session;
        m_mfxFrameAllocator.Alloc  = sys_alloc;
        m_mfxFrameAllocator.Free   = sys_free;
        m_mfxFrameAllocator.Lock   = sys_lock;
        m_mfxFrameAllocator.Unlock = sys_unlock;
        m_mfxFrameAllocator.GetHDL = sys_gethdl;

        sts = m_session.SetFrameAllocator(&m_mfxFrameAllocator);
        if (sts) {
            ERR_AND_EXIT("SetFrameAllocator() failed");
        }
    }

    // query actual implementation
    sts = m_session.QueryIMPL(&m_impl);
    if (sts) {
        ERR_AND_EXIT("QueryIMPL() failed");
    }

    // query actual API version
    sts = m_session.QueryVersion(&m_version);
    if (sts) {
        ERR_AND_EXIT("QueryVersion() failed");
    }

    printf(
        "MSDK Init - actual API version ...... %d.%02d, actual implementation ...... 0x%04x\n",
        m_version.Major,
        m_version.Minor,
        m_impl);

    return sts;
}

// close MSDK session
mfxStatus VideoDecoder::CloseSession(void) {
    CloseDisplay();

    m_session.Close();

    return MFX_ERR_NONE;
}

// fill bitstream buffer
// returns number of bytes available in buffer after reading
// 0 indicates EOF
mfxU32 VideoDecoder::FillBitstreamBuffer(FILE *infile, mfxBitstream *bs) {
    mfxU32 toRead, nRead;

    // shift unused data to start of buffer
    memmove(bs->Data, bs->Data + bs->DataOffset, bs->DataLength);
    bs->DataOffset = 0;

    toRead = (bs->MaxLength - bs->DataLength);
    nRead  = (mfxU32)fread(bs->Data + bs->DataLength, 1, toRead, infile);
    bs->DataLength += nRead;

    return bs->DataLength;
}

// allocate surfaces for decoded pictures
mfxStatus VideoDecoder::AllocateSurfaces(void) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxU32 surfBytes, surfIdx;
    mfxU16 width, height;

    // query for required number of decoded surfaces to allocate
    memset(&m_mfxFrameAllocRequest, 0, sizeof(m_mfxFrameAllocRequest));
    sts = m_mfxVideoDecode->QueryIOSurf(&m_mfxVideoParams,
                                        &m_mfxFrameAllocRequest);
    if (sts) {
        ERR_AND_EXIT("QueryIOSurf() failed");
    }

    // allocate pool of NV12 surfaces (size = W*H*1.5 bytes)
    m_nSurfaces      = m_mfxFrameAllocRequest.NumFrameSuggested;
    m_mfxSurfacePool = new mfxFrameSurface1[m_nSurfaces];
    if (!m_mfxSurfacePool) {
        ERR_AND_EXIT("Allocating surface pool failed");
    }

    if (m_bUseExtAlloc) {
        // set up external allocator if requested
        sts = m_mfxFrameAllocator.Alloc(m_mfxFrameAllocator.pthis,
                                        &m_mfxFrameAllocRequest,
                                        &m_mfxFrameAllocResponse);
        if (sts) {
            ERR_AND_EXIT("Video frame Alloc() failed");
        }

        // initialize surface pool
        for (surfIdx = 0; surfIdx < m_nSurfaces; surfIdx++) {
            memset(&(m_mfxSurfacePool[surfIdx]), 0, sizeof(mfxFrameSurface1));
            memcpy(&(m_mfxSurfacePool[surfIdx].Info),
                   &(m_mfxVideoParams.mfx.FrameInfo),
                   sizeof(mfxFrameInfo));
            m_mfxSurfacePool[surfIdx].Data.MemId =
                &(m_mfxFrameAllocResponse.mids[surfIdx]);
        }
    }
    else {
        // use internal MSDK allocator
        width     = (mfxU16)ALIGN_N(m_mfxFrameAllocRequest.Info.Width, 32);
        height    = (mfxU16)ALIGN_N(m_mfxFrameAllocRequest.Info.Height, 32);
        surfBytes = width * height * 3 / 2;

        // allocate contiguous chunk of memory large enough for all surfaces
        m_surfBuf = new mfxU8[surfBytes * m_nSurfaces];
        if (!m_surfBuf) {
            ERR_AND_EXIT("Allocating surface buffer failed");
        }

        // initialize surface pool
        for (surfIdx = 0; surfIdx < m_nSurfaces; surfIdx++) {
            memset(&(m_mfxSurfacePool[surfIdx]), 0, sizeof(mfxFrameSurface1));
            memcpy(&(m_mfxSurfacePool[surfIdx].Info),
                   &(m_mfxVideoParams.mfx.FrameInfo),
                   sizeof(mfxFrameInfo));

            // NV12 = Y plane (w*h) followed by interleaved U/V plane (w*h/2)
            m_mfxSurfacePool[surfIdx].Data.Y =
                m_surfBuf + (surfIdx * surfBytes);
            m_mfxSurfacePool[surfIdx].Data.U =
                m_mfxSurfacePool[surfIdx].Data.Y + width * height;
            m_mfxSurfacePool[surfIdx].Data.V =
                m_mfxSurfacePool[surfIdx].Data.U + width * height / 4;
            m_mfxSurfacePool[surfIdx].Data.Pitch = width;
        }
    }

    return sts;
}

// free surfaces
mfxStatus VideoDecoder::FreeSurfaces(void) {
    if (m_bUseExtAlloc) {
        m_mfxFrameAllocator.Free(m_mfxFrameAllocator.pthis,
                                 &m_mfxFrameAllocResponse);
    }
    else {
        delete[] m_surfBuf;
    }
    delete[] m_mfxSurfacePool;

    return MFX_ERR_NONE;
}

// initialize MSDK decoder
mfxStatus VideoDecoder::InitDecoder(mfxU32 codecId, FILE *infile) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxU32 bytesAvailable;

    m_codecId         = codecId;
    m_infile          = infile;
    m_bDrainingBuffer = 0;

    // create new decoder instance
    m_mfxVideoDecode = new MFXVideoDECODE(m_session);

    // system memory for now
    memset(&m_mfxVideoParams, 0, sizeof(m_mfxVideoParams));
    m_mfxVideoParams.mfx.CodecId = codecId;
    m_mfxVideoParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    // allocate input buffer
    m_inbuf = new mfxU8[DEF_INBUF_BYTES];
    if (!m_inbuf) {
        ERR_AND_EXIT("Failed to allocate inbuf");
    }

    // create bitstream object
    memset(&m_mfxBitstream, 0, sizeof(mfxBitstream));
    m_mfxBitstream.Data      = m_inbuf;
    m_mfxBitstream.MaxLength = DEF_INBUF_BYTES;

    // fill bitstream buffer
    bytesAvailable = FillBitstreamBuffer(m_infile, &m_mfxBitstream);
    if (bytesAvailable == 0) {
        ERR_AND_EXIT("Empty input file");
    }

    // decode stream header
    sts = m_mfxVideoDecode->DecodeHeader(&m_mfxBitstream, &m_mfxVideoParams);
    if (sts) {
        ERR_AND_EXIT("Failed to decode stream header");
    }

    // validate decoder parameters
    sts = m_mfxVideoDecode->Query(&m_mfxVideoParams, &m_mfxVideoParams);
    if (sts) {
        ERR_AND_EXIT("Failed query");
    }

    // allocate frame surfaces for decoder
    sts = AllocateSurfaces();
    if (sts) {
        ERR_AND_EXIT("Failed to allocate surfaces");
    }

    // initialize decoder
    sts = m_mfxVideoDecode->Init(&m_mfxVideoParams);
    if (sts) {
        ERR_AND_EXIT("Failed to init decoder");
    }

    return sts;
}

// close MSDK decoder
mfxStatus VideoDecoder::CloseDecoder(void) {
    FreeSurfaces();

    delete[] m_inbuf;
    delete m_mfxVideoDecode; // destructor calls MFXVideoDECODE_Close()

    return MFX_ERR_NONE;
}

// decode a single frame
// return number of unused bytes remaining in internal bitstream buffer
mfxStatus VideoDecoder::DecodeOneFrame(mfxU32 *nBytesBuffered) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxSyncPoint syncp;
    mfxU32 surfIdx, bytesAvailable, nFramesDecoded = 0;

    // loop until a single frame is decoded (or EOF reached)
    while (nFramesDecoded == 0) {
        // find first unlocked surface
        for (surfIdx = 0; surfIdx < m_nSurfaces; surfIdx++) {
            if (m_mfxSurfacePool[surfIdx].Data.Locked == 0)
                break;
        }

        if (surfIdx == m_nSurfaces) {
            ERR_AND_EXIT("No free surfaces in pool");
        }

        // decode new frame
        m_mfxOutSurface = NULL;
        sts             = m_mfxVideoDecode->DecodeFrameAsync(
            m_bDrainingBuffer ? NULL : &m_mfxBitstream,
            &m_mfxSurfacePool[surfIdx],
            &m_mfxOutSurface,
            &syncp);

        if (sts > MFX_ERR_NONE) {
            // warning = frame may or may not have been produced.
            // if syncp != NULL, we have a valid frame, otherwise will just call DecodeFrameAync again
            sts = MFX_ERR_NONE;
        }
        else if (sts == MFX_ERR_MORE_SURFACE) {
            // DecodeFrameAsync needs more surfaces - call again
            continue;
        }
        else if (sts == MFX_ERR_MORE_DATA) {
            if (m_bDrainingBuffer) {
                // all done - the decoded frame buffer has been completely drained
                return MFX_ERR_MORE_DATA;
            }
            else {
                // refill bitstream buffer and call again - if EOF, start draining decoded frames
                bytesAvailable = FillBitstreamBuffer(m_infile, &m_mfxBitstream);
                if (bytesAvailable == 0)
                    m_bDrainingBuffer = true;
                continue;
            }
        }
        else if (sts < MFX_ERR_NONE) {
            printf("Fatal error in DecodeOneFrame: 0x%x\n", sts);
            return sts;
        }

        // sync - wait for frame decoding to finish
        if (syncp && sts == MFX_ERR_NONE) {
            do {
                sts = m_session.SyncOperation(syncp, 10000);
            } while (sts == MFX_WRN_IN_EXECUTION);

            // sync successful - decode frame is returned in m_mfxOutSurface
            if (sts == MFX_ERR_NONE) {
                nFramesDecoded++;
            }
        }
    }

    // return number of unused bytes remaining in bitstream buffer
    *nBytesBuffered = m_mfxBitstream.DataLength;

    return MFX_ERR_NONE;
}

// write the previously-decoded frame to an output file
mfxStatus VideoDecoder::WriteFrameToFile(FILE *outfile) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxU32 w, h, y, pitch, offset;

    if (!outfile) {
        return MFX_ERR_NULL_PTR;
    }

    // lock frame if external allocator is used
    if (m_bUseExtAlloc) {
        sts = m_mfxFrameAllocator.Lock(m_mfxFrameAllocator.pthis,
                                       m_mfxOutSurface->Data.MemId,
                                       &(m_mfxOutSurface->Data));
        if (sts) {
            ERR_AND_EXIT("Failed to lock external surface");
        }
    }

    pitch = m_mfxOutSurface->Data.Pitch;
    w     = m_mfxOutSurface->Info.CropW;
    h     = m_mfxOutSurface->Info.CropH;

    // write Y plane
    for (y = 0; y < h; y++) {
        offset = pitch * (m_mfxOutSurface->Info.CropY + y) +
                 m_mfxOutSurface->Info.CropX;
        fwrite(m_mfxOutSurface->Data.Y + offset, 1, w, outfile);
    }

    // write U plane
    pitch = m_mfxOutSurface->Data.Pitch;
    for (y = 0; y < h / 2; y++) {
        offset = pitch / 2 * (m_mfxOutSurface->Info.CropY + y) +
                 m_mfxOutSurface->Info.CropX;
        fwrite(m_mfxOutSurface->Data.U + offset, 1, w / 2, outfile);
    }

    // write V plane
    pitch = m_mfxOutSurface->Data.Pitch;
    for (y = 0; y < h / 2; y++) {
        offset = pitch / 2 * (m_mfxOutSurface->Info.CropY + y) +
                 m_mfxOutSurface->Info.CropX;
        fwrite(m_mfxOutSurface->Data.V + offset, 1, w / 2, outfile);
    }

    // unlock frame if external allocator is used
    if (m_bUseExtAlloc) {
        sts = m_mfxFrameAllocator.Unlock(m_mfxFrameAllocator.pthis,
                                         m_mfxOutSurface->Data.MemId,
                                         &(m_mfxOutSurface->Data));
        if (sts) {
            ERR_AND_EXIT("Failed to lock external surface");
        }
    }

    return MFX_ERR_NONE;
}

// get basic info about the decoded stream
mfxStatus VideoDecoder::GetDecoderInfo(DecoderInfo *decoderInfo) {
    mfxStatus sts = MFX_ERR_NONE;

    // must call only after initializing decoder
    if (!m_mfxVideoDecode) {
        return MFX_ERR_NOT_INITIALIZED;
    }

    if (!decoderInfo) {
        return MFX_ERR_NULL_PTR;
    }

    decoderInfo->width  = m_mfxVideoParams.mfx.FrameInfo.Width;
    decoderInfo->height = m_mfxVideoParams.mfx.FrameInfo.Height;

    return sts;
}
