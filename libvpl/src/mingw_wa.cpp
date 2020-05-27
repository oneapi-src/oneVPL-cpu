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
                                unsigned long dwFlags) {
    return 0;
}
int BCryptGenRandom(void *hAlgorithm,
                    unsigned char *pbBuffer,
                    unsigned long cbBuffer,
                    unsigned long dwFlags) {
    return 0;
}
int BCryptCloseAlgorithmProvider(void *hAlgorithm, unsigned long dwFlags) {
    return 0;
}
}