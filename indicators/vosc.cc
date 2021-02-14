/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include "../indicators.h"


int ti_vosc_start(TI_REAL const *options) {
    return (int)options[1]-1;
}


int ti_vosc(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    TI_REAL *output = outputs[0];

    const int short_period = (int)options[0];
    const int long_period = (int)options[1];

    const TI_REAL short_div = 1.0 / short_period;
    const TI_REAL long_div = 1.0 / long_period;

    if (short_period < 1) return TI_INVALID_OPTION;
    if (long_period < short_period) return TI_INVALID_OPTION;
    if (size <= ti_vosc_start(options)) return TI_OKAY;

    TI_REAL short_sum = 0;
    TI_REAL long_sum = 0;

    int i;
    for (i = 0; i < long_period; ++i) {
        if (i >= (long_period - short_period)) {
            short_sum += input[i];
        }
        long_sum += input[i];
    }

    {
        const TI_REAL savg = short_sum * short_div;
        const TI_REAL lavg = long_sum * long_div;
        *output++ = (savg - lavg) ? 100.0 * (savg - lavg) / lavg : 0;
    }

    for (i = long_period; i < size; ++i) {
        short_sum += input[i];
        short_sum -= input[i-short_period];

        long_sum += input[i];
        long_sum -= input[i-long_period];

        const TI_REAL savg = short_sum * short_div;
        const TI_REAL lavg = long_sum * long_div;
        *output++ = (savg - lavg) ? 100.0 * (savg - lavg) / lavg : 0;
    }

    assert(output - outputs[0] == size - ti_vosc_start(options));
    return TI_OKAY;
}
