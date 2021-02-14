/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"


int ti_wilders_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_wilders(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_wilders_start(options)) return TI_OKAY;

    const TI_REAL per = 1.0 / ((TI_REAL)period);

    TI_REAL sum = 0;

    int i;
    for (i = 0; i < period; ++i) {
        sum += input[i];
    }


    TI_REAL val = sum / period;
    *output++ = val;

    for (i = period; i < size; ++i) {
        val = (input[i]-val) * per + val;
        *output++ = val;
    }


    assert(output - outputs[0] == size - ti_wilders_start(options));
    return TI_OKAY;
}
