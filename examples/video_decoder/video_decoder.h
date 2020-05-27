#ifndef _VIDEO_DECODER_H
#define _VIDEO_DECODER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// enable memory leak check (Windows only)
#if defined _DEBUG && (defined _WIN32 || defined _WIN64)
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif

#include "mfxvideo++.h"

#ifdef LIBVA_SUPPORT
    #include <fcntl.h>
    #include <unistd.h>
    #include "va/va.h"
    #include "va/va_drm.h"
#endif

#define ERR_AND_EXIT(x)                                               \
    {                                                                 \
        printf("Error - " x "  (%s, line %d)\n", __FILE__, __LINE__); \
        exit(-1);                                                     \
    }

#define NUM_CODECS      3
#define DEF_INBUF_BYTES (1024 * 1024)

#define ALIGN_N(X, N) (((X) + ((N)-1)) & (~((N)-1)))

typedef struct _DecoderInfo {
    mfxU32 width;
    mfxU32 height;
} DecoderInfo;

class VideoDecoder {
public:
    VideoDecoder();
    ~VideoDecoder();

    mfxStatus InitSession(mfxU32 bUseExtAlloc);
    mfxStatus CloseSession(void);
    mfxStatus InitDecoder(mfxU32 codecId, FILE *infile);
    mfxStatus CloseDecoder(void);
    mfxStatus DecodeOneFrame(mfxU32 *nBytesBuffered);
    mfxStatus WriteFrameToFile(FILE *outfile);
    mfxStatus GetDecoderInfo(DecoderInfo *decoderInfo);

private:
    mfxU32 FillBitstreamBuffer(FILE *infile, mfxBitstream *bs);
    mfxStatus InitDisplay(void);
    mfxStatus CloseDisplay(void);
    mfxStatus AllocateSurfaces(void);
    mfxStatus FreeSurfaces(void);

    // session state
    MFXVideoSession m_session;
    mfxIMPL m_impl;
    mfxVersion m_version;
    mfxU32 m_bUseExtAlloc;

    // decoder state
    MFXVideoDECODE *m_mfxVideoDecode;
    mfxVideoParam m_mfxVideoParams;
    mfxBitstream m_mfxBitstream;
    mfxU32 m_codecId;
    mfxU8 *m_inbuf;
    mfxU32 m_bDrainingBuffer;

    // external allocator state
    mfxFrameAllocator m_mfxFrameAllocator;
    mfxFrameAllocRequest m_mfxFrameAllocRequest;
    mfxFrameAllocResponse m_mfxFrameAllocResponse;

#ifdef LIBVA_SUPPORT
    // VA display state
    VADisplay m_vaDisp;
    int m_drmFD;
#endif

    // surface pool
    mfxU8 *m_surfBuf;
    mfxFrameSurface1 *m_mfxSurfacePool;
    mfxFrameSurface1 *m_mfxOutSurface;
    mfxU32 m_nSurfaces;
    FILE *m_infile;
};

#endif // _VIDEO_DECODER_H