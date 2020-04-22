############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################

# Keep formatting for maintainability

# yapf: disable

# pylint: disable-all

import csv, sys, os, operator
import generate_caps_decode as dec
import generate_caps_encode as enc
import generate_caps_vpp as vpp

def usage():
    print("\nUsage: gencaps mode capsFileName.csv capsFileName.cpp\n")
    print("  Required:")
    print("       mode                 Type of props - options: dec, enc, vpp")
    print("       capsFileName.csv     Input capabilities file in .csv format")
    print("       capsFileName.cpp     Output capabilities file in .cpp format")
    exit(-1)

def print_cpp_copyright_header(outfile):
    print("/*############################################################################", file=outfile)
    print("# Copyright (C) 2020 Intel Corporation", file=outfile)
    print("#", file=outfile)
    print("# SPDX-License-Identifier: MIT", file=outfile)
    print("############################################################################*/", file=outfile)
    print("", file=outfile)
    print("//NOLINT(build/header_guard)", file=outfile)
    print("", file=outfile)

    return

def open_files(capsFileName_csv, capsFileName_cpp=None):
    # read reference results file
    if os.path.exists(capsFileName_csv) == 0:
        print('Error - no caps file found. Exiting.')
        return -1

    infile = open(capsFileName_csv, mode='r')
    infile = csv.reader(infile, skipinitialspace=True, delimiter=',', lineterminator='\n')

    # open output C++ file
    if (capsFileName_cpp == None):
        print('Warning - no output file specified. Writing to stdout.')
        outfile = sys.stdout
    else:
        try:
            outfile = open(capsFileName_cpp, mode="wt")
        except IOError:
            print('Error - could not open output file. Exiting.')
            return -1

    # print standard header
    print_cpp_copyright_header(outfile)

    # read input file into 2D list
    data = []
    rowIdx = 0
    for row in infile:
        if not (row):
            continue # skip blank lines

        # skip column names in first row
        if rowIdx != 0:
            data.append(row)
        rowIdx += 1

    return [data, outfile];


if __name__ == '__main__':
    if (len(sys.argv) < 2):
        usage()

    capsFileName_csv = sys.argv[2]

    capsFileName_cpp = None
    if (len(sys.argv) == 4):
        capsFileName_cpp = sys.argv[3]

    [data, outfile] = open_files(capsFileName_csv, capsFileName_cpp)

    if (sys.argv[1] == "dec"):
        dec.generate_caps_decode(data, outfile)
    elif (sys.argv[1] == "enc"):
        enc.generate_caps_encode(data, outfile)
    elif (sys.argv[1] == "vpp"):
        vpp.generate_caps_vpp(data, outfile)
    else:
        print("Error - invalid mode %s" % sys.argv[1])
