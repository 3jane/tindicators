

#include "../indicators.h"


int ti_var_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_var(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    const TI_REAL scale = 1.0 / period;

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_var_start(options)) return TI_OKAY;

    TI_REAL sum = 0;
    TI_REAL sum2 = 0;

    int i;
    for (i = 0; i < period; ++i) {
        sum += input[i];
        sum2 += input[i] * input[i];
    }

    *output++ = sum2 * scale - (sum * scale) * (sum * scale);

    for (i = period; i < size; ++i) {
        sum += input[i];
        sum2 += input[i] * input[i];

        sum -= input[i-period];
        sum2 -= input[i-period] * input[i-period];

        *output++ = sum2 * scale - (sum * scale) * (sum * scale);
    }

    assert(output - outputs[0] == size - ti_var_start(options));
    return TI_OKAY;
}
