/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef TEST_UNIT_API_TEST_BITSTREAMS_H_
#define TEST_UNIT_API_TEST_BITSTREAMS_H_

class test_bitstream_96x64_8bit_hevc {
public:
    static unsigned int getlen() {
        return len;
    }
    static unsigned char *getdata() {
        return data;
    }

private:
    static unsigned char data[];
    static unsigned int len;
};

class test_bitstream_96x64_10bit_hevc {
public:
    static unsigned int getlen() {
        return len;
    }
    static unsigned char *getdata() {
        return data;
    }

private:
    static unsigned char data[];
    static unsigned int len;
};

#endif //TEST_UNIT_API_TEST_BITSTREAMS_H_