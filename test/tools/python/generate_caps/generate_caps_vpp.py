############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################

# Keep formatting for maintainability

# yapf: disable

# pylint: disable-all

import os, operator

DEF_STRUCT_VERSION_MINOR = 0
DEF_STRUCT_VERSION_MAJOR = 1

def generate_caps_vpp(data, outfile):

    # stable sorting - work backwards to group by
    #   CodecID, then Profile, then MemHandleType

    # sort by InFormat (column 9)
    data = sorted(data, key=operator.itemgetter(9))

    # sort by MemHandleType (column 2)
    data = sorted(data, key=operator.itemgetter(2))

    # sort by FilterFourCC (column 0)
    data = sorted(data, key=operator.itemgetter(0))

    # add dummy rows at beginning and end
    nRows = len(data)
    nCols = len(data[0])
    data = ([["SOT"] * nCols]) + data + ([["EOT"] * nCols])

    # DBG
    # print("Sorted table")
    # for row in data:
    #     print(row)
    # print("")

    print("#include \"src/libmfxvplsw_caps.h\"\n", file=outfile)

    idx_filter = 0
    idx_mem = 0
    idx_infmt = 0
    idx_outfmt = 0

    buf_filter = "const VPPFilter vppFilter[] = {\n"

    for i in range(1, nRows+1):
        row_prev = data[i-1]
        row_curr = data[i]
        row_next = data[i+1]

        start_filter   = (row_prev[0] != row_curr[0])
        start_mem = (start_filter or (row_prev[2] != row_curr[2]))
        start_infmt     = (start_mem or (row_prev[9] != row_curr[9]))

        end_filter     = (row_next[0] != row_curr[0])
        end_mem   = (end_filter or (row_next[2] != row_curr[2]))
        end_infmt       = (end_mem or (row_next[9] != row_curr[9]))

        # start new memory list
        if (start_filter):
            buf_mem = "const VPPMemDesc vppMemDesc_f%02d[] = {\n" % ((idx_filter))

        # start new in format list
        if (start_mem):
            buf_infmt = "const VPPFormat vppFormatIn_f%02d_m%02d[] = {\n" % ((idx_filter), int(idx_mem))

        # start new output color format list
        if (start_infmt):
            buf_outfmt = "const mfxU32 vppFormatOut_f%02d_m%02d_i%02d[] = {\n" % ((idx_filter), int(idx_mem), int(idx_infmt))

        # print next output color format
        buf_outfmt += "    %s,\n" % (row_curr[10])
        idx_outfmt += 1

        # print next mem struct
        if (end_infmt):
            buf_infmt += ("    {\n")
            buf_infmt += ("        %s,\n" % (row_curr[9]) )
            buf_infmt += ("        {},\n")
            buf_infmt += ("        %d,\n" % idx_outfmt)
            buf_infmt += ("        (mfxU32 *)vppFormatOut_f%02d_m%02d_i%02d,\n" % (idx_filter, idx_mem, idx_infmt) )
            buf_infmt += ("    },\n")
            idx_infmt += 1

        # print next profile struct
        if (end_mem):
            buf_mem += ("    {\n")
            buf_mem += ("        %s,\n" % (row_curr[2]) )
            buf_mem += ("        { %d, %d, %d },\n" % (int(row_curr[3]), int(row_curr[4]), int(row_curr[5])) )
            buf_mem += ("        { %d, %d, %d },\n" % (int(row_curr[6]), int(row_curr[7]), int(row_curr[8])) )
            buf_mem += ("        {},\n")
            buf_mem += ("        %d,\n" % idx_infmt)
            buf_mem += ("        (VPPFormat *)vppFormatIn_f%02d_m%02d,\n" % (idx_filter, idx_mem) )
            buf_mem += ("    },\n")
            idx_mem += 1

        if (end_filter):
            buf_filter += ("    {\n")
            buf_filter += ("        %s,\n" % (row_curr[0]) )
            buf_filter += ("        %s,\n" % (row_curr[1]))
            buf_filter += ("        {},\n")
            buf_filter += ("        %d,\n" % idx_mem)
            buf_filter += ("        (VPPMemDesc *)vppMemDesc_f%02d,\n" % idx_filter )
            buf_filter += ("    },\n")
            idx_filter += 1

        if (end_infmt):
            buf_outfmt += "};\n"
            print(buf_outfmt, file=outfile)
            idx_outfmt = 0

        if (end_mem):
            buf_infmt += "};\n"
            print(buf_infmt, file=outfile)
            idx_outfmt = 0
            idx_infmt = 0

        if (end_filter):
            buf_mem += "};\n"
            print(buf_mem, file=outfile)
            idx_outfmt = 0
            idx_infmt = 0
            idx_mem = 0

    buf_filter += "};\n"
    print(buf_filter, file=outfile)

    buf_desc =  "const mfxVPPDescription vppDesc = {\n"
    buf_desc += "    { %d, %d },\n" % (DEF_STRUCT_VERSION_MINOR, DEF_STRUCT_VERSION_MAJOR)
    buf_desc += "    {},\n"
    buf_desc += "    %d,\n" % idx_filter
    buf_desc += "    (VPPFilter *)vppFilter,\n"
    buf_desc += "};"
    print(buf_desc, file=outfile)

    print("Success. VPP - number of filters = %d" % idx_filter)

    return 0
