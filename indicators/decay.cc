#include "../indicators.h"


int ti_decay_start(TI_REAL const *options) {
    (void)options;
    return 0;
}


int ti_decay(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    TI_REAL *output = outputs[0];
    const int period = (int)options[0];
    const TI_REAL scale = 1.0 / period;

    int i = 0;
    for (; i < 1 && i < size; ++i) {
        *output++ = input[0];
    }
    for (; i < size; ++i) {
        TI_REAL d = output[-1] - scale;
        *output++ = input[i] > d ? input[i] : d;
    }

    return TI_OKAY;
}
