/* Crude performance test
 *
 * To include xorshift128+ compile with:
 * gcc test.c -Wall -O3 -march=native -o test -DTEST_XORSHIFT128P -I ../SIMDxorshift/include ../SIMDxorshift/src/\*.c
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "xoroshiro128plus.h"

#ifdef TEST_XORSHIFT128P
/* from https://github.com/lemire/SIMDxorshift */
# include "xorshift128plus.h"
# include "simdxorshift128plus.h"
#endif

/* Frequency table size. */
#define CHI_MAX     128000
/* Expected average number of occurences. */
#define CHI_AVG     1000.0
/* Number of rand calls, CHI_AVG = CHI_ITERS / CHI_MAX. */
#define CHI_ITERS   (CHI_MAX * CHI_AVG)

#define POW2(x) ((x)*(x))

int chi_freq[CHI_MAX];

/* https://en.wikipedia.org/wiki/Pearson%27s_chi-squared_test
 * https://en.wikibooks.org/wiki/Algorithm_Implementation/Pseudorandom_Numbers/Chi-Square_Test
 */
int chi_test(void) {
    int i;
    double chi_sq = 0;
    for(i=0; i<CHI_MAX; i++) chi_sq += POW2(chi_freq[i] - CHI_AVG);
    chi_sq /= CHI_AVG;
    return fabs(chi_sq - CHI_MAX) < 2 * sqrt((double) CHI_MAX);
}

/* Test defining macros. */
#define elapsed() ((clock()-start)/(float) CLOCKS_PER_SEC)

#define prng_test_start() \
    memset(chi_freq, 0, sizeof(chi_freq)); \
    start = clock();

#define prng_test_end(rand_next) \
    printf("speed=%.2fM/s\t", CHI_ITERS / elapsed() / 1000000.0f); \
    printf("chi_test=%d\t" #rand_next "\n", chi_test()); \

#define prng_test(rand_next) \
    prng_test_start(); \
    for(i=0; i<CHI_ITERS; i++) chi_freq[rand_next % CHI_MAX]++; \
    prng_test_end(rand_next);

int main(void) {
    int i;
    time_t start;
    uint64_t simd_v[4], seed;
    xrshr128p_state_t      state;
    xrshr128p_avx2_state_t simd_state;
#ifdef TEST_XORSHIFT128P
    xorshift128plus_key_t     key; 
    avx_xorshift128plus_key_t simd_key;
#endif
    seed = time(NULL);

    /* init the PRNG states */
    srand((unsigned int) seed);
    xrshr128p_init(seed, &state);
    xrshr128p_avx2_init(seed, &simd_state);
#ifdef TEST_XORSHIFT128P
    xorshift128plus_init(seed, splitmix64(seed), &key);
    avx_xorshift128plus_init(seed, splitmix64(seed), &simd_key);
#endif

    /* default implementation */
    prng_test(rand());

    /* Xorshift128+ */
#ifdef TEST_XORSHIFT128P
    prng_test(xorshift128plus(&key));

    prng_test_start();
    for(i=0; i<CHI_ITERS; i+=4) {
        _mm256_storeu_si256((__m256i *) simd_v, avx_xorshift128plus(&simd_key));
        chi_freq[simd_v[0] % CHI_MAX]++;
        chi_freq[simd_v[1] % CHI_MAX]++;
        chi_freq[simd_v[2] % CHI_MAX]++;
        chi_freq[simd_v[3] % CHI_MAX]++;
    }
    prng_test_end(avx_xorshift128plus(&simd_key));
#endif

    /* Xoroshiro128+ */
    prng_test(xrshr128p_next(&state));
    
    prng_test_start();
    for(i=0; i<CHI_ITERS; i+=4) {
        _mm256_storeu_si256((__m256i *) simd_v, xrshr128p_avx2_next(&simd_state));
        chi_freq[simd_v[0] % CHI_MAX]++;
        chi_freq[simd_v[1] % CHI_MAX]++;
        chi_freq[simd_v[2] % CHI_MAX]++;
        chi_freq[simd_v[3] % CHI_MAX]++;
    }
    prng_test_end(xrshr128p_avx2_next(&simd_state));
    
    return 0;
}

