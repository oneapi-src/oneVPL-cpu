/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_DISPATCHER_COMMON_MFX_DISPATCHER_ONEVPL_H_
#define SRC_DISPATCHER_COMMON_MFX_DISPATCHER_ONEVPL_H_

#include <list>
#include <memory>
#include <sstream>
#include <string>

#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>

    #include "windows/mfx_dispatcher.h"
    #include "windows/mfx_load_dll.h"

    // use wide char on Windows
    #define MAKE_STRING(x) L##x
typedef std::wstring STRING_TYPE;
typedef wchar_t CHAR_TYPE;

#else
    #include <dirent.h>
    #include <dlfcn.h>
    #include <string.h>

    // use standard char on Linux
    #define MAKE_STRING(x) x
typedef std::string STRING_TYPE;
typedef char CHAR_TYPE;
#endif

#define MAX_ONEVPL_SEARCH_PATH 1024

// internal function to load dll by full path, fail if unsuccessful
mfxStatus MFXInitEx2(mfxInitParam par, mfxSession* session, CHAR_TYPE* dllName);

typedef void(MFX_CDECL* oneVPLFunctionPtr)(void);

enum OneVPLFunctionIdx {
    IdxMFXQueryImplDescription = 0,
    IdxMFXReleaseImplDescription,
    IdxMFXMemory_GetSurfaceForVPP,
    IdxMFXMemory_GetSurfaceForEncode,
    IdxMFXMemory_GetSurfaceForDecode,

    NumOneVPLFunctions
};

// both Windows and Linux use char* for function names
struct OneVPLFunctionDesc {
    const char* pName;
    mfxVersion apiVersion;
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

    mfxStatus SetFilterProperty(const mfxU8* name, mfxVariant value) {
        m_propName = std::string((char*)name);

        m_propValue.Type = value.Type;
        m_propValue.Data = value.Data;

        return MFX_ERR_NONE;
    }

private:
    std::string m_propName;
    mfxVariant m_propValue;
};

struct LibInfo {
    // during search store candidate file names
    //   and priority based on rules in spec
    STRING_TYPE libNameFull;
    STRING_TYPE libNameBase;
    mfxU32 libPriority;

    // if valid oneVPL library, store file handle
    //   and table of exported functions
    void* hModuleVPL;
    oneVPLFunctionPtr vplFuncTable[NumOneVPLFunctions]; // NOLINT
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
    mfxStatus SearchDirForLibs(STRING_TYPE searchDir,
                               std::list<LibInfo*>& libInfoList,
                               mfxU32 priority);
    LibInfo* GetLibInfo(std::list<LibInfo*> libInfoList, mfxU32 idx);

    std::list<LibInfo*> m_libInfoList;
    std::list<ConfigCtxOneVPL*> m_configCtxList;

    STRING_TYPE m_vplPackageDir;
};

#endif // SRC_DISPATCHER_COMMON_MFX_DISPATCHER_ONEVPL_H_
