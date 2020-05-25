#ifndef _CPU_WORKSTREAM_H
#define _CPU_WORKSTREAM_H

#include "mfxstructures.h"
#include "mfxjpeg.h"

#define ENABLE_DECODE
//#define ENABLE_VPP
#define ENABLE_ENCODE

#define ENABLE_LIBAV_AUTO_THREADS

#if !defined (WIN32) && !defined(memcpy_s)
#define memcpy_s(dest, destsz, src, count) memcpy(dest, src, count)
#endif


extern "C"
{
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
}

#define ERR_EXIT(ws)  { /* optional logging, etc. here */ return MFX_ERR_UNKNOWN; }

class CpuWorkstream {
 public:
  CpuWorkstream(); 
  ~CpuWorkstream();

  // decode
  mfxStatus InitDecode(mfxU32 FourCC);
  mfxStatus DecodeFrame(mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out);
  void FreeDecode(void);
  mfxStatus DecodeGetVideoParams(mfxVideoParam *par);

  // VPP
  mfxStatus InitVPP(void);
  mfxStatus ProcessFrame(void);
  void FreeVPP(void);

  // encode
  mfxStatus InitEncode(mfxVideoParam *par);
  mfxStatus EncodeFrame(mfxFrameSurface1 *surface, mfxBitstream *bs);
  void FreeEncode(void);

  bool  m_decInit;
  bool  m_vppInit;
  bool  m_vppBypass;
  bool  m_encInit;

private:
  CpuWorkstream(const CpuWorkstream&){ /* copy not allowed */ }
  CpuWorkstream& operator=(const CpuWorkstream&){ return *this; /* copy not allowed */ }
  
  // libav objects - Decode
  const AVCodec         * m_avDecCodec;
  AVCodecContext        * m_avDecContext;
  AVCodecParserContext  * m_avDecParser;
  AVPacket              * m_avDecPacket;

  // bitstream buffer - Decode
  uint8_t               * m_bsDecData;
  uint32_t                m_bsDecValidBytes;
  uint32_t                m_bsDecMaxBytes;
  
  // libav objects - VPP
  struct SwsContext     * m_avVppContext;

  // libav objects - Encode
  const AVCodec         * m_avEncCodec;
  AVCodecContext        * m_avEncContext;
  AVPacket              * m_avEncPacket;

  // libav frames
  AVFrame               * m_avDecFrameOut;
  AVFrame               * m_avVppFrameIn;
  AVFrame               * m_avVppFrameOut;
  AVFrame               * m_avEncFrameIn;

  // other internal state
  mfxU32 m_encCodecId;
};

#endif // _CPU_WORKSTREAM_H