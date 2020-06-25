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

// TODO(JR) - automatic index generation (macro or struct)
enum ExtVPLFunctionsIdx {
    eMFXQueryImplDescription       = 0,
    eMFXReleaseImplDescription     = 1,
    eMFXMemory_GetSurfaceForVPP    = 2,
    eMFXMemory_GetSurfaceForEncode = 3,
    eMFXMemory_GetSurfaceForDecode = 4,
};

#define NumExtVPLFunctions (sizeof(ExtVPLFunctions) / sizeof(char*))

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
    mfxFunctionPointer vplFuncTable[NumExtVPLFunctions];
};

// loader class implementation
class LoaderCtxOneVPL {
public:
    LoaderCtxOneVPL();
    ~LoaderCtxOneVPL();

    mfxStatus SearchDirForLibs(wchar_t* searchDir,
                               std::list<LibInfo*>& libInfoList,
                               mfxU32 priority);
    mfxStatus BuildListOfCandidateLibs();
    mfxU32 CheckValidLibraries();

    mfxStatus Free();

    mfxStatus QueryImpl(mfxU32 i,
                        mfxImplCapsDeliveryFormat format,
                        mfxHDL* idesc);
    mfxStatus CreateSession();

    ConfigCtxOneVPL* m_configCtx;
    mfxSession m_session;

private:
    std::list<LibInfo*> m_libInfoList;
    wchar_t m_vplPackageDir[MFX_MAX_DLL_PATH];

    //mfxModuleHandle m_hModuleVPL;
    //mfxFunctionPointer m_vplFuncTable[NumExtVPLFunctions];
};

LoaderCtxOneVPL::LoaderCtxOneVPL() : m_configCtx(), m_session() {
    return;
}

LoaderCtxOneVPL::~LoaderCtxOneVPL() {
    return;
}

#endif // SRC_DISPATCHER_WINDOWS_MFX_DISPATCHER_ONEVPL_H_
