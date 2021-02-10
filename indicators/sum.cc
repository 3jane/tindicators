#include "../indicators.h"


int ti_sum_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_sum(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_sum_start(options)) return TI_OKAY;

    TI_REAL sum = 0;

    int i;
    for (i = 0; i < period; ++i) {
        sum += input[i];
    }

    *output++ = sum;

    for (i = period; i < size; ++i) {
        sum += input[i];
        sum -= input[i-period];
        *output++ = sum;
    }

    assert(output - outputs[0] == size - ti_sum_start(options));
    return TI_OKAY;
}
