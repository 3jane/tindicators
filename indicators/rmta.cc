/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019-2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"

int ti_rmta_start(TI_REAL const *options) {
    return (int)options[0]-1;
}

/* Name: Recursive Moving Trend Average
 * Source: Dennis Meyers. The Japanese Yen, Recursed, 1998. Original description
 * URL: https://meyersanalytics.com/publications2/jyrecursed.pdf
 */

int ti_rmta(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    const TI_REAL beta = options[1];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_rmta_start(options)) return TI_OKAY;

    const TI_REAL alpha = 1. - beta;
    TI_REAL b = (1. - alpha) * input[0] + input[0];
    TI_REAL rmta = (1. - alpha) * input[0] + alpha * (input[0] + b);

    for (int i = 1; i < period-1; ++i) {
        TI_REAL next_b = (1. - alpha) * b + input[i];
        rmta = (1. - alpha) * rmta + alpha * (input[i] + next_b - b);
        b = next_b;
    }

    for (int i = period-1; i < size; ++i) {
        TI_REAL next_b = (1. - alpha) * b + input[i];
        rmta = (1. - alpha) * rmta + alpha * (input[i] + next_b - b);
        b = next_b;
        *output++ = rmta;
    }

    assert(output - outputs[0] == size - ti_rmta_start(options));
    return TI_OKAY;
}
