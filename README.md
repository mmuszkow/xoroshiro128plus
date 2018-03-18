# xoroshiro128plus
Xoroshiro128+ PRNG implementation

https://en.wikipedia.org/wiki/Xoroshiro128%2B

## Code sample

```C
#include "xoroshiro128plus.h"

uint64_t seed = 1; /* random value */

xrshr128p_state_t state;
xrshr128p_init(seed, &state);
uint64_t r = xrshr128p_next(&state);

/* SIMD version, requires x86 AVX2 extensions */
xrshr128p_avx2_state_t avx2_state;
xrshr128p_avx2_init(seed, &avx2_state);
__m256i simd_r = xrshr128p_avx2_next(&avx2_state);
```

## Performance

5 PRNGs were used to generate 128M random input values for the Pearson's chi-squared test. See `test.c` for more details.

Results on 1.6 GHz Intel Core i5:
```
speed=94.93M/s	chi_test=0	rand()
speed=109.60M/s	chi_test=1	xorshift128plus(&key)
speed=213.19M/s	chi_test=1	avx_xorshift128plus(&simd_key)
speed=231.95M/s	chi_test=1	xrshr128p_next(&state)
speed=291.28M/s	chi_test=1	xrshr128p_avx2_next(&simd_state)
```

