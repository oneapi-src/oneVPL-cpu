#include "video_decoder.h"

#define PROGRESS_BAR_STEPS 30

typedef struct _CodecInfo {
    const char *codecName; // codec name (user friendly string)
    mfxU32 codecId; // codec FourCC
} CodecInfo;

static const CodecInfo codecInfoTab[NUM_CODECS] = {
    { "h264", MFX_CODEC_AVC },
    { "h265", MFX_CODEC_HEVC },
    { "mpeg2", MFX_CODEC_MPEG2 },
};

static void Usage(void) {
    mfxU32 codecIdx;

    printf("\nUsage:\n\n");
    printf(
        ">> video_decoder.exe codec -i infile.bit [-o outfile.yuv] [other options]\n\n");
    printf("valid options for codec: ");
    for (codecIdx = 0; codecIdx < NUM_CODECS; codecIdx++)
        printf("%s ", codecInfoTab[codecIdx].codecName);
    printf("\n\n");
    printf("Options:\n");
    printf("  -i  infile.bit     encoded bitstream\n");
    printf("  -o  outfile.yuv    decoded YUV stream (NV12)\n");
    printf(
        "  -e                 use external frame allocator (default = internal)\n");
    printf("\n");

    exit(-1);
}

// estimate decode progress as (bytes consumed by MSDK) / (total bytes in bitstream)
static void UpdateProgress(FILE *infile,
                           mfxU32 nBytesTotal,
                           mfxU32 nBytesBuffered,
                           mfxU32 nOutFrames) {
    mfxU32 i, nBytesRead;
    mfxF32 fracDone;

    nBytesRead = ftell(infile);

    fracDone = (mfxF32)(nBytesRead - nBytesBuffered) / nBytesTotal;
    printf("Total frames decoded = %6d -- Progress |", nOutFrames);
    for (i = 0; i < (mfxU32)(fracDone * PROGRESS_BAR_STEPS); i++)
        printf("*");
    for (; i < PROGRESS_BAR_STEPS; i++)
        printf(" ");
    printf("|\r");
    fflush(stdout);
}

int main(int argc, char **argv) {
    mfxU32 argIdx, codecIdx, nOutFrames, bUseExtAlloc = 0;
    mfxU32 nBytesBuffered, nBytesTotal, nBytesRead;
    DecoderInfo decoderInfo;
    char *infileName = NULL, *outfileName = NULL;
    FILE *infile = NULL, *outfile = NULL;

    mfxStatus sts = MFX_ERR_NONE;

#if defined _DEBUG && (defined _WIN32 || defined _WIN64)
    // enable memory leak check (Windows only)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif

    if (argc < 2) {
        Usage();
    }

    // read input codec name - must be first cmd-line argument
    // print error message and exit if it does not match list of valid codecs
    for (codecIdx = 0; codecIdx < NUM_CODECS; codecIdx++) {
        if (!strcmp(argv[1], codecInfoTab[codecIdx].codecName))
            break;
    }

    if (codecIdx == NUM_CODECS) {
        ERR_AND_EXIT("invalid codec specified");
    }

    // parse other arguments
    for (argIdx = 2; argIdx < (mfxU32)argc;) {
        if (argv[argIdx][0] == '-') {
            switch (argv[argIdx++][1]) {
                case 'i':
                    infileName = argv[argIdx++];
                    continue;
                case 'o':
                    outfileName = argv[argIdx++];
                    continue;
                case 'e':
                    bUseExtAlloc = 1;
                    continue;
                default:
                    printf("invalid cmd-line");
                    Usage();
                    break;
            }
        }
        ERR_AND_EXIT("invalid cmd-line");
        Usage();
    }

    // open input file
    if (!infileName) {
        ERR_AND_EXIT("input file not specified");
    }

    infile = fopen(infileName, "rb");
    if (!infile) {
        ERR_AND_EXIT("unable to open input file");
    }

    // open output file (optional)
    if (outfileName) {
        outfile = fopen(outfileName, "wb");
        if (!outfile) {
            ERR_AND_EXIT("unable to open output file");
        }
    }

    // get total file length (bytes) to estimate decode progress
    fseek(infile, 0, SEEK_END);
    nBytesTotal = ftell(infile);
    fseek(infile, 0, SEEK_SET);
    nBytesRead = 0;

    // create new decoder object
    VideoDecoder *videoDecoder = new VideoDecoder();

    // init session
    sts = videoDecoder->InitSession(bUseExtAlloc);
    if (sts != MFX_ERR_NONE) {
        printf("VideoDecoder::InitSession() returned sts = 0x%08x\n", sts);
        exit(-1);
    }

    // init video decoder
    sts = videoDecoder->InitDecoder(codecInfoTab[codecIdx].codecId, infile);
    if (sts != MFX_ERR_NONE) {
        printf("VideoDecoder::InitDecoder() returned sts = 0x%08x\n", sts);
        exit(-1);
    }

    // print stream info
    sts = videoDecoder->GetDecoderInfo(&decoderInfo);
    if (sts != MFX_ERR_NONE) {
        printf("VideoDecoder::GetDecoderInfo() returned sts = 0x%08x\n", sts);
        exit(-1);
    }
    printf("\n");
    printf("Decoding started:\n");
    printf("Codec ...............  %s\n", codecInfoTab[codecIdx].codecName);
    printf("Width ............... % 5d\n", decoderInfo.width);
    printf("Height .............. % 5d\n", decoderInfo.height);
    printf("Frame allocator .....  %s\n",
           (bUseExtAlloc ? "External" : "Internal"));
    printf("\n");

    // main processing loop - decode one frame at a time
    // assumes synchronous behavior (write current frame before starting to decode next frame)
    nOutFrames = 0;
    while (sts == MFX_ERR_NONE) {
        // DecodeOneFrame returns MFX_ERR_NONE if a single frame was decoded successfully or
        //    MFX_ERR_MORE_DATA if EOF was reached (other error codes indicate abnormal termination)
        sts = videoDecoder->DecodeOneFrame(&nBytesBuffered);

        if (sts == MFX_ERR_NONE) {
            if (outfile) {
                videoDecoder->WriteFrameToFile(outfile);
            }
            nOutFrames++;
            //UpdateProgress(infile, nBytesTotal, nBytesBuffered, nOutFrames);
        }
    }
    printf("\n\n");

    if (sts == MFX_ERR_MORE_DATA) {
        printf("Decoding completed successfully\n");
        printf("Total frames decoded = %d\n", nOutFrames);
    }
    else {
        printf("Warning - decoding terminated prematurely (sts == 0x%08x)\n",
               sts);
    }

    // cleanup
    videoDecoder->CloseDecoder();
    videoDecoder->CloseSession();
    delete videoDecoder;

    if (infile) {
        fclose(infile);
    }

    if (outfile) {
        fclose(outfile);
    }

    return 0;
}
