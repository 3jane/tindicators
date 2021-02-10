#include "../indicators.h"


int ti_nvi_start(TI_REAL const *options) {
    (void)options;
    return 0;
}


int ti_nvi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *close = inputs[0];
    const TI_REAL *volume = inputs[1];

    (void)options;

    TI_REAL *output = outputs[0];

    if (size <= ti_nvi_start(options)) return TI_OKAY;

    TI_REAL nvi = 1000;
    *output++ = nvi;

    int i;
    for (i = 1; i < size; ++i) {

        if (volume[i] < volume[i-1]) {
            nvi += ((close[i] - close[i-1])/close[i-1]) * nvi;
        }
        *output++ = nvi;
    }

    assert(output - outputs[0] == size - ti_nvi_start(options));
    return TI_OKAY;
}
