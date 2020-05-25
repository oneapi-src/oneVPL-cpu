// Copyright (c) 2012-2019 Intel Corporation
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

#include <Windows.h>
#include <stringapiset.h>

#include <new>
#include <memory>

#include "mfx_dispatcher.h"
#include "mfx_load_dll.h"
#include "mfx_dispatcher_log.h"
#include "mfx_library_iterator.h"
#include "mfx_critical_section.h"

#include <string.h> /* for memset on Linux */

#include <stdlib.h> /* for qsort on Linux */

// module-local definitions
namespace
{

    const
    struct
    {
        // instance implementation type
        eMfxImplType implType;
        // real implementation
        mfxIMPL impl;
        // adapter numbers
        mfxU32 adapterID;

    } implTypes[] =
    {
        // MFX_IMPL_AUTO case
        {MFX_LIB_HARDWARE, MFX_IMPL_HARDWARE,  0},
        {MFX_LIB_SOFTWARE, MFX_IMPL_SOFTWARE,  0},

        // MFX_IMPL_ANY case
        {MFX_LIB_HARDWARE, MFX_IMPL_HARDWARE,  0},
        {MFX_LIB_HARDWARE, MFX_IMPL_HARDWARE2, 1},
        {MFX_LIB_HARDWARE, MFX_IMPL_HARDWARE3, 2},
        {MFX_LIB_HARDWARE, MFX_IMPL_HARDWARE4, 3},
        {MFX_LIB_SOFTWARE, MFX_IMPL_SOFTWARE,  0},
        {MFX_LIB_SOFTWARE, MFX_IMPL_SOFTWARE | MFX_IMPL_AUDIO,  0},
        {MFX_LIB_SOFTWARE_VPL, MFX_IMPL_SOFTWARE_VPL,  0},
    };

    const
    struct
    {
        // start index in implTypes table for specified implementation
        mfxU32 minIndex;
        // last index in implTypes table for specified implementation
        mfxU32 maxIndex;

    } implTypesRange[] =
    {
        {0, 1},  // MFX_IMPL_AUTO
        {1, 1},  // MFX_IMPL_SOFTWARE
        {0, 0},  // MFX_IMPL_HARDWARE
        {2, 6},  // MFX_IMPL_AUTO_ANY
        {2, 5},  // MFX_IMPL_HARDWARE_ANY
        {3, 3},  // MFX_IMPL_HARDWARE2
        {4, 4},  // MFX_IMPL_HARDWARE3
        {5, 5},  // MFX_IMPL_HARDWARE4
        {8, 8},  // MFX_IMPL_SOFTWARE_VPL
        {2, 6},  // MFX_IMPL_RUNTIME, same as MFX_IMPL_HARDWARE_ANY
        {7, 7},  // MFX_IMPL_AUDIO
    };

    MFX::mfxCriticalSection dispGuard = 0;

} // namespace

using namespace MFX;

//
// Implement DLL exposed functions. MFXInit and MFXClose have to do
// slightly more than other. They require to be implemented explicitly.
// All other functions are implemented implicitly.
//

typedef MFXVector<MFX_DISP_HANDLE*> HandleVector;
typedef MFXVector<mfxStatus>        StatusVector;

struct VectorHandleGuard
{
    VectorHandleGuard(HandleVector& aVector): m_vector(aVector) {}
    ~VectorHandleGuard()
    {
        HandleVector::iterator it = m_vector.begin(),
                               et = m_vector.end();
        for ( ; it != et; ++it)
        {
            delete *it;
        }
    }

    HandleVector& m_vector;
private:
    void operator=(const VectorHandleGuard&);
};


int HandleSort (const void * plhs, const void * prhs)
{
    const MFX_DISP_HANDLE * lhs = *(const MFX_DISP_HANDLE **)plhs;
    const MFX_DISP_HANDLE * rhs = *(const MFX_DISP_HANDLE **)prhs;

    if (lhs->actualApiVersion < rhs->actualApiVersion)
    {
        return -1;
    }
    if (rhs->actualApiVersion < lhs->actualApiVersion)
    {
        return 1;
    }

    // if versions are equal prefer library with HW
    if (lhs->loadStatus == MFX_WRN_PARTIAL_ACCELERATION && rhs->loadStatus == MFX_ERR_NONE)
    {
        return 1;
    }
    if (lhs->loadStatus == MFX_ERR_NONE && rhs->loadStatus == MFX_WRN_PARTIAL_ACCELERATION)
    {
        return -1;
    }

    return 0;
}

mfxStatus MFXInitEx(mfxInitParam par, mfxSession *session)
{
    MFX::MFXAutomaticCriticalSection guard(&dispGuard);

    DISPATCHER_LOG_BLOCK( ("MFXInitEx (impl=%s, pVer=%d.%d, ExternalThreads=%d session=0x%p\n"
        , DispatcherLog_GetMFXImplString(par.Implementation).c_str()
        , par.Version.Major
        , par.Version.Minor
        , par.ExternalThreads
        , session));

    mfxStatus mfxRes = MFX_ERR_UNSUPPORTED;
    HandleVector allocatedHandle;
    VectorHandleGuard handleGuard(allocatedHandle);

    MFX_DISP_HANDLE *pHandle;
    wchar_t dllName[MFX_MAX_DLL_PATH] = { 0 };
    MFX::MFXLibraryIterator libIterator;

    // there iterators are used only if the caller specified implicit type like AUTO
    mfxU32 curImplIdx, maxImplIdx;
    // implementation method masked from the input parameter
    // special case for audio library
    const mfxIMPL implMethod = (par.Implementation & MFX_IMPL_AUDIO) ? (sizeof(implTypesRange) / sizeof(implTypesRange[0]) - 1) : (par.Implementation & (MFX_IMPL_VIA_ANY - 1));

    // implementation interface masked from the input parameter
    mfxIMPL implInterface = par.Implementation & -MFX_IMPL_VIA_ANY;
    mfxIMPL implInterfaceOrig = implInterface;
    mfxVersion requiredVersion = {{MFX_VERSION_MINOR, MFX_VERSION_MAJOR}};

    // check error(s)
    if (NULL == session)
    {
        return MFX_ERR_NULL_PTR;
    }

    if (((MFX_IMPL_AUTO > implMethod) || (MFX_IMPL_RUNTIME < implMethod)) && !(par.Implementation & MFX_IMPL_AUDIO))
    {
        return MFX_ERR_UNSUPPORTED;
    }

    // set the minimal required version
    requiredVersion = par.Version;

    try
    {
        // reset the session value
        *session = 0;

        // allocate the dispatching handle and call-table
        pHandle = new MFX_DISP_HANDLE(requiredVersion);
    }
    catch(...)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }

    DISPATCHER_LOG_INFO((("Required API version is %u.%u\n"), requiredVersion.Major, requiredVersion.Minor));
    // particular implementation value
    mfxIMPL curImpl;

    // Load HW library or RT from system location
    curImplIdx = implTypesRange[implMethod].minIndex;
    maxImplIdx = implTypesRange[implMethod].maxIndex;
    do
    {
        int currentStorage = MFX::MFX_STORAGE_ID_FIRST;
        implInterface = implInterfaceOrig;
        do
        {
            // this storage will be checked below
            if (currentStorage == MFX::MFX_APP_FOLDER)
            {
                currentStorage += 1;
                continue;
            }

            // initialize the library iterator
            mfxRes = libIterator.Init(implTypes[curImplIdx].implType,
                implInterface,
                implTypes[curImplIdx].adapterID,
                currentStorage);

            // look through the list of installed SDK version,
            // looking for a suitable library with higher merit value.
            if (MFX_ERR_NONE == mfxRes)
            {

                if (
                    MFX_LIB_HARDWARE == implTypes[curImplIdx].implType
                    && (!implInterface
                    || MFX_IMPL_VIA_ANY == implInterface))
                {
                    implInterface = libIterator.GetImplementationType();
                }

                do
                {
                    eMfxImplType implType = implTypes[curImplIdx].implType;

                    // select a desired DLL
                    mfxRes = libIterator.SelectDLLVersion(dllName,
                        sizeof(dllName) / sizeof(dllName[0]),
                        &implType,
                        pHandle->apiVersion);
                    if (MFX_ERR_NONE != mfxRes)
                    {
                        break;
                    }
                    DISPATCHER_LOG_INFO((("loading library %S\n"), dllName));
                    // try to load the selected DLL
                    curImpl = implTypes[curImplIdx].impl;
                    mfxRes = pHandle->LoadSelectedDLL(dllName, implType, curImpl, implInterface, par);
                    // unload the failed DLL
                    if (MFX_ERR_NONE != mfxRes)
                    {
                        pHandle->Close();
                    }
                    else
                    {
                        libIterator.GetSubKeyName(pHandle->subkeyName, sizeof(pHandle->subkeyName) / sizeof(pHandle->subkeyName[0]));
                        pHandle->storageID = libIterator.GetStorageID();
                        allocatedHandle.push_back(pHandle);
                        pHandle = new MFX_DISP_HANDLE(requiredVersion);
                    }

                } while (MFX_ERR_NONE != mfxRes);
            }

            // select another place for loading engine
            currentStorage += 1;

        } while ((MFX_ERR_NONE != mfxRes) && (MFX::MFX_STORAGE_ID_LAST >= currentStorage));

    } while ((MFX_ERR_NONE != mfxRes) && (++curImplIdx <= maxImplIdx));

    curImplIdx = implTypesRange[implMethod].minIndex;
    maxImplIdx = implTypesRange[implMethod].maxIndex;

    // Load RT from app folder (libmfxsw64 with API >= 1.10)
    do
    {
        implInterface = implInterfaceOrig;
        // initialize the library iterator
        mfxRes = libIterator.Init(implTypes[curImplIdx].implType,
            implInterface,
            implTypes[curImplIdx].adapterID,
            MFX::MFX_APP_FOLDER);

        if (MFX_ERR_NONE == mfxRes)
        {

            if (
                MFX_LIB_HARDWARE == implTypes[curImplIdx].implType
                && (!implInterface
                || MFX_IMPL_VIA_ANY == implInterface))
            {
                implInterface = libIterator.GetImplementationType();
            }

            do
            {
                eMfxImplType implType;

                // select a desired DLL
                mfxRes = libIterator.SelectDLLVersion(dllName,
                    sizeof(dllName) / sizeof(dllName[0]),
                    &implType,
                    pHandle->apiVersion);
                if (MFX_ERR_NONE != mfxRes)
                {
                    break;
                }
                DISPATCHER_LOG_INFO((("loading library %S\n"), dllName));

                // try to load the selected DLL
                curImpl = implTypes[curImplIdx].impl;
                mfxRes = pHandle->LoadSelectedDLL(dllName, implType, curImpl, implInterface, par);
                // unload the failed DLL
                if (MFX_ERR_NONE != mfxRes)
                {
                    pHandle->Close();
                }
                else
                {
                    if (pHandle->actualApiVersion.Major == 1 && pHandle->actualApiVersion.Minor <= 9)
                    {
                        // this is not RT, skip it
                        mfxRes = MFX_ERR_ABORTED;
                        break;
                    }
                    pHandle->storageID = MFX::MFX_UNKNOWN_KEY;
                    allocatedHandle.push_back(pHandle);
                    pHandle = new MFX_DISP_HANDLE(requiredVersion);
                }

            } while (MFX_ERR_NONE != mfxRes);
        }
    } while ((MFX_ERR_NONE != mfxRes) && (++curImplIdx <= maxImplIdx));

    // Load HW and SW libraries using legacy default DLL search mechanism
    // set current library index again
    curImplIdx = implTypesRange[implMethod].minIndex;
    do
    {
        implInterface = implInterfaceOrig;

        if (par.Implementation & MFX_IMPL_AUDIO)
        {
            mfxRes = MFX::mfx_get_default_audio_dll_name(dllName,
                sizeof(dllName) / sizeof(dllName[0]),
                implTypes[curImplIdx].implType);
        }
        else if (par.Implementation & MFX_IMPL_SOFTWARE_VPL)
        {
            mfxRes = MFX::mfx_get_default_dll_name(dllName,
                sizeof(dllName) / sizeof(dllName[0]),
                MFX_LIB_SOFTWARE_VPL);
        }
        else
        {
            mfxRes = MFX::mfx_get_default_dll_name(dllName,
                sizeof(dllName) / sizeof(dllName[0]),
                implTypes[curImplIdx].implType);
        }

        if (MFX_ERR_NONE == mfxRes)
        {
            DISPATCHER_LOG_INFO((("loading default library %S\n"), dllName))

                // try to load the selected DLL using default DLL search mechanism
                if (MFX_LIB_HARDWARE == implTypes[curImplIdx].implType)
                {
                    if (!implInterface)
                    {
                        implInterface = MFX_IMPL_VIA_ANY;
                    }
                    mfxU32 curVendorID = 0, curDeviceID = 0;
                    mfxRes = MFX::SelectImplementationType(implTypes[curImplIdx].adapterID, &implInterface, &curVendorID, &curDeviceID);
                    if (curVendorID != INTEL_VENDOR_ID)
                        mfxRes = MFX_ERR_UNKNOWN;
                }
                if (MFX_ERR_NONE == mfxRes)
                {
                    // try to load the selected DLL using default DLL search mechanism
                    mfxRes = pHandle->LoadSelectedDLL(dllName,
                        implTypes[curImplIdx].implType,
                        implTypes[curImplIdx].impl,
                        implInterface,
                        par);
                }
                // unload the failed DLL
                if ((MFX_ERR_NONE != mfxRes) &&
                    (MFX_WRN_PARTIAL_ACCELERATION != mfxRes))
                {
                    pHandle->Close();
                }
                else
                {
                    pHandle->storageID = MFX::MFX_UNKNOWN_KEY;
                    allocatedHandle.push_back(pHandle);
                    pHandle = new MFX_DISP_HANDLE(requiredVersion);
                }
        }
    }
    while ((MFX_ERR_NONE > mfxRes) && (++curImplIdx <= maxImplIdx));
    delete pHandle;

    if (allocatedHandle.size() == 0)
        return MFX_ERR_UNSUPPORTED;

    { // sort candidate list
        bool NeedSort = false;
        HandleVector::iterator first = allocatedHandle.begin(),
            it = allocatedHandle.begin(),
            et = allocatedHandle.end();
        for (it++; it != et; ++it)
            if (HandleSort(&(*first), &(*it)) != 0)
                NeedSort = true;

        // select dll with version with lowest version number still greater or equal to requested
        if (NeedSort)
            qsort(&(*allocatedHandle.begin()), allocatedHandle.size(), sizeof(MFX_DISP_HANDLE*), &HandleSort);
    }
    HandleVector::iterator candidate = allocatedHandle.begin();
    // check the final result of loading
    try
    {
        pHandle = *candidate;
        //pulling up current mediasdk version, that required to match plugin version
        mfxVersion apiVerActual = { { 0, 0 } };
        mfxStatus stsQueryVersion = MFXQueryVersion((mfxSession)pHandle, &apiVerActual);

        if (MFX_ERR_NONE !=  stsQueryVersion)
        {
            DISPATCHER_LOG_ERROR((("MFXQueryVersion returned: %d, cannot load plugins\n"), mfxRes))
        }
    }
    catch(...)
    {
        DISPATCHER_LOG_ERROR((("unknown exception while loading plugins\n")))
    }

    // everything is OK. Save pointers to the output variable
    *candidate = 0; // keep this one safe from guard destructor
    *((MFX_DISP_HANDLE **) session) = pHandle;

    return pHandle->loadStatus;

} // mfxStatus MFXInitEx(mfxIMPL impl, mfxVersion *ver, mfxSession *session)

mfxStatus MFXClose(mfxSession session)
{
    MFX::MFXAutomaticCriticalSection guard(&dispGuard);

    mfxStatus mfxRes = MFX_ERR_INVALID_HANDLE;
    MFX_DISP_HANDLE *pHandle = (MFX_DISP_HANDLE *) session;

    // check error(s)
    if (pHandle)
    {
        try
        {
            // unload the DLL library
            mfxRes = pHandle->Close();

            // it is possible, that there is an active child session.
            // can't unload library in that case.
            if (MFX_ERR_UNDEFINED_BEHAVIOR != mfxRes)
            {
                // release the handle
                delete pHandle;
            }
        }
        catch(...)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
    }

    return mfxRes;

} // mfxStatus MFXClose(mfxSession session)

mfxStatus MFXJoinSession(mfxSession session, mfxSession child_session)
{
    mfxStatus mfxRes = MFX_ERR_INVALID_HANDLE;
    MFX_DISP_HANDLE *pHandle = (MFX_DISP_HANDLE *)session;
    MFX_DISP_HANDLE *pChildHandle = (MFX_DISP_HANDLE *)child_session;

    // get the function's address and make a call
    if ((pHandle) && (pChildHandle) && (pHandle->apiVersion == pChildHandle->apiVersion))
    {
        /* check whether it is audio session or video */
        int tableIndex = eMFXJoinSession;
        mfxFunctionPointer pFunc;
        {
            pFunc = pHandle->callTable[tableIndex];
        }

        if (pFunc)
        {
            // pass down the call
            mfxRes = (*(mfxStatus(MFX_CDECL *) (mfxSession, mfxSession)) pFunc) (pHandle->session,
                pChildHandle->session);
        }
    }

    return mfxRes;

} // mfxStatus MFXJoinSession(mfxSession session, mfxSession child_session)

mfxStatus MFXCloneSession(mfxSession session, mfxSession *clone)
{
    mfxStatus mfxRes = MFX_ERR_INVALID_HANDLE;
    MFX_DISP_HANDLE *pHandle = (MFX_DISP_HANDLE *)session;
    mfxVersion apiVersion;
    mfxIMPL impl;

    // check error(s)
    if (pHandle)
    {
        // initialize the clone session
        apiVersion = pHandle->apiVersion;
        impl = pHandle->impl | pHandle->implInterface;
        mfxRes = MFXInit(impl, &apiVersion, clone);
        if (MFX_ERR_NONE != mfxRes)
        {
            return mfxRes;
        }

        // join the sessions
        mfxRes = MFXJoinSession(session, *clone);
        if (MFX_ERR_NONE != mfxRes)
        {
            MFXClose(*clone);
            *clone = NULL;
            return mfxRes;
        }
    }

    return mfxRes;

} // mfxStatus MFXCloneSession(mfxSession session, mfxSession *clone)

mfxStatus MFXInit(mfxIMPL impl, mfxVersion *pVer, mfxSession *session)
{
    mfxInitParam par = {};

    par.Implementation = impl;
    if (pVer)
    {
        par.Version = *pVer;
    }
    else
    {
        par.Version.Major = DEFAULT_API_VERSION_MAJOR;
        par.Version.Minor = DEFAULT_API_VERSION_MINOR;
    }
    par.ExternalThreads = 0;

    return MFXInitEx(par, session);
}

//
//
// implement all other calling functions.
// They just call a procedure of DLL library from the table.
//

// define for common functions (from mfxsession.h)
#undef FUNCTION
#define FUNCTION(return_value, func_name, formal_param_list, actual_param_list) \
    return_value func_name formal_param_list \
{ \
    mfxStatus mfxRes = MFX_ERR_INVALID_HANDLE; \
     _mfxSession *pHandle = (_mfxSession *) session; \
    /* get the function's address and make a call */ \
    if (pHandle) \
{ \
    /* check whether it is audio session or video */ \
    int tableIndex = e##func_name; \
    mfxFunctionPointer pFunc; \
    if (0) \
{ \
} \
        else \
{ \
    pFunc = pHandle->callTable[tableIndex]; \
} \
    if (pFunc) \
{ \
    /* get the real session pointer */ \
    session = pHandle->session; \
    /* pass down the call */ \
    mfxRes = (*(mfxStatus (MFX_CDECL  *) formal_param_list) pFunc) actual_param_list; \
} \
} \
    return mfxRes; \
}

FUNCTION(mfxStatus, MFXQueryIMPL, (mfxSession session, mfxIMPL *impl), (session, impl))
FUNCTION(mfxStatus, MFXQueryVersion, (mfxSession session, mfxVersion *version), (session, version))

// these functions are not necessary in LOADER part of dispatcher and
// need to be included only in in SOLID dispatcher or PROCTABLE part of dispatcher

FUNCTION(mfxStatus, MFXDisjoinSession, (mfxSession session), (session))
FUNCTION(mfxStatus, MFXSetPriority, (mfxSession session, mfxPriority priority), (session, priority))
FUNCTION(mfxStatus, MFXGetPriority, (mfxSession session, mfxPriority *priority), (session, priority))

#undef FUNCTION
#define FUNCTION(return_value, func_name, formal_param_list, actual_param_list) \
    return_value func_name formal_param_list \
{ \
    mfxStatus mfxRes = MFX_ERR_INVALID_HANDLE; \
     _mfxSession *pHandle = (_mfxSession *) session;\
    /* get the function's address and make a call */ \
    if (pHandle) \
{ \
    mfxFunctionPointer pFunc = pHandle->callTable[e##func_name]; \
    if (pFunc) \
{ \
    /* get the real session pointer */ \
    session = pHandle->session; \
    /* pass down the call */ \
    mfxRes = (*(mfxStatus (MFX_CDECL  *) formal_param_list) pFunc) actual_param_list; \
} \
} \
    return mfxRes; \
}

#include "mfx_exposed_functions_list.h"
#undef FUNCTION
#define FUNCTION(return_value, func_name, formal_param_list, actual_param_list) \
    return_value func_name formal_param_list \
{ \
    mfxStatus mfxRes = MFX_ERR_INVALID_HANDLE; \
     _mfxSession *pHandle = (_mfxSession *) session; \
    /* get the function's address and make a call */ \
    if (pHandle) \
{ \
    mfxFunctionPointer pFunc = pHandle->callAudioTable[e##func_name]; \
    if (pFunc) \
{ \
    /* get the real session pointer */ \
    session = pHandle->session; \
    /* pass down the call */ \
    mfxRes = (*(mfxStatus (MFX_CDECL  *) formal_param_list) pFunc) actual_param_list; \
} \
} \
    return mfxRes; \
}

