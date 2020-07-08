/*############################################################################
  # Copyright (C) 2017-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_DISPATCHER_LINUX_MFXLOADER_H_
#define SRC_DISPATCHER_LINUX_MFXLOADER_H_

#include <limits.h>

#include <list>
#include <sstream>
#include <string>

#include "vpl/mfxdefs.h"
#ifndef DISABLE_NON_VPL_DISPATCHER
    #include "mfxplugin.h"
#endif

#ifndef DISABLE_NON_VPL_DISPATCHER
inline bool operator==(const mfxPluginUID& lhs, const mfxPluginUID& rhs) {
    return !memcmp(lhs.Data, rhs.Data, sizeof(mfxPluginUID));
}

inline bool operator!=(const mfxPluginUID& lhs, const mfxPluginUID& rhs) {
    return !(lhs == rhs);
}
#endif

inline bool operator<(const mfxVersion& lhs, const mfxVersion& rhs) {
    return (lhs.Major < rhs.Major ||
            (lhs.Major == rhs.Major && lhs.Minor < rhs.Minor));
}

inline bool operator<=(const mfxVersion& lhs, const mfxVersion& rhs) {
    return (lhs < rhs || (lhs.Major == rhs.Major && lhs.Minor == rhs.Minor));
}

#ifndef DISABLE_NON_VPL_DISPATCHER
namespace MFX {

class PluginInfo : public mfxPluginParam {
public:
    PluginInfo() : mfxPluginParam(), m_parsed(), m_path(), m_default() {}

    inline bool isValid() {
        return m_parsed;
    }

    inline mfxPluginUID getUID() {
        return PluginUID;
    }

    inline std::string getPath() {
        return std::string(m_path);
    }

    void Load(const char* name, const char* value);
    void Print();

private:
    enum {
        PARSED_TYPE        = 0x1,
        PARSED_CODEC_ID    = 0x2,
        PARSED_UID         = 0x4,
        PARSED_PATH        = 0x8,
        PARSED_DEFAULT     = 0x10,
        PARSED_VERSION     = 0x20,
        PARSED_API_VERSION = 0x40,
        PARSED_NAME        = 0x80,
    };

    mfxU32 m_parsed;

    char m_path[PATH_MAX];
    bool m_default;
};

void parse(const char* file_name, std::list<PluginInfo>& all_records);

} // namespace MFX
#endif

#endif // SRC_DISPATCHER_LINUX_MFXLOADER_H_
