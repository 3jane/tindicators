/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include <new>

#include "../indicators.h"
#include "../utils/buffer.h"


int ti_hma_start(TI_REAL const *options) {
    const int period = (int)options[0];
    const int periodsqrt = (int)(sqrt(period));
    return period >= 0 ? period + periodsqrt - 2 : -1;
}


int ti_hma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_hma_start(options)) return TI_OKAY;

    /* HMA(input, N) = WMA((2 * WMA(input, N/2) - WMA(input, N)), sqrt(N)) */
    /* Need to do three WMAs, with periods N, N/2, and sqrt N.*/

    const int period2 = (int)(period / 2);
    const int periodsqrt = (int)(sqrt(period));

    const TI_REAL weights = period * (period+1) / 2;
    const TI_REAL weights2 = period2 * (period2+1) / 2;
    const TI_REAL weightssqrt = periodsqrt * (periodsqrt+1) / 2;

    TI_REAL sum = 0; /* Flat sum of previous numbers. */
    TI_REAL weight_sum = 0; /* Weighted sum of previous numbers. */

    TI_REAL sum2 = 0;
    TI_REAL weight_sum2 = 0;

    TI_REAL sumsqrt = 0;
    TI_REAL weight_sumsqrt = 0;

    /* Setup up the WMA(period) and WMA(period/2) on the input. */
    int i;
    for (i = 0; i < period-1; ++i) {
        weight_sum += input[i] * (i+1);
        sum += input[i];

        if (i >= period - period2) {
            weight_sum2 += input[i] * (i+1-(period-period2));
            sum2 += input[i];
        }
    }

    ti_buffer *buff = ti_buffer_new(periodsqrt);

    for (i = period-1; i < size; ++i) {
        weight_sum += input[i] * period;
        sum += input[i];

        weight_sum2 += input[i] * period2;
        sum2 += input[i];

        const TI_REAL wma = weight_sum / weights;
        const TI_REAL wma2 = weight_sum2 / weights2;
        const TI_REAL diff = 2 * wma2 - wma;

        weight_sumsqrt += diff * periodsqrt;
        sumsqrt += diff;

        ti_buffer_qpush(buff, diff);

        if (i >= (period-1) + (periodsqrt-1)) {
            *output++ = weight_sumsqrt / weightssqrt;

            weight_sumsqrt -= sumsqrt;
            sumsqrt -= ti_buffer_get(buff, 1);
        } else {
            weight_sumsqrt -= sumsqrt;
        }


        weight_sum -= sum;
        sum -= input[i-period+1];

        weight_sum2 -= sum2;
        sum2 -= input[i-period2+1];
    }

    ti_buffer_free(buff);

    assert(output - outputs[0] == size - ti_hma_start(options));
    return TI_OKAY;
}
