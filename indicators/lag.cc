#include "../indicators.h"


int ti_lag_start(TI_REAL const *options) {
    return (int)options[0];
}


int ti_lag(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 0) return TI_INVALID_OPTION;
    if (size <= ti_lag_start(options)) return TI_OKAY;

    int i;
    for (i = period; i < size; ++i) {
        *output++ = input[i-period];
    }

    assert(output - outputs[0] == size - ti_lag_start(options));
    return TI_OKAY;
}
