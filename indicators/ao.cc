#include "../indicators.h"


int ti_ao_start(TI_REAL const *options) {
    (void)options;
    return 33;
}


int ti_ao(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];

    const int period = 34;

    TI_REAL *output = outputs[0];

    if (size <= ti_ao_start(options)) return TI_OKAY;

    TI_REAL sum34 = 0;
    TI_REAL sum5 = 0;
    const TI_REAL per34 = 1.0 / 34.0;
    const TI_REAL per5 = 1.0 / 5.0;

    int i;
    for (i = 0; i < 34; ++i) {
        TI_REAL hl = 0.5 * (high[i] + low[i]);
        sum34 += hl;
        if (i >= 29) sum5 += hl;
    }

    *output++ = (per5 * sum5 - per34 * sum34);

    for (i = period; i < size; ++i) {
        TI_REAL hl = 0.5 * (high[i] + low[i]);
        sum34 += hl;
        sum5 += hl;

        sum34 -= 0.5 * (high[i-34] + low[i-34]);
        sum5  -= 0.5 * (high[i-5] + low[i-5]);

        *output++ = (per5 * sum5 - per34 * sum34);
    }

    assert(output - outputs[0] == size - ti_ao_start(options));
    return TI_OKAY;

}
