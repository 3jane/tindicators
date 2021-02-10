#include "../indicators.h"


int ti_ad_start(TI_REAL const *options) {
    (void)options;
    return 0;
}


int ti_ad(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];
    const TI_REAL *volume = inputs[3];

    (void)options;

    TI_REAL *output = outputs[0];
    TI_REAL sum = 0;
    int i;
    for (i = 0; i < size; ++i) {
        const TI_REAL hl = (high[i] - low[i]);
        if (hl != 0.0) {
            sum += (close[i] - low[i] - high[i] + close[i]) / hl * volume[i];
        }
        output[i] = sum;
    }

    return TI_OKAY;
}
