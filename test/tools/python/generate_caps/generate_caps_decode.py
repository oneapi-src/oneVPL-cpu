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

def generate_caps_decode(data, outfile):

    # stable sorting - work backwards to group by
    #   CodecID, then Profile, then MemHandleType

    # sort by MemHandleType (column 3)
    data = sorted(data, key=operator.itemgetter(3))

    # sort by Profile (column 2)
    data = sorted(data, key=operator.itemgetter(2))

    # sort by CodecID (column 0)
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

    idx_codec = 0
    idx_profile = 0
    idx_mem = 0
    idx_fmt = 0

    buf_codec = "const DecCodec decCodec[] = {\n"

    for i in range(1, nRows+1):
        row_prev = data[i-1]
        row_curr = data[i]
        row_next = data[i+1]

        start_codec   = (row_prev[0] != row_curr[0])
        start_profile = (start_codec or (row_prev[2] != row_curr[2]))
        start_mem     = (start_profile or (row_prev[3] != row_curr[3]))

        end_codec     = (row_next[0] != row_curr[0])
        end_profile   = (end_codec or (row_next[2] != row_curr[2]))
        end_mem       = (end_profile or (row_next[3] != row_curr[3]))

        # start new profile list
        if (start_codec):
            buf_profile = "const DecProfile decProfile_c%02d[] = {\n" % ((idx_codec))

        # start new mem list
        if (start_profile):
            buf_mem = "const DecMemDesc decMemDesc_c%02d_p%02d[] = {\n" % ((idx_codec), int(idx_profile))

        # start new color format list
        if (start_mem):
            buf_fmt = "const mfxU32 decColorFmt_c%02d_p%02d_m%02d[] = {\n" % ((idx_codec), int(idx_profile), int(idx_mem))

        # print next color format
        buf_fmt += "    %s,\n" % (row_curr[10])
        idx_fmt += 1

        # print next mem struct
        if (end_mem):
            buf_mem += ("    {\n")
            buf_mem += ("        %s,\n" % (row_curr[3]) )
            buf_mem += ("        { %d, %d, %d },\n" % (int(row_curr[4]), int(row_curr[5]), int(row_curr[6])) )
            buf_mem += ("        { %d, %d, %d },\n" % (int(row_curr[7]), int(row_curr[8]), int(row_curr[9])) )
            buf_mem += ("        {},\n")
            buf_mem += ("        %d,\n" % idx_fmt)
            buf_mem += ("        (mfxU32 *)decColorFmt_c%02d_p%02d_m%02d,\n" % (idx_codec, idx_profile, idx_mem) )
            buf_mem += ("    },\n")
            idx_mem += 1

        # print next profile struct
        if (end_profile):
            buf_profile += ("    {\n")
            buf_profile += ("        %s,\n" % (row_curr[2]) )
            buf_profile += ("        {},\n")
            buf_profile += ("        %d,\n" % idx_mem)
            buf_profile += ("        (DecMemDesc *)decMemDesc_c%02d_p%02d,\n" % (idx_codec, idx_profile) )
            buf_profile += ("    },\n")
            idx_profile += 1

        if (end_codec):
            buf_codec += ("    {\n")
            buf_codec += ("        %s,\n" % (row_curr[0]) )
            buf_codec += ("        {},\n")
            buf_codec += ("        %s,\n" % (row_curr[1]))
            buf_codec += ("        %d,\n" % idx_profile)
            buf_codec += ("        (DecProfile *)decProfile_c%02d,\n" % idx_codec )
            buf_codec += ("    },\n")
            idx_codec += 1

        if (end_mem):
            buf_fmt += "};\n"
            print(buf_fmt, file=outfile)
            idx_fmt = 0

        if (end_profile):
            buf_mem += "};\n"
            print(buf_mem, file=outfile)
            idx_fmt = 0
            idx_mem = 0

        if (end_codec):
            buf_profile += "};\n"
            print(buf_profile, file=outfile)
            idx_fmt = 0
            idx_mem = 0
            idx_profile = 0

    buf_codec += "};\n"
    print(buf_codec, file=outfile)

    buf_desc =  "const mfxDecoderDescription decoderDesc = {\n"
    buf_desc += "    { %d, %d },\n" % (DEF_STRUCT_VERSION_MINOR, DEF_STRUCT_VERSION_MAJOR)
    buf_desc += "    {},\n"
    buf_desc += "    %d,\n" % idx_codec
    buf_desc += "    (DecCodec *)decCodec,\n"
    buf_desc += "};"
    print(buf_desc, file=outfile)

    print("Success. Decode - number of codecs = %d" % idx_codec)

    return 0
