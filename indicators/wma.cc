/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"


int ti_wma_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_wma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_wma_start(options)) return TI_OKAY;

    /* Weights for 6 period WMA:
     * 1 2 3 4 5 6
     */

    const TI_REAL weights = period * (period+1) / 2;

    TI_REAL sum = 0; /* Flat sum of previous numbers. */
    TI_REAL weight_sum = 0; /* Weighted sum of previous numbers. */

    int i;
    for (i = 0; i < period-1; ++i) {
        weight_sum += input[i] * (i+1);
        sum += input[i];
    }

    for (i = period-1; i < size; ++i) {
        weight_sum += input[i] * period;
        sum += input[i];

        *output++ = weight_sum / weights;

        weight_sum -= sum;
        sum -= input[i-period+1];
    }


    assert(output - outputs[0] == size - ti_wma_start(options));
    return TI_OKAY;
}
