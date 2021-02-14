/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"
#include "trend.h"


int ti_linreg_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_linreg(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_linreg_start(options)) return TI_OKAY;

    LINEAR_REGRESSION(size, input, period, output, period);

    assert(output - outputs[0] == size - ti_linreg_start(options));
    return TI_OKAY;
}
