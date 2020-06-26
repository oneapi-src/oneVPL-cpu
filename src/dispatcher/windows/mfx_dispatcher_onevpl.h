/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_DISPATCHER_WINDOWS_MFX_DISPATCHER_ONEVPL_H_
#define SRC_DISPATCHER_WINDOWS_MFX_DISPATCHER_ONEVPL_H_

#include <wchar.h>
#include <windows.h>

#include <list>
#include <memory>

#include "vpl/mfxvideo.h"

#include "vpl/mfxdispatcher.h"

#include "windows/mfx_dispatcher.h"
#include "windows/mfx_dispatcher_log.h"
#include "windows/mfx_load_dll.h"

static const char* ExtVPLFunctions[] = {
    "MFXQueryImplDescription",       "MFXReleaseImplDescription",
    "MFXMemory_GetSurfaceForVPP",    "MFXMemory_GetSurfaceForEncode",
    "MFXMemory_GetSurfaceForDecode",
};

mfxStatus MFXInitEx2(mfxInitParam par, mfxSession* session, wchar_t* dllName);

// priority of runtime loading, based on oneAPI-spec
enum LibPriority {
    LIB_PRIORITY_USE_DEFINED    = 1,
    LIB_PRIORITY_ONEVPL_PACKAGE = 2,
    LIB_PRIORITY_MSDK_PACKAGE   = 3,
};

// config class implementation
class ConfigCtxOneVPL {
public:
    ConfigCtxOneVPL();
    ~ConfigCtxOneVPL();

private:
};

ConfigCtxOneVPL::ConfigCtxOneVPL() {
    return;
}

ConfigCtxOneVPL::~ConfigCtxOneVPL() {
    return;
}

struct LibInfo {
    // during search store candidate file names
    //   and priority based on rules in spec
    wchar_t libNameFull[MFX_MAX_DLL_PATH];
    wchar_t* libNameBase;
    mfxU32 libPriority;

    // if valid oneVPL library, store file handle
    //   and table of exported functions
    mfxModuleHandle hModuleVPL;
    mfxFunctionPointer vplFuncTable[eVideoFunc2Total]; // NOLINT
    mfxHDL implDesc;

    // used for session initialization with this implementation
    mfxInitParam initPar;

    // assign unique index after validating library
    mfxU32 libIdx;
};

// loader class implementation
class LoaderCtxOneVPL {
public:
    LoaderCtxOneVPL();
    ~LoaderCtxOneVPL();

    mfxStatus BuildListOfCandidateLibs();
    mfxU32 CheckValidLibraries();

    mfxStatus QueryImpl(mfxU32 idx,
                        mfxImplCapsDeliveryFormat format,
                        mfxHDL* idesc);

    mfxStatus ReleaseImpl(mfxHDL idesc);

    mfxStatus CreateSession(mfxU32 idx, mfxSession* session);

    mfxStatus UnloadAllLibraries();

    // TODO(JR) - should be a list of any number of configs
    ConfigCtxOneVPL* m_configCtx;

private:
    // helper functions
    mfxStatus SearchDirForLibs(wchar_t* searchDir,
                               std::list<LibInfo*>& libInfoList,
                               mfxU32 priority);
    LibInfo* GetLibInfo(std::list<LibInfo*> libInfoList, mfxU32 idx);

    std::list<LibInfo*> m_libInfoList;
    wchar_t m_vplPackageDir[MFX_MAX_DLL_PATH];
};

LoaderCtxOneVPL::LoaderCtxOneVPL()
        : m_configCtx(),
          m_libInfoList(),
          m_vplPackageDir() {
    return;
}

LoaderCtxOneVPL::~LoaderCtxOneVPL() {
    return;
}

#endif // SRC_DISPATCHER_WINDOWS_MFX_DISPATCHER_ONEVPL_H_
