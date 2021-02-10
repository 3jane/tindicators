#include "../indicators.h"


int ti_ema_start(TI_REAL const *options) {
    (void)options;
    return 0;
}


int ti_ema(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_ema_start(options)) return TI_OKAY;

    const TI_REAL per = 2 / ((TI_REAL)period + 1);

    TI_REAL val = input[0];
    *output++ = val;

    int i;
    for (i = 1; i < size; ++i) {
        val = (input[i]-val) * per + val;
        *output++ = val;
    }


    assert(output - outputs[0] == size - ti_ema_start(options));
    return TI_OKAY;
}
