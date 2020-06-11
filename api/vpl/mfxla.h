// Copyright (c) 2017-2019 Intel Corporation
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifndef __MFXLA_H__
#define __MFXLA_H__
#include "mfxdefs.h"
#include "mfxvstructures.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


enum
{
    MFX_EXTBUFF_LOOKAHEAD_CTRL  =   MFX_MAKEFOURCC('L','A','C','T'), /*!< See mfxExtLAControl structure for more details. */
    MFX_EXTBUFF_LOOKAHEAD_STAT  =   MFX_MAKEFOURCC('L','A','S','T'), /*!< See mfxExtLAFrameStatistics structure for more details. */
};

MFX_PACK_BEGIN_USUAL_STRUCT()
/*!
   The mfxExtLAControl structure is used to control standalone look ahead behavior. This LA is performed by ENC class of functions and its
   results are used later by ENCODE class of functions to improve coding efficiency.

   This LA is intended for one to N transcoding scenario, where one input bitstream is transcoded to several output ones with different
   bitrates and resolutions. Usage of integrated into the SDK encoder LA in this scenario is also possible but not efficient in term of
   performance and memory consumption. Standalone LA by ENC class of functions is executed only once for input bitstream in contrast to
   the integrated LA where LA is executed for each of output streams.

   This structure is used at ENC initialization time and should be attached to the mfxVideoParam structure.

   The algorithm of QP calculation is the following:

   - Analyze LookAheadDepth frames to find per-frame costs using a sliding window of DependencyDepth frames.
   - After such analysis we have costs for (LookAheadDepth - DependencyDepth) frames. Cost is the estimation of frame complexity based on inter-prediction.
   - Calculate QP for the first frame using costs of (LookAheadDepth - DependencyDepth) frames.
*/
typedef struct
{
    mfxExtBuffer    Header;  /*!< Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_LOOKAHEAD_CTRL. */
    mfxU16  LookAheadDepth;  /*!< Look ahead depth. This parameter has exactly the same meaning as LookAheadDepth in the mfxExtCodingOption2 structure. */
    mfxU16  DependencyDepth; /*!< Dependency depth. This parameter specifies the number of frames that SDK analyzes to calculate inter-frame
                                  dependency. The recommendation is to set this parameter in the following range: greater than (GopRefDist + 1) and
                                  less than (LookAheadDepth/4). */
    mfxU16  DownScaleFactor; /*!< Down scale factor. This parameter has exactly the same meaning as LookAheadDS in the mfxExtCodingOption2 structure.
                                  It is recommended to execute LA on downscaled image to improve performance without significant quality degradation. */
    mfxU16  BPyramid;        /*!< Turn ON this flag to enable BPyramid feature (this mode is not supported by h264 encoder). See the CodingOptionValue
                             enumerator for values of this option. */

    mfxU16  reserved1[23];

    mfxU16  NumOutStream;    /*!< Number of output streams in one to N transcode scenario. */
    /*! This structure defines utput stream parameters. */
    struct  mfxStream{
        mfxU16  Width;  /*!< Output stream width. */
        mfxU16  Height; /*!< Output stream height. */
        mfxU16  reserved2[14];
    } OutStream[16];    /*!< Output streams. */
}mfxExtLAControl;
MFX_PACK_END()

MFX_PACK_BEGIN_STRUCT_W_L_TYPE()
/*!
   The structure describes LA statistics for the frame in output stream.
*/
typedef struct
{
    mfxU16  Width;             /*!< Output stream width. */
    mfxU16  Height;            /*!< Output stream height. */

    mfxU32  FrameType;         /*!< Output frame type. */
    mfxU32  FrameDisplayOrder; /*!< Output frame number in display order. */
    mfxU32  FrameEncodeOrder;  /*!< Output frame number in encoding order. */

    mfxU32  IntraCost;         /*!< Intra cost of output frame. */
    mfxU32  InterCost;         /*!< Inter cost of output frame. */
    mfxU32  DependencyCost;    /*!< Aggregated dependency cost. It shows how this frame influences subsequent frames. */
    mfxU16  Layer;             /*!< BPyramid layer number. zero if BPyramid is not used. */
    mfxU16  reserved[23];

    mfxU64 EstimatedRate[52];  /*!< Estimated rate for each QP. */
}mfxLAFrameInfo;
MFX_PACK_END()

MFX_PACK_BEGIN_STRUCT_W_PTR()
/*!
   The mfxExtLAFrameStatistics structure is used to pass standalone look ahead statistics to the SDK encoder in one to N transcode scenario.
   This structure is used at runtime and should be attached to the mfxENCOutput structure and then passed, attached, to the mfxEncodeCtrl structure.
*/
typedef struct  {
    mfxExtBuffer    Header;       /*!< Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_LOOKAHEAD_STAT. */

    mfxU16  reserved[20];

    mfxU16  NumAlloc;             /*!< Number of allocated elements in the FrameStat array. */
    mfxU16  NumStream;            /*!< Number of streams in the FrameStat array. */
    mfxU16  NumFrame;             /*!< Number of frames for each stream in the FrameStat array. */
    mfxLAFrameInfo   *FrameStat;  /*!< LA statistics for each frame in output stream. */

    mfxFrameSurface1 *OutSurface; /*!< Output surface. */

} mfxExtLAFrameStatistics;
MFX_PACK_END()

#ifdef __cplusplus
} // extern "C"
#endif /* __cplusplus */


#endif

