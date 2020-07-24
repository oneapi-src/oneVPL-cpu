====================================
Supported Parameter List
====================================


Decode
------

|

**Input:**

+---------------------------+------------------------------------------+
| Parameter                 |  Accepted values                         | 
+===========================+==========================================+
| CodecId                   | MFX_CODEC_HEVC MFX_CODEC_AV1             |
|                           | MFX_CODEC_AVC MFX_CODEC_JPEG             | 
+---------------------------+------------------------------------------+
| mfxBitstream.DataFlag     | MFX_BITSTREAM_COMPLETE_FRAME             | 
|                           | MFX_BITSTREAM_EOS                        |
+---------------------------+------------------------------------------+


|

**Output:**

+---------------------------+------------------------------------------+
| Parameter                 |  Accepted values                         | 
+===========================+==========================================+
| FrameInfo.FourCC          | MFX_FOURCC_I420==MFX_FOURCC_IYUV         |
|                           | MFX_FOURCC_I010                          |
|                           | MFX_FOURCC_RGB4                          |
+---------------------------+------------------------------------------+
| IOPattern                 | MFX_IOPATTERN_OUT_SYSTEM_MEMORY          |
+---------------------------+------------------------------------------+


|
|
|

Encode
------

|

**Input:**

+---------------------------+------------------------------------------+
| Parameter                 |  Accepted values                         | 
+===========================+==========================================+
| IOPattern                 | MFX_IOPATTERN_IN_SYSTEM_MEMORY           | 
+---------------------------+------------------------------------------+
| FrameInfo.FourCC          | MFX_FOURCC_I420==MFX_FOURCC_IYUV         |
|                           | MFX_FOURCC_I010                          |
+---------------------------+------------------------------------------+



|

**Output:**

+-------------------------------+------------------------------------------+
| Parameter                     |  Accepted values                         |
+===============================+==========================================+
| CodecID                       | MFX_CODEC_HEVC MFX_CODEC_AV1             |
|                               | MFX_CODEC_AVC MFX_CODEC_JPEG             |
+-------------------------------+------------------------------------------+
| FrameInfo.Width               | [64 - 8192] HEVC, [64 - 4096] AV1        | 
|                               | [64 - 4096] AVC, [64-8192] JPEG          |   
+-------------------------------+------------------------------------------+
| FrameInfo.Height              | [64 - 4320] HEVC, [16 - 2304] AV1        |
|                               | [64 - 2304] AVC, [64-8192] JPEG          |
+-------------------------------+------------------------------------------+
| GopPicSize                    | 0-65535                                  |
+-------------------------------+------------------------------------------+
| GopRefDist                    | 0-65535                                  |
+-------------------------------+------------------------------------------+
| TargetKbps                    | 1-65535                                  |
+-------------------------------+------------------------------------------+
| FrameInfo.FrameRateExtN       | 1-65535                                  |
+-------------------------------+------------------------------------------+
| FrameInfo.FrameRateExtD       | 1-65535                                  |
+-------------------------------+------------------------------------------+
| FrameInfo.AspectRatioH        | 1-65535                                  |
+-------------------------------+------------------------------------------+
| FrameInfo.AspectRatioW        | 1-65535                                  |
+-------------------------------+------------------------------------------+
| NumSlice                      | 0-65535 (depends on resolution)          |
+-------------------------------+------------------------------------------+
| NumRefFrame                   | 0-65535                                  |
+-------------------------------+------------------------------------------+
| GopOptFlag                    | MFX_GOP_CLOSED                           |
+-------------------------------+------------------------------------------+
| FrameInfo.BitDepthChroma      | 8, 10                                    |
+-------------------------------+------------------------------------------+
| RateControlMethod             | HEVC: CQP, VBR  AV1: CQP, VBR, CVBR=CBR  |
|                               | AVC: CQP, VBR, CBR                       |
+-------------------------------+------------------------------------------+
| QP (I/P/B)                    | [0-51] HEVC/AVC, [0-63] AV1              |
| (use QPI for general QP)      |                                          |
+-------------------------------+------------------------------------------+
| InitialDelayInKB              | 0-65535                                  |
+-------------------------------+------------------------------------------+
| BufferSizeInKB                | 0-65535                                  |
+-------------------------------+------------------------------------------+
| MaxKbps                       | 0-65535                                  |
+-------------------------------+------------------------------------------+
| CodecProfile                  | see below                                |
+-------------------------------+------------------------------------------+
| CodecLevel                    | see below                                |
+-------------------------------+------------------------------------------+
| TargetUsage                   | 1(best quality)  -7(best speed)          |
+-------------------------------+------------------------------------------+
| FrameInfo.BitDepthLuma        | 8, 10                                    |
+-------------------------------+------------------------------------------+
| FrameData.Timestamp           | 0-4,294,967,295                          |
+-------------------------------+------------------------------------------+
| FrameData.PitchLow            | 0-65535                                  |
| FrameData.PitchHigh           |                                          |
+-------------------------------+------------------------------------------+
| FrameData.DataFlag            | 0, MFX_FRAMEDATA_ORIGINAL_TIMESTAMP      |
+-------------------------------+------------------------------------------+
| BRCParamMultiplier            | 0-65535                                  +
+-------------------------------+------------------------------------------+
| FrameInfo.Quality (JPEG)      | 1-100                                    |
+-------------------------------+------------------------------------------+


|
|

**Encode Profiles:**

HEVC and AV1 introduced in Beta08.  AV1 profiles to be added by a future spec version.

+-------------------------------+-------------------------------+
| HEVC Profiles                 | AV1 Profiles                  |
+===============================+===============================+
| MFX_PROFILE_HEVC_MAIN         |                               |
| MFX_PROFILE_HEVC_MAIN10       |                               |
+-------------------------------+-------------------------------+

|

AVC and JPEG introduced in Beta09.  

+-------------------------------+-------------------------------+
| AVC Profiles                  | JPEG Profiles                 |
+===============================+===============================+
| MFX_PROFILE_AVC_BASELINE      |  MFX_PROFILE_JPEG_BASELINE    |
| MFX_PROFILE_AVC_MAIN          |                               |
| MFX_PROFILE_AVC_HIGH          |                               |
+-------------------------------+-------------------------------+


|
|

**Encode Levels:**

HEVC and AV1 introduced in Beta08.  AV1 levels to be added by a future spec version


===================        ==============
    HEVC Levels            AV1 levels
===================        ==============
 MFX_LEVEL_HEVC_1           future 
 MFX_LEVEL_HEVC_2       
 MFX_LEVEL_HEVC_21    
 MFX_LEVEL_HEVC_3     
 MFX_LEVEL_HEVC_31     
 MFX_LEVEL_HEVC_4      
 MFX_LEVEL_HEVC_41     
 MFX_LEVEL_HEVC_5       
 MFX_LEVEL_HEVC_51      
 MFX_LEVEL_HEVC_52       
 MFX_LEVEL_HEVC_6       
 MFX_LEVEL_HEVC_61       
 MFX_LEVEL_HEVC_62      
===================        ==============

Note: high tier is implemented by or (|) with MFX_TIER_HEVC_HIGH (0x100).

|
|

AVC and JPEG introduced in Beta09.  No levels for JPEG.

=================   ==============
    AVC Levels       MJPEG levels
=================   ==============
MFX_LEVEL_AVC_1       n/a 
MFX_LEVEL_AVC_1b    
MFX_LEVEL_AVC_11    
MFX_LEVEL_AVC_12    
MFX_LEVEL_AVC_13    
MFX_LEVEL_AVC_2     
MFX_LEVEL_AVC_21    
MFX_LEVEL_AVC_22    
MFX_LEVEL_AVC_3     
MFX_LEVEL_AVC_31    
MFX_LEVEL_AVC_32    
MFX_LEVEL_AVC_4     
MFX_LEVEL_AVC_41    
MFX_LEVEL_AVC_42    
MFX_LEVEL_AVC_5     
MFX_LEVEL_AVC_51    
MFX_LEVEL_AVC_52     
=================   ==============

|
|

VPP
------

VPP parameters can be used for input and output.
All VPP features introduced in beta09. 

+-------------------------------+------------------------------------------+
| Parameter                     |  Accepted values                         |
+===============================+==========================================+
| FrameInfo.Width               |  16-8192                                 |
| FrameInfo.Height              |                                          |
+-------------------------------+------------------------------------------+
| FrameInfo.CropX               |  0- 8192                                 |
| FrameInfo.CropY               |                                          |
| FrameInfo.CropW               |                                          |
| FrameInfo.CropH               |                                          |
+-------------------------------+------------------------------------------+ 
| FrameInfo.FourCC              | MFX_FOURCC_I420==MFX_FOURCC_IYUV         |
|                               | MFX_FOURCC_I010                          | 
|                               | MFX_FOURCC_RGB4                          |
+-------------------------------+------------------------------------------+
