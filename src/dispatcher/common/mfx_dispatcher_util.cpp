/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "common/mfx_dispatcher_onevpl.h"

// new functions for API >= 2.0
static const OneVPLFunctionDesc FunctionDesc2[NumOneVPLFunctions] = {
    { "MFXQueryImplDescription", { { 0, 2 } } },
    { "MFXReleaseImplDescription", { { 0, 2 } } },
    { "MFXMemory_GetSurfaceForVPP", { { 0, 2 } } },
    { "MFXMemory_GetSurfaceForEncode", { { 0, 2 } } },
    { "MFXMemory_GetSurfaceForDecode", { { 0, 2 } } },
};

// implementation of config context (mfxConfig)
// each loader instance can have one or more configs
//   associated with it - used for filtering implementations
//   based on what they support (codec types, etc.)
ConfigCtxOneVPL::ConfigCtxOneVPL() : m_propName(), m_propValue() {
    m_propValue.Version = { 0, 1 };
    m_propValue.Type    = MFX_VARIANT_TYPE_UNSET;

    return;
}

ConfigCtxOneVPL::~ConfigCtxOneVPL() {
    return;
}

// implementation of loader context (mfxLoader)
// each loader instance will build a list of
//   valid oneVPL runtimes and allow application
//   to create sessions with them
LoaderCtxOneVPL::LoaderCtxOneVPL()
        : m_libInfoList(),
          m_configCtxList(),
          m_vplPackageDir() {
    return;
}

LoaderCtxOneVPL::~LoaderCtxOneVPL() {
    return;
}

mfxStatus LoaderCtxOneVPL::SearchDirForLibs(STRING_TYPE searchDir,
                                            std::list<LibInfo*>& libInfoList,
                                            mfxU32 priority) {
    // okay to call with empty searchDir
    if (searchDir.empty())
        return MFX_ERR_NONE;

#if defined(_WIN32) || defined(_WIN64)
    HANDLE hTestFile = nullptr;
    WIN32_FIND_DATAW testFileData;
    DWORD err;
    STRING_TYPE testFileName = MAKE_STRING("*.dll");

    // iterate over all candidate files in directory
    hTestFile = FindFirstFileW(testFileName.c_str(), &testFileData);
    if (hTestFile != INVALID_HANDLE_VALUE) {
        do {
            wchar_t libNameFull[MAX_ONEVPL_SEARCH_PATH];
            wchar_t* libNameBase;

            LibInfo* libInfo = new LibInfo;
            if (!libInfo)
                return MFX_ERR_MEMORY_ALLOC;
            memset(libInfo, 0, sizeof(LibInfo));

            err = GetFullPathNameW(testFileData.cFileName,
                                   MAX_ONEVPL_SEARCH_PATH,
                                   libNameFull,
                                   &libNameBase);
            if (!err) {
                // unknown error - skip it and move on to next file
                delete libInfo;
                continue;
            }
            libInfo->libNameFull = libNameFull;
            libInfo->libPriority = priority;

            // add to list
            libInfoList.push_back(libInfo);
        } while (FindNextFileW(hTestFile, &testFileData));

        FindClose(hTestFile);
    }
#else
    DIR* pSearchDir;
    struct dirent* currFile;

    pSearchDir = opendir(searchDir.c_str());
    if (pSearchDir) {
        while (1) {
            currFile = readdir(pSearchDir);
            if (!currFile)
                break;

            // save files with ".so" (including .so.1, etc.)
            if (strstr(currFile->d_name, ".so")) {
                char filePathC[MAX_ONEVPL_SEARCH_PATH];

                LibInfo* libInfo = new LibInfo;
                if (!libInfo)
                    return MFX_ERR_MEMORY_ALLOC;
                memset(libInfo, 0, sizeof(LibInfo));

                // get full path to found library
                snprintf(filePathC,
                         MAX_ONEVPL_SEARCH_PATH,
                         "%s/%s",
                         searchDir.c_str(),
                         currFile->d_name);
                char* fullPath = realpath(filePathC, NULL);
                if (!fullPath) {
                    // unknown error - skip it and move on to next file
                    delete libInfo;
                    continue;
                }

                libInfo->libNameFull = fullPath;
                libInfo->libPriority = priority;
                free(fullPath);

                // add to list
                libInfoList.push_back(libInfo);
            }
        }
    }
#endif

    return MFX_ERR_NONE;
}

// this is where we implement searching for
//   potential oneVPL libraries according to the rules
//   in the spec
// for now, we only look in the current working directory
// TODO(JR) - need to add categories 1 and 3
mfxStatus LoaderCtxOneVPL::BuildListOfCandidateLibs() {
    mfxStatus sts = MFX_ERR_NONE;

    STRING_TYPE emptyPath; // default construction = empty

    // first priority: user-defined directories in
    //   environment variable ONEVPL_SEARCH_PATH
    // TODO(JR) - parse env var and iterate over directories found
    sts = SearchDirForLibs(emptyPath, m_libInfoList, LIB_PRIORITY_USE_DEFINED);

    // second priority: oneVPL package (current location for now)
    m_vplPackageDir = MAKE_STRING("./");

    sts = SearchDirForLibs(m_vplPackageDir,
                           m_libInfoList,
                           LIB_PRIORITY_ONEVPL_PACKAGE);

    // third priority: standalone MSDK/driver installation
    sts = SearchDirForLibs(emptyPath, m_libInfoList, LIB_PRIORITY_MSDK_PACKAGE);

    return sts;
}

// return number of valid libraries found
mfxU32 LoaderCtxOneVPL::CheckValidLibraries() {
    // unique index assigned to each valid library
    mfxU32 libIdx = 0;

    // load all libraries
    std::list<LibInfo*>::iterator it = m_libInfoList.begin();
    while (it != m_libInfoList.end()) {
        mfxU32 i         = 0;
        LibInfo* libInfo = (*it);

#if defined(_WIN32) || defined(_WIN64)
        // load DLL
        libInfo->hModuleVPL = MFX::mfx_dll_load(libInfo->libNameFull.c_str());

        // load video functions: pointers to exposed functions
        if (libInfo->hModuleVPL) {
            for (i = 0; i < NumOneVPLFunctions; i += 1) {
                oneVPLFunctionPtr pProc =
                    (oneVPLFunctionPtr)MFX::mfx_dll_get_addr(
                        libInfo->hModuleVPL,
                        FunctionDesc2[i].pName);
                if (pProc) {
                    libInfo->vplFuncTable[i] = pProc;
                }
                else {
                    MFX::mfx_dll_free(libInfo->hModuleVPL);
                    break;
                }
            }
        }
#else
        // Linux
        libInfo->hModuleVPL =
            dlopen(libInfo->libNameFull.c_str(), RTLD_LOCAL | RTLD_NOW);

        if (libInfo->hModuleVPL) {
            for (i = 0; i < NumOneVPLFunctions; i += 1) {
                oneVPLFunctionPtr pProc =
                    (oneVPLFunctionPtr)dlsym(libInfo->hModuleVPL,
                                             FunctionDesc2[i].pName);

                if (pProc) {
                    libInfo->vplFuncTable[i] = pProc;
                }
                else {
                    dlclose(libInfo->hModuleVPL);
                    break;
                }
            }
        }
#endif
        if (i == NumOneVPLFunctions) {
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

mfxStatus LoaderCtxOneVPL::UnloadAllLibraries() {
    LibInfo* libInfo;
    mfxU32 i = 0;

    // iterate over all implementation runtimes
    // unload DLL's and free memory
    while (1) {
        libInfo = GetLibInfo(m_libInfoList, i++);

        if (libInfo) {
#if defined(_WIN32) || defined(_WIN64)
            MFX::mfx_dll_free(libInfo->hModuleVPL);
#else
            dlclose(libInfo->hModuleVPL);
#endif
            delete libInfo;
        }
        else {
            break;
        }
    }

    return MFX_ERR_NONE;
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

    oneVPLFunctionPtr pFunc = libInfo->vplFuncTable[IdxMFXQueryImplDescription];

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

    LibInfo* libInfo = nullptr;

    // all we get from the application is a handle to the descriptor,
    //   not the implementation associated with it, so we search
    //   through the full list until we find a match
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
        return MFX_ERR_INVALID_HANDLE;

    oneVPLFunctionPtr pFunc =
        libInfo->vplFuncTable[IdxMFXReleaseImplDescription];

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

    // initialize this library via MFXInitEx, or else fail
    //   (specify full path to library)
    sts = MFXInitEx2(libInfo->initPar,
                     session,
                     (CHAR_TYPE*)libInfo->libNameFull.c_str());

    return sts;
}

ConfigCtxOneVPL* LoaderCtxOneVPL::AddConfigFilter() {
    // create new config filter context and add
    //   to list associated with this loader
    std::unique_ptr<ConfigCtxOneVPL> configCtx;
    try {
        configCtx.reset(new ConfigCtxOneVPL{});
    }
    catch (...) {
        return nullptr;
    }

    ConfigCtxOneVPL* config = (ConfigCtxOneVPL*)(configCtx.release());

    m_configCtxList.push_back(config);

    return config;
}

mfxStatus LoaderCtxOneVPL::FreeConfigFilters() {
    std::list<ConfigCtxOneVPL*>::iterator it = m_configCtxList.begin();

    while (it != m_configCtxList.end()) {
        ConfigCtxOneVPL* config = (*it);
        if (config)
            delete config;
        it++;
    }

    return MFX_ERR_NONE;
}
