#include "../indicators.h"


int ti_emv_start(TI_REAL const *options) {
    (void)options;
    return 1;
}


int ti_emv(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *volume = inputs[2];

    (void)options;

    TI_REAL *output = outputs[0];

    if (size <= ti_emv_start(options)) return TI_OKAY;

    TI_REAL last = (high[0] + low[0]) * 0.5;

    int i;
    for (i = 1; i < size; ++i) {
        TI_REAL hl = (high[i] + low[i]) * 0.5;
        TI_REAL br = volume[i] / 10000.0 / (high[i] - low[i]);

        *output++ = hl - last ? (hl - last) / br : 0;
        last = hl;
    }

    assert(output - outputs[0] == size - ti_emv_start(options));
    return TI_OKAY;
}
