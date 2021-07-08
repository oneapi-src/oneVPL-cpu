/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef CPU_SRC_CPU_WORKSTREAM_H_
#define CPU_SRC_CPU_WORKSTREAM_H_

#include <map>
#include <memory>
#include "src/cpu_common.h"
#include "src/cpu_decode.h"
#include "src/cpu_decodevpp.h"
#include "src/cpu_encode.h"
#include "src/cpu_frame.h"
#include "src/cpu_frame_pool.h"
#include "src/cpu_vpp.h"

class CpuWorkstream {
public:
    CpuWorkstream();
    ~CpuWorkstream();

    void SetDecoder(CpuDecode *decode) {
        m_decode.reset(decode);
    }
    void SetEncoder(CpuEncode *encode) {
        m_encode.reset(encode);
    }
    void SetVPP(CpuVPP *vpp) {
        m_vpp.reset(vpp);
    }
    void SetDecodeVPP(CpuDecodeVPP *decvpp) {
        m_decvpp.reset(decvpp);
    }

    CpuDecode *GetDecoder() {
        return m_decode.get();
    }
    CpuEncode *GetEncoder() {
        return m_encode.get();
    }
    CpuVPP *GetVPP() {
        return m_vpp.get();
    }
    CpuDecodeVPP *GetDecodeVPP() {
        return m_decvpp.get();
    }

    mfxStatus Sync(mfxSyncPoint &syncp, mfxU32 wait);

    mfxStatus SetFrameAllocator(mfxFrameAllocator *allocator) {
        RET_IF_FALSE(allocator, MFX_ERR_NULL_PTR);
        m_allocator = *allocator;
        return MFX_ERR_NONE;
    }

    mfxFrameAllocator *GetFrameAllocator() {
        return m_allocator.pthis ? &m_allocator : nullptr;
    }

    void SetHandle(mfxHandleType ht, mfxHDL hdl) {
        m_handles[ht] = hdl;
    }

    mfxStatus GetHandle(mfxHandleType ht, mfxHDL *hdl) {
        if (m_handles.find(ht) == m_handles.end()) {
            *hdl = nullptr;
            return MFX_ERR_NOT_FOUND;
        }
        else {
            *hdl = m_handles[ht];
            return MFX_ERR_NONE;
        }
    }

private:
    std::unique_ptr<CpuDecode> m_decode;
    std::unique_ptr<CpuEncode> m_encode;
    std::unique_ptr<CpuVPP> m_vpp;
    std::unique_ptr<CpuDecodeVPP> m_decvpp;

    mfxFrameAllocator m_allocator;
    std::map<mfxHandleType, mfxHDL> m_handles;

    /* copy not allowed */
    CpuWorkstream(const CpuWorkstream &);
    CpuWorkstream &operator=(const CpuWorkstream &);
};

#endif // CPU_SRC_CPU_WORKSTREAM_H_
