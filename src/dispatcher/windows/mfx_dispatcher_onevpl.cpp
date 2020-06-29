/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "windows/mfx_dispatcher_onevpl.h"

LoaderCtxOneVPL::LoaderCtxOneVPL()
        : m_configCtx(),
          m_libInfoList(),
          m_vplPackageDir() {
    return;
}

LoaderCtxOneVPL::~LoaderCtxOneVPL() {
    return;
}

mfxStatus LoaderCtxOneVPL::SearchDirForLibs(wchar_t* searchDir,
                                            std::list<LibInfo*>& libInfoList,
                                            mfxU32 priority) {
    wchar_t testFileName[MFX_MAX_DLL_PATH] = {};
    HANDLE hTestFile                       = nullptr;
    WIN32_FIND_DATAW testFileData;
    DWORD err;

    // okay to call with empty searchDir
    if (searchDir == nullptr || searchDir[0] == 0)
        return MFX_ERR_NONE;

    // search for all files named *.dll in given path
    wcscpy_s(testFileName, MFX_MAX_DLL_PATH, searchDir);
    wcscat_s(testFileName, MFX_MAX_DLL_PATH, L"*.dll");

    // iterate over all candidate files in directory
    hTestFile = FindFirstFileW(testFileName, &testFileData);
    if (hTestFile != INVALID_HANDLE_VALUE) {
        do {
            LibInfo* libInfo = new LibInfo;
            if (!libInfo)
                return MFX_ERR_MEMORY_ALLOC;
            memset(libInfo, 0, sizeof(LibInfo));

            err = GetFullPathNameW(testFileData.cFileName,
                                   MFX_MAX_DLL_PATH,
                                   libInfo->libNameFull,
                                   &libInfo->libNameBase);
            if (!err) {
                // unknown error - skip it and move on to next file
                delete libInfo;
                continue;
            }
            libInfo->libPriority = priority;

            // add to list
            libInfoList.push_back(libInfo);
        } while (FindNextFileW(hTestFile, &testFileData));

        FindClose(hTestFile);
    }

    return MFX_ERR_NONE;
}

// this is where we implement searching for
//   potential oneVPL libraries according to the rules
//   in the spec
// for now, we only look in the current working directory
// TODO(JR) - need to add categories 1 and 3
mfxStatus LoaderCtxOneVPL::BuildListOfCandidateLibs() {
    mfxStatus sts = MFX_ERR_NONE;

    // first priority: user-defined directories in
    //   environment variable ONEVPL_SEARCH_PATH
    // TODO(JR) - parse env var and iterate over directories found
    sts = SearchDirForLibs(0, m_libInfoList, LIB_PRIORITY_USE_DEFINED);

    // second priority: oneVPL package (current location for now)
    wcscpy_s(m_vplPackageDir, MFX_MAX_DLL_PATH, L"./");
    sts = SearchDirForLibs(m_vplPackageDir,
                           m_libInfoList,
                           LIB_PRIORITY_ONEVPL_PACKAGE);

    // third priority: standalone MSDK/driver installation
    sts = SearchDirForLibs(0, m_libInfoList, LIB_PRIORITY_MSDK_PACKAGE);

    return sts;
}

// return number of valid libraries found
mfxU32 LoaderCtxOneVPL::CheckValidLibraries() {
    // unique index assigned to each valid library
    mfxU32 libIdx = 0;

    // load all libraries
    std::list<LibInfo*>::iterator it = m_libInfoList.begin();
    while (it != m_libInfoList.end()) {
        LibInfo* libInfo = (*it);

        // load DLL
        libInfo->hModuleVPL = MFX::mfx_dll_load(libInfo->libNameFull);

        // load video functions: pointers to exposed functions
        mfxU32 i;
        for (i = 0; i < eVideoFunc2Total; i += 1) {
            mfxFunctionPointer pProc =
                (mfxFunctionPointer)MFX::mfx_dll_get_addr(
                    libInfo->hModuleVPL,
                    APIVideoFunc2[i].pName);
            if (pProc) {
                libInfo->vplFuncTable[i] = pProc;
            }
            else {
                MFX::mfx_dll_free(libInfo->hModuleVPL);
                break;
            }
        }

        if (i == eVideoFunc2Total) {
            libInfo->libIdx = libIdx++;
            it++;
        }
        else {
            // required function is missing from DLL
            // remove this library from the list of options
            delete libInfo;
            it = m_libInfoList.erase(it);
        }
    }

    // number of valid oneVPL libs
    return (mfxU32)m_libInfoList.size();
}

mfxStatus LoaderCtxOneVPL::UnloadAllLibraries() {
    LibInfo* libInfo;
    mfxU32 i = 0;

    // iterate over all implementation runtimes
    // unload DLL's and free memory
    while (1) {
        libInfo = GetLibInfo(m_libInfoList, i++);

        if (libInfo) {
            MFX::mfx_dll_free(libInfo->hModuleVPL);
            delete libInfo;
        }
        else {
            break;
        }
    }

    return MFX_ERR_NONE;
}

// helper function to get library with given index
LibInfo* LoaderCtxOneVPL::GetLibInfo(std::list<LibInfo*> libInfoList,
                                     mfxU32 idx) {
    std::list<LibInfo*>::iterator it = m_libInfoList.begin();
    while (it != m_libInfoList.end()) {
        LibInfo* libInfo = (*it);

        if (libInfo->libIdx == idx) {
            return libInfo;
        }
        else {
            it++;
        }
    }

    return nullptr; // not found
}

// query implementation i
mfxStatus LoaderCtxOneVPL::QueryImpl(mfxU32 idx,
                                     mfxImplCapsDeliveryFormat format,
                                     mfxHDL* idesc) {
    mfxStatus sts = MFX_ERR_NONE;

    if (format != MFX_IMPLCAPS_IMPLDESCSTRUCTURE)
        return MFX_ERR_UNSUPPORTED;

    // find library with given index
    LibInfo* libInfo = GetLibInfo(m_libInfoList, idx);
    if (libInfo == nullptr)
        return MFX_ERR_NOT_FOUND;

    mfxFunctionPointer pFunc = libInfo->vplFuncTable[eMFXQueryImplDescription];

    // call MFXQueryImplDescription() for this implementation
    // return handle to description in requested format
    libInfo->implDesc =
        (*(mfxHDL(MFX_CDECL*)(mfxImplCapsDeliveryFormat))pFunc)(format);
    if (!libInfo->implDesc)
        return MFX_ERR_UNSUPPORTED;

    // fill out mfxInitParam struct for when we call MFXInitEx
    //   in CreateSession()
    mfxImplDescription* implDesc =
        reinterpret_cast<mfxImplDescription*>(libInfo->implDesc);
    memset(&(libInfo->initPar), 0, sizeof(mfxInitParam));
    libInfo->initPar.Version        = implDesc->ApiVersion;
    libInfo->initPar.Implementation = implDesc->Impl;

    *idesc = libInfo->implDesc;

    return MFX_ERR_NONE;
}

mfxStatus LoaderCtxOneVPL::ReleaseImpl(mfxHDL idesc) {
    mfxStatus sts = MFX_ERR_NONE;

    if (idesc == nullptr)
        return MFX_ERR_NULL_PTR;

    // all we get from the application is a handle to the descriptor,
    //   not the implementation associated with it, so we search
    //   through the full list until we find a match
    // TODO(JR) - should the API be changed to pass index to
    //   MFXDispReleaseImplDescription() also?

    LibInfo* libInfo                 = nullptr;
    std::list<LibInfo*>::iterator it = m_libInfoList.begin();
    while (it != m_libInfoList.end()) {
        libInfo = (*it);

        if (libInfo->implDesc == idesc) {
            break;
        }
        else {
            libInfo = nullptr;
            it++;
        }
    }

    // did not find a matching handle - should not happen
    if (libInfo == nullptr)
        return MFX_ERR_NOT_FOUND;

    mfxFunctionPointer pFunc =
        libInfo->vplFuncTable[eMFXReleaseImplDescription];

    // call MFXReleaseImplDescription() for this implementation
    sts = (*(mfxStatus(MFX_CDECL*)(mfxHDL))pFunc)(libInfo->implDesc);

    libInfo->implDesc = nullptr; // no longer valid

    return sts;
}

mfxStatus LoaderCtxOneVPL::CreateSession(mfxU32 idx, mfxSession* session) {
    mfxStatus sts = MFX_ERR_NONE;

    // find library with given index
    LibInfo* libInfo = GetLibInfo(m_libInfoList, idx);
    if (libInfo == nullptr)
        return MFX_ERR_NOT_FOUND;

    // initialize this library via MFXInitEx
    sts = MFXInitEx2(libInfo->initPar, session, libInfo->libNameFull);

    return sts;
}

// begin exported functions

mfxLoader MFXLoad() {
    LoaderCtxOneVPL* loaderCtx;

    // create unique loader context
    try {
        std::unique_ptr<LoaderCtxOneVPL> pLoaderCtx;
        pLoaderCtx.reset(new LoaderCtxOneVPL{});
        loaderCtx = (LoaderCtxOneVPL*)pLoaderCtx.release();
    }
    catch (...) {
        return nullptr;
    }

    // search directories for candidate oneVPL implementations
    //   based on search order in spec
    mfxStatus sts = loaderCtx->BuildListOfCandidateLibs();
    if (MFX_ERR_NONE != sts) {
        return nullptr;
    }

    // prune libraries which are not oneVPL implementations
    // fill function ptr table for each library which is
    mfxU32 numLibs = loaderCtx->CheckValidLibraries();
    if (numLibs == 0) {
        return nullptr;
    }

    return (mfxLoader)loaderCtx;
}

void MFXUnload(mfxLoader loader) {
    // TODO(JR) - destroy ALL created mfxConfig objects, other memory

    if (loader) {
        LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

        loaderCtx->UnloadAllLibraries();

        delete loaderCtx;
    }

    return;
}

mfxConfig MFXCreateConfig(mfxLoader loader) {
    if (!loader)
        return nullptr;

    // create config context
    // each loader may have more than one config
    LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

    std::unique_ptr<ConfigCtxOneVPL> configCtx;
    try {
        configCtx.reset(new ConfigCtxOneVPL{});
    }
    catch (...) {
        return nullptr;
    }

    loaderCtx->m_configCtx = (ConfigCtxOneVPL*)(configCtx.release());

    return (mfxConfig)(loaderCtx->m_configCtx);
}

mfxStatus MFXSetConfigFilterProperty(mfxConfig config,
                                     const mfxU8* name,
                                     mfxVariant value) {
    if (!config)
        return MFX_ERR_NULL_PTR;

    ConfigCtxOneVPL* configCtx = (ConfigCtxOneVPL*)config;

    return MFX_ERR_NONE;
}

mfxStatus MFXEnumImplementations(mfxLoader loader,
                                 mfxU32 i,
                                 mfxImplCapsDeliveryFormat format,
                                 mfxHDL* idesc) {
    if (!loader)
        return MFX_ERR_NULL_PTR;

    LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

    mfxStatus sts = loaderCtx->QueryImpl(i, format, idesc);

    return sts;
}

mfxStatus MFXCreateSession(mfxLoader loader, mfxU32 i, mfxSession* session) {
    if (!loader)
        return MFX_ERR_NULL_PTR;

    LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

    mfxStatus sts = loaderCtx->CreateSession(i, session);

    return sts;
}

mfxStatus MFXDispReleaseImplDescription(mfxLoader loader, mfxHDL hdl) {
    if (!loader)
        return MFX_ERR_NULL_PTR;

    LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

    mfxStatus sts = loaderCtx->ReleaseImpl(hdl);

    return sts;
}

// passthrough functions to implementation
mfxStatus MFXMemory_GetSurfaceForVPP(mfxSession session,
                                     mfxFrameSurface1** surface) {
    mfxStatus mfxRes         = MFX_ERR_INVALID_HANDLE;
    MFX_DISP_HANDLE* pHandle = (MFX_DISP_HANDLE*)session;

    if (pHandle) {
        mfxFunctionPointer pFunc;
        pFunc = pHandle->callVideoTable2[eMFXMemory_GetSurfaceForVPP];
        if (pFunc) {
            session = pHandle->session;
            mfxRes =
                (*(mfxStatus(MFX_CDECL*)(mfxSession, mfxFrameSurface1**))pFunc)(
                    session,
                    surface);
        }
    }

    return mfxRes;
}

mfxStatus MFXMemory_GetSurfaceForEncode(mfxSession session,
                                        mfxFrameSurface1** surface) {
    mfxStatus mfxRes         = MFX_ERR_INVALID_HANDLE;
    MFX_DISP_HANDLE* pHandle = (MFX_DISP_HANDLE*)session;

    if (pHandle) {
        mfxFunctionPointer pFunc;
        pFunc = pHandle->callVideoTable2[eMFXMemory_GetSurfaceForEncode];
        if (pFunc) {
            session = pHandle->session;
            mfxRes =
                (*(mfxStatus(MFX_CDECL*)(mfxSession, mfxFrameSurface1**))pFunc)(
                    session,
                    surface);
        }
    }

    return mfxRes;
}

mfxStatus MFXMemory_GetSurfaceForDecode(mfxSession session,
                                        mfxFrameSurface1** surface) {
    mfxStatus mfxRes         = MFX_ERR_INVALID_HANDLE;
    MFX_DISP_HANDLE* pHandle = (MFX_DISP_HANDLE*)session;

    if (pHandle) {
        mfxFunctionPointer pFunc;
        pFunc = pHandle->callVideoTable2[eMFXMemory_GetSurfaceForDecode];
        if (pFunc) {
            session = pHandle->session;
            mfxRes =
                (*(mfxStatus(MFX_CDECL*)(mfxSession, mfxFrameSurface1**))pFunc)(
                    session,
                    surface);
        }
    }

    return mfxRes;
}
