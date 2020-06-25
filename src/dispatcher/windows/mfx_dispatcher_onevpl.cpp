/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "windows/mfx_dispatcher_onevpl.h"

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
    // load all libraries
    std::list<LibInfo*>::iterator it = m_libInfoList.begin();
    while (it != m_libInfoList.end()) {
        LibInfo* libInfo = (*it);
        printf("LibInfo2 name (pre-sort)  = %ws\n", libInfo->libNameFull);

        // load DLL
        libInfo->hModuleVPL = MFX::mfx_dll_load(libInfo->libNameFull);

        // load video functions: pointers to exposed functions
        mfxU32 i;
        for (i = 0; i < NumExtVPLFunctions; i += 1) {
            mfxFunctionPointer pProc =
                (mfxFunctionPointer)MFX::mfx_dll_get_addr(libInfo->hModuleVPL,
                                                          ExtVPLFunctions[i]);
            if (pProc) {
                libInfo->vplFuncTable[i] = pProc;
            }
            else {
                MFX::mfx_dll_free(libInfo->hModuleVPL);
                break;
            }
        }

        if (i == NumExtVPLFunctions) {
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

mfxStatus LoaderCtxOneVPL::Free() {
    //if (!m_libInfo.hModuleVPL)
    //    return MFX_ERR_NULL_PTR;
    //
    //MFX::mfx_dll_free(m_libInfo.hModuleVPL);

    return MFX_ERR_NONE;
}

mfxStatus LoaderCtxOneVPL::QueryImpl(mfxU32 i,
                                     mfxImplCapsDeliveryFormat format,
                                     mfxHDL* idesc) {
    if (format != MFX_IMPLCAPS_IMPLDESCSTRUCTURE)
        return MFX_ERR_UNSUPPORTED;

    return MFX_ERR_NONE;
}

mfxStatus LoaderCtxOneVPL::CreateSession() {
    mfxStatus sts = MFX_ERR_NONE;

    mfxInitParam initPar   = { 0 };
    initPar.Version.Major  = VPL_MINIMUM_VERSION_MAJOR;
    initPar.Version.Minor  = VPL_MINIMUM_VERSION_MINOR;
    initPar.Implementation = MFX_IMPL_SOFTWARE;

    // load all libraries
    std::list<LibInfo*>::iterator it = m_libInfoList.begin();
    while (it != m_libInfoList.end()) {
        LibInfo* libInfo = (*it);

        printf("MFXInitEx2 %ws\n", libInfo->libNameFull);
        sts = MFXInitEx2(initPar, &(m_session), libInfo->libNameFull);
        printf("Result = %d\n", sts);

        if (sts == MFX_ERR_NONE)
            break;
        it++;
    }

    return MFX_ERR_NONE;
}

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
    // TODO(JR) - destroy ALL created mfxConfig objects

    if (loader) {
        LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

        loaderCtx->Free();

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

    mfxStatus sts = loaderCtx->CreateSession();

    if (sts == MFX_ERR_NONE) {
        *session = loaderCtx->m_session;
    }
    else {
        *session = nullptr;
        return sts;
    }

    return MFX_ERR_NONE;
}

mfxStatus MFXDispReleaseImplDescription(mfxLoader loader, mfxHDL hdl) {
    return MFX_ERR_UNSUPPORTED;
}
