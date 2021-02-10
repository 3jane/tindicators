#include "../utils/log.h"


#ifndef POSTPROC
#define POSTPROC(x) (x)
#endif


#ifndef INIT

#define INIT() const TI_REAL p = (1.0 / (period))

#define FINAL(forecast) do { \
        const TI_REAL a = (y - b * x) * p; \
        *output++ = POSTPROC(a + b * (forecast)); \
} while (0)

#endif





#define LINEAR_REGRESSION(size, input, period, output, forecast) \
    do { \
    TI_REAL x = 0; /* Sum of Xs. */ \
    TI_REAL x2 = 0; /* Sum of square of Xs. */ \
 \
    TI_REAL y = 0; /* Flat sum of previous numbers. */ \
    TI_REAL xy = 0; /* Weighted sum of previous numbers. */ \
 \
    INIT(); \
 \
    int i; \
    for (i = 0; i < (period)-1; ++i) { \
        x += i+1; \
        x2 += (i+1)*(i+1); \
        xy += (input)[i] * (i+1); \
        y += (input)[i]; \
    } \
 \
    x += (period); \
    x2 += (period) * (period); \
 \
    const TI_REAL bd = 1.0 / ((period) * x2 - x * x); \
 \
    for (i = (period)-1; i < (size); ++i) { \
        xy += (input)[i] * (period); \
        y += (input)[i]; \
 \
        const TI_REAL b = ((period) * xy - x * y) * bd; \
        FINAL((forecast)); \
 \
        xy -= y; \
        y -= (input)[i-(period)+1]; \
    } \
} while (0)


