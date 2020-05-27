/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

// temporary workaround for bootstrap link errors on Windows
extern "C" {
int clock_gettime(int clk_id, struct timespec *tp) {
    return 0;
}
int nanosleep(void *req, void *rem) {
    return 0;
}
int BCryptOpenAlgorithmProvider(void *phAlgorithm,
                                void *pszAlgId,
                                void *pszImplementation,
                                unsigned int dwFlags) {
    return 0;
}
int BCryptGenRandom(void *hAlgorithm,
                    unsigned char *pbBuffer,
                    unsigned int cbBuffer,
                    unsigned int dwFlags) {
    return 0;
}
int BCryptCloseAlgorithmProvider(void *hAlgorithm, unsigned int dwFlags) {
    return 0;
}
}