
#define CALC_TRUERANGE() do{\
        const TI_REAL l = low[i];\
        const TI_REAL h = high[i];\
        const TI_REAL c = close[i-1];\
        const TI_REAL ych = fabs(h - c);\
        const TI_REAL ycl = fabs(l - c);\
        TI_REAL v = h - l;\
        if (ych > v) v = ych;\
        if (ycl > v) v = ycl;\
        truerange = v;\
}while(0)

