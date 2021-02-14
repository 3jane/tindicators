/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"


int ti_trima_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_trima(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_trima_start(options)) return TI_OKAY;
    if (period <= 2) return ti_sma(size, inputs, options, outputs);

    /* Weights for 6 period TRIMA:
     * 1 2 3 3 2 1 = 12
     *
     * Weights for 7 period TRIMA:
     * 1 2 3 4 3 2 1 = 16
     */

    TI_REAL weights = 1 / (TI_REAL) ((period%2) ?
        ((period/2+1) * (period/2+1)):
        ((period/2+1) * (period/2)));

    TI_REAL weight_sum = 0; /* Weighted sum of previous numbers, spans one period back. */
    TI_REAL lead_sum = 0; /* Flat sum of most recent numbers. */
    TI_REAL trail_sum = 0; /* Flat sum of oldest numbers. */

    /* example for period of 9 */
    /* weight_sum       1 2 3 4 5 4 3 2 1 */
    /* lead_sum                   1 1 1 1 */
    /* trail_sum        1 1 1 1 1        */

    const int lead_period = period%2 ? period/2 : period/2-1;
    const int trail_period = lead_period + 1;

    int i, w = 1;
    /* Initialize until before the first value. */
    for (i = 0; i < period-1; ++i) {
        weight_sum += input[i] * w;

        if (i+1 > period-lead_period) lead_sum += input[i];
        if (i+1 <= trail_period) trail_sum += input[i];

        if (i+1 < trail_period) ++w;
        if (i+1 >= period-lead_period) --w;
    }


    int lsi = (period-1)-lead_period+1;
    int tsi1 = (period-1)-period+1+trail_period;
    int tsi2 = (period-1)-period+1;

    for (i = period-1; i < size; ++i) {
        weight_sum += input[i];
        *output++ = weight_sum * weights;

        lead_sum += input[i];

        /* 1 2 3 4 5 4 3 2 1 */
        weight_sum += lead_sum;
        /* 1 2 3 4 5 5 4 3 2 */
        weight_sum -= trail_sum;
        /*   1 2 3 4 5 4 3 2 */

        /* weight_sum       1 2 3 4 5 4 3 2 1 */
        /* lead_sum                   1 1 1 1 */
        /* trail_sum        1 1 1 1 1        */
        lead_sum -= input[lsi++];
        trail_sum += input[tsi1++];
        trail_sum -= input[tsi2++];

    }

    assert(output - outputs[0] == size - ti_trima_start(options));
    return TI_OKAY;
}
