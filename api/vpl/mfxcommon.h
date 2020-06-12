// Copyright (c) 2018-2019 Intel Corporation
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
#ifndef API_VPL_MFXCOMMON_H_
#define API_VPL_MFXCOMMON_H_
#include "./mfxdefs.h"

#if !defined(__GNUC__)
    #pragma warning(disable : 4201)
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MFX_MAKEFOURCC(A, B, C, D)                            \
    (((static_cast<int>(A))) + ((static_cast<int>(B)) << 8) + \
     ((static_cast<int>(C)) << 16) + ((static_cast<int>(D)) << 24))

/* Extended Configuration Header Structure */
MFX_PACK_BEGIN_USUAL_STRUCT()
/*! This structure is the common header definition for external buffers and video
    processing hints. */
typedef struct {
    mfxU32
        BufferId; /*!< Identifier of the buffer content. See the ExtendedBufferID enumerator for a complete list of extended buffers. */
    mfxU32 BufferSz; /*!< Size of the buffer. */
} mfxExtBuffer;
MFX_PACK_END()

/* Library initialization and deinitialization */
/*!
    This enumerator itemizes SDK implementation types.
    The implementation type is a bit OR’ed value of the base type and any decorative flags.
 */
typedef mfxI32 mfxIMPL;
/*! 
    The application can use the macro MFX_IMPL_BASETYPE(x) to obtain the base implementation type.
*/
#define MFX_IMPL_BASETYPE(x) (0x00ff & (x))

enum {
    MFX_IMPL_AUTO     = 0x0000, /*!< Auto Selection/In or Not Supported/Out */
    MFX_IMPL_SOFTWARE = 0x0001, /*!< Pure Software Implementation */
    MFX_IMPL_HARDWARE =
        0x0002, /*!< Hardware Accelerated Implementation (default device) */
    MFX_IMPL_AUTO_ANY =
        0x0003, /*!< Auto selection of any hardware/software implementation */
    MFX_IMPL_HARDWARE_ANY =
        0x0004, /*!< Auto selection of any hardware implementation */
    MFX_IMPL_HARDWARE2 =
        0x0005, /*!< Hardware accelerated implementation (2nd device) */
    MFX_IMPL_HARDWARE3 =
        0x0006, /*!< Hardware accelerated implementation (3rd device) */
    MFX_IMPL_HARDWARE4 =
        0x0007, /*!< Hardware accelerated implementation (4th device) */
    MFX_IMPL_SOFTWARE_VPL = 0x0008, /*!< VPL Software Implementation */
    MFX_IMPL_RUNTIME      = 0x0009,

#if (MFX_VERSION >= MFX_VERSION_NEXT)
    MFX_IMPL_SINGLE_THREAD = 0x0010,
#endif
    MFX_IMPL_VIA_ANY =
        0x0100, /*!< Hardware acceleration can go through any supported OS infrastructure. This is default value,
                                          it is used by the SDK if none of MFX_IMPL_VIA_xxx flag is specified by application. */
    MFX_IMPL_VIA_D3D9 =
        0x0200, /*!< Hardware acceleration goes through the Microsoft* Direct3D9* infrastructure. */
    MFX_IMPL_VIA_D3D11 =
        0x0300, /*!< Hardware acceleration goes through the Microsoft* Direct3D11* infrastructure. */
    MFX_IMPL_VIA_VAAPI =
        0x0400, /*!< Hardware acceleration goes through the Linux* VA API infrastructure. */

    MFX_IMPL_AUDIO = 0x8000,
#if (MFX_VERSION >= MFX_VERSION_NEXT)
    MFX_IMPL_EXTERNAL_THREADING = 0x10000,
#endif

    MFX_IMPL_UNSUPPORTED = 0x0000 /*!< One of the MFXQueryIMPL returns */
};

/* Version Info */
MFX_PACK_BEGIN_USUAL_STRUCT()
/*! The mfxVersion union describes the version of the SDK implementation.*/
typedef union {
    /*! @brief Structure with Major and Minor fields.  */
    /*! @struct Anonimouse */
    struct {
        mfxU16 Minor; /*!< Minor number of the SDK implementation */
        mfxU16 Major; /*!< Major number of the SDK implementation */
    };
    mfxU32 Version; /*!< SDK implementation version number */
} mfxVersion;
MFX_PACK_END()

/* MFX structures version info */
MFX_PACK_BEGIN_USUAL_STRUCT()
/*! Introduce field Version for any structures with modifications after API 1.XX.
Minor number is incremented when reserved fields are used, 
major number is incremnted when size of structure is increased.
Assumed that any structure changes are backward binary compatible
*/
typedef union {
    /*! Structure with Major and Minor fields.  */
    /*! @struct Anonimouse */
    struct {
        mfxU8 Minor; /*!< Minor number of the correspondent structure */
        mfxU8 Major; /*!< Major number of the correspondent structure  */
    };
    mfxU16 Version; /*!< Structure version number */
} mfxStructVersion;
MFX_PACK_END()

/*! The mfxPriority enumerator describes the session priority. */
typedef enum {
    MFX_PRIORITY_LOW =
        0, /*!< Low priority: the session operation halts when high priority tasks are executing and more than 75% of the CPU is being used for normal priority tasks.*/
    MFX_PRIORITY_NORMAL =
        1, /*!< Normal priority: the session operation is halted if there are high priority tasks.*/
    MFX_PRIORITY_HIGH =
        2 /*!< High priority: the session operation blocks other lower priority session operations.*/

} mfxPriority;

typedef struct _mfxEncryptedData mfxEncryptedData;
MFX_PACK_BEGIN_STRUCT_W_L_TYPE()
/*! The mfxBitstream structure defines the buffer that holds compressed video data. */
typedef struct {
    /*! @internal :unnamed(union) @endinternal */
    union {
        struct {
            mfxEncryptedData *EncryptedData; /*!< Reserved and must be zero. */
            mfxExtBuffer **
                ExtParam; /*!< Array of extended buffers for additional bitstream configuration. See the ExtendedBufferID enumerator for a complete list of extended buffers. */
            mfxU16
                NumExtParam; /*!< The number of extended buffers attached to this structure. */
        };
        mfxU32 reserved[6];
    };
    /*! Decode time stamp of the compressed bitstream in units of 90KHz. A value of MFX_TIMESTAMP_UNKNOWN indicates that there is no time stamp. 
        This value is calculated by the SDK encoder from presentation time stamp provided by the application in mfxFrameSurface1 structure and
        from frame rate provided by the application during the SDK encoder initialization. */
    mfxI64 DecodeTimeStamp;
    mfxU64
        TimeStamp; /*!< Time stamp of the compressed bitstream in units of 90KHz. A value of MFX_TIMESTAMP_UNKNOWN indicates that there is no time stamp. */
    mfxU8 *Data; /*!< Bitstream buffer pointer, 32-bytes aligned */
    mfxU32
        DataOffset; /*!< Next reading or writing position in the bitstream buffer */
    mfxU32 DataLength; /*!< Size of the actual bitstream data in bytes */
    mfxU32 MaxLength; /*!< Allocated bitstream buffer size in bytes */

    mfxU16
        PicStruct; /*!< Type of the picture in the bitstream; this is an output parameter. */
    mfxU16
        FrameType; /*!< Frame type of the picture in the bitstream; this is an output parameter. */
    mfxU16
        DataFlag; /*!< Indicates additional bitstream properties; see the BitstreamDataFlag enumerator for details. */
    mfxU16 reserved2; /*!< Reserved for future use. */
} mfxBitstream;
MFX_PACK_END()

/*! Syncronization point object handle */
typedef struct _mfxSyncPoint *mfxSyncPoint;

/*! The GPUCopy enumerator controls usage of GPU accelerated copying between video and system memory in the SDK components. */
enum {
    MFX_GPUCOPY_DEFAULT =
        0, /*!< Use default mode for the current SDK implementation. */
    MFX_GPUCOPY_ON  = 1, /*!< Enable GPU accelerated copying. */
    MFX_GPUCOPY_OFF = 2 /*!< Disable GPU accelerated copying. */
};

MFX_PACK_BEGIN_STRUCT_W_PTR()
/*! This structure specifies advanced initialization parameters.
    A zero value in any of the fields indicates that the corresponding field
    is not explicitly specified.
*/
typedef struct {
    mfxIMPL
        Implementation; /*!< mfxIMPL enumerator that indicates the desired SDK implementation */
    mfxVersion
        Version; /*!< Structure which specifies minimum library version or zero, if not specified */
    mfxU16
        ExternalThreads; /*!< Desired threading mode. Value 0 means internal threading, 1 – external. */
    /*! @internal :unnamed(union) @endinternal */
    union {
        struct {
            mfxExtBuffer **
                ExtParam; /*!< Points to an array of pointers to the extra configuration structures; see the ExtendedBufferID enumerator for a list of extended configurations. */
            mfxU16
                NumExtParam; /*!< The number of extra configuration structures attached to this structure. */
        };
        mfxU16 reserved2[5];
    };
    mfxU16
        GPUCopy; /*!< Enables or disables GPU accelerated copying between video and system memory in the SDK components. See the GPUCopy enumerator for a list of valid values. */
    mfxU16 reserved[21];
} mfxInitParam;
MFX_PACK_END()

enum {
    MFX_EXTBUFF_THREADS_PARAM =
        MFX_MAKEFOURCC('T', 'H', 'D', 'P') /*!< mfxExtThreadsParam buffer ID */
};

MFX_PACK_BEGIN_USUAL_STRUCT()
/*! Attached to the mfxInitParam structure during the SDK session initialization,
    mfxExtThreadsParam structure specifies options for threads created by this session. */
typedef struct {
    mfxExtBuffer Header; /*!< Must be MFX_EXTBUFF_THREADS_PARAM */

    mfxU16 NumThread; /*!< The number of threads. */
    mfxI32 SchedulingType; /*!< Scheduling policy for all threads. */
    mfxI32 Priority; /*!< Priority for all threads. */
    mfxU16 reserved[55]; /*!< Reserved for future use */
} mfxExtThreadsParam;
MFX_PACK_END()

/*! The PlatformCodeName enumerator itemizes Intel® microarchitecture code names. For details about any particular code name, see ark.intel.com. */
enum {
    MFX_PLATFORM_UNKNOWN     = 0, /*!< Unknown platform */
    MFX_PLATFORM_SANDYBRIDGE = 1, /*!< Sandy Bridge */
    MFX_PLATFORM_IVYBRIDGE   = 2, /*!< Ivy Bridge */
    MFX_PLATFORM_HASWELL     = 3, /*!< Haswell */
    MFX_PLATFORM_BAYTRAIL    = 4, /*!< Bay Trail*/
    MFX_PLATFORM_BROADWELL   = 5, /*!< Broadwell */
    MFX_PLATFORM_CHERRYTRAIL = 6, /*!< Cherry Trail */
    MFX_PLATFORM_SKYLAKE     = 7, /*!< Skylake */
    MFX_PLATFORM_APOLLOLAKE  = 8, /*!< Apollo Lake */
    MFX_PLATFORM_KABYLAKE    = 9, /*!< Kaby Lake */
#if (MFX_VERSION >= 1025)
    MFX_PLATFORM_GEMINILAKE = 10, /*!< Gemini Lake */
    MFX_PLATFORM_COFFEELAKE = 11, /*!< Coffe Lake */
    MFX_PLATFORM_CANNONLAKE = 20, /*!< Cannon Lake */
#endif
#if (MFX_VERSION >= 1027)
    MFX_PLATFORM_ICELAKE = 30, /*!< Ice Lake */
#endif
    MFX_PLATFORM_JASPERLAKE  = 32, /*!< Jasper Lake */
    MFX_PLATFORM_ELKHARTLAKE = 33, /*!< Elkhart Lake*/
    MFX_PLATFORM_TIGERLAKE   = 40, /*!< Tiger Lake */
};

#if (MFX_VERSION >= 1031)
/*! The mfxMediaAdapterType enumerator itemizes types of Intel Gen Graphics adapters. */
typedef enum {
    MFX_MEDIA_UNKNOWN    = 0xffff, /*!< Unknown type. */
    MFX_MEDIA_INTEGRATED = 0, /*!< Integrated Intel Gen Graphics adapter. */
    MFX_MEDIA_DISCRETE   = 1 /*!< Discrete Intel Gen Graphics adapter. */
} mfxMediaAdapterType;
#endif

MFX_PACK_BEGIN_USUAL_STRUCT()
/*! The mfxPlatform structure contains information about hardware platform. */
typedef struct {
    mfxU16
        CodeName; /*!< Intel® microarchitecture code name. See the PlatformCodeName enumerator for a list of possible values. */
    mfxU16 DeviceId; /*!< Unique identifier of graphics device. */
#if (MFX_VERSION >= 1031)
    mfxU16
        MediaAdapterType; /*!< Description of Intel Gen Graphics adapter type. See the mfxMediaAdapterType enumerator for a list of possible values. */
    mfxU16 reserved[13]; /*!< Reserved for future use. */
#else
    mfxU16 reserved[14]; /*!< Reserved for future use. */
#endif
} mfxPlatform;
MFX_PACK_END()

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // API_VPL_MFXCOMMON_H_
