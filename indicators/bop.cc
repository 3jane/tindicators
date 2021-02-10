#include "../indicators.h"


int ti_bop_start(TI_REAL const *options) {
    (void)options;
    return 0;
}


int ti_bop(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *open = inputs[0];
    const TI_REAL *high = inputs[1];
    const TI_REAL *low = inputs[2];
    const TI_REAL *close = inputs[3];

    (void)options;

    TI_REAL *output = outputs[0];
    int i;
    for (i = 0; i < size; ++i) {
        TI_REAL hl = high[i] - low[i];
        if (hl <= 0.0) {
            output[i] = 0;
        } else {
            output[i] = (close[i] - open[i]) / hl;
        }
    }

    return TI_OKAY;
}
