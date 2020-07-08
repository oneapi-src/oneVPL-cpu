/*############################################################################
  # Copyright (C) 2013-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#ifndef DISABLE_NON_VPL_DISPATCHER

    #include <stdio.h>
    #include <memory>
    #include <string>
    #include "mfx_dispatcher_defs.h"
    #include "mfx_vector.h"
    #include "mfx_win_reg_key.h"
    #include "vpl/mfxplugin.h"

struct MFX_DISP_HANDLE;

namespace MFX {

inline bool operator==(const mfxPluginUID &lhs, const mfxPluginUID &rhs) {
    return !memcmp(lhs.Data, rhs.Data, sizeof(mfxPluginUID));
}

inline bool operator!=(const mfxPluginUID &lhs, const mfxPluginUID &rhs) {
    return !(lhs == rhs);
}
    #ifdef _WIN32
        //warning C4351: new behavior: elements of array 'MFX::PluginDescriptionRecord::sName' will be default initialized
        #pragma warning(disable : 4351)
    #endif
class PluginDescriptionRecord : public mfxPluginParam {
public:
    wchar_t sPath[MAX_PLUGIN_PATH];
    char sName[MAX_PLUGIN_NAME];
    //used for FS plugins that has poor description
    bool onlyVersionRegistered;
    bool Default;
    PluginDescriptionRecord()
            : mfxPluginParam(),
              sPath(),
              sName(),
              onlyVersionRegistered(),
              Default() {}
};

typedef MFXVector<PluginDescriptionRecord> MFXPluginStorage;

class MFXPluginStorageBase : public MFXPluginStorage {
protected:
    mfxVersion mCurrentAPIVersion;

protected:
    explicit MFXPluginStorageBase(mfxVersion currentAPIVersion)
            : mCurrentAPIVersion(currentAPIVersion) {}
    void ConvertAPIVersion(mfxU32 APIVersion,
                           PluginDescriptionRecord &descriptionRecord) const {
        descriptionRecord.APIVersion.Minor =
            static_cast<mfxU16>(APIVersion & 0x0ff);
        descriptionRecord.APIVersion.Major =
            static_cast<mfxU16>(APIVersion >> 8);
    }
};

    #if !defined(MEDIASDK_UWP_DISPATCHER)

//populated from registry
class MFXPluginsInHive : public MFXPluginStorageBase {
public:
    MFXPluginsInHive(int mfxStorageID,
                     const wchar_t *msdkLibSubKey,
                     mfxVersion currentAPIVersion);
};

//plugins are loaded from FS close to executable
class MFXPluginsInFS : public MFXPluginStorageBase {
    bool mIsVersionParsed;
    bool mIsAPIVersionParsed;

public:
    explicit MFXPluginsInFS(mfxVersion currentAPIVersion);

private:
    bool ParseFile(FILE *f, PluginDescriptionRecord &des);
    bool ParseKVPair(wchar_t *key,
                     wchar_t *value,
                     PluginDescriptionRecord &des);
};

    #endif //#if !defined(MEDIASDK_UWP_DISPATCHER)

//plugins are loaded from FS close to Runtime library
class MFXDefaultPlugins : public MFXPluginStorageBase {
public:
    MFXDefaultPlugins(mfxVersion currentAPIVersion,
                      MFX_DISP_HANDLE *hdl,
                      int implType);

private:
};

} // namespace MFX

#endif // DISABLE_NON_VPL_DISPATCHER
