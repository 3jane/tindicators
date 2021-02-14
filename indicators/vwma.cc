/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"


int ti_vwma_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_vwma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const TI_REAL *volume = inputs[1];

    const int period = (int)options[0];

    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_vwma_start(options)) return TI_OKAY;

    TI_REAL sum = 0;
    TI_REAL vsum = 0;

    int i;
    for (i = 0; i < period; ++i) {
        sum += input[i] * volume[i];
        vsum += volume[i];
    }

    *output++ = sum ? sum / vsum : 0;

    for (i = period; i < size; ++i) {
        sum += input[i] * volume[i];
        sum -= input[i-period] * volume[i-period];
        vsum += volume[i];
        vsum -= volume[i-period];

        *output++ = sum ? sum / vsum : 0;
    }

    assert(output - outputs[0] == size - ti_vwma_start(options));
    return TI_OKAY;
}
