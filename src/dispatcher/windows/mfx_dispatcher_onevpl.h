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
#include <sstream>
#include <string>

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

    void SetFilterProperty(const mfxU8* name, mfxVariant value) {
        m_propName = std::string((char*)name);

        m_propValue.Type = value.Type;
        m_propValue.Data = value.Data;
    }

private:
    std::string m_propName;
    mfxVariant m_propValue;
};

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

    // manage library implementations
    mfxStatus BuildListOfCandidateLibs();
    mfxU32 CheckValidLibraries();
    mfxStatus UnloadAllLibraries();

    // query capabilities of each implementation
    mfxStatus QueryImpl(mfxU32 idx,
                        mfxImplCapsDeliveryFormat format,
                        mfxHDL* idesc);
    mfxStatus ReleaseImpl(mfxHDL idesc);

    // create mfxSession
    mfxStatus CreateSession(mfxU32 idx, mfxSession* session);

    // manage configuration filters
    ConfigCtxOneVPL* AddConfigFilter();
    mfxStatus FreeConfigFilters();

private:
    // helper functions
    mfxStatus SearchDirForLibs(wchar_t* searchDir,
                               std::list<LibInfo*>& libInfoList,
                               mfxU32 priority);
    LibInfo* GetLibInfo(std::list<LibInfo*> libInfoList, mfxU32 idx);

    std::list<LibInfo*> m_libInfoList;
    std::list<ConfigCtxOneVPL*> m_configCtxList;

    wchar_t m_vplPackageDir[MFX_MAX_DLL_PATH];
};

#endif // SRC_DISPATCHER_WINDOWS_MFX_DISPATCHER_ONEVPL_H_
