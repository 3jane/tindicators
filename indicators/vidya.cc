/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include <new>

#include "../indicators.h"


int ti_vidya_start(TI_REAL const *options) {
    return ((int)(options[1])) - 2;
}


int ti_vidya(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];

    const int short_period = (int)options[0];
    const int long_period = (int)options[1];
    const TI_REAL alpha = options[2];

    TI_REAL *output = outputs[0];

    const TI_REAL short_div = 1.0 / short_period;
    const TI_REAL long_div = 1.0 / long_period;

    if (short_period < 1) return TI_INVALID_OPTION;
    if (long_period < short_period) return TI_INVALID_OPTION;
    if (long_period < 2) return TI_INVALID_OPTION;
    if (alpha < 0.0 || alpha > 1.0) return TI_INVALID_OPTION;
    if (size <= ti_vidya_start(options)) return TI_OKAY;

    TI_REAL short_sum = 0;
    TI_REAL short_sum2 = 0;

    TI_REAL long_sum = 0;
    TI_REAL long_sum2 = 0;

    int i;
    for (i = 0; i < long_period; ++i) {
        long_sum += input[i];
        long_sum2 += input[i] * input[i];

        if (i >= long_period - short_period) {
            short_sum += input[i];
            short_sum2 += input[i] * input[i];
        }
    }

    TI_REAL val = input[long_period-2];
    *output++ = val;

    if (long_period - 1 < size) {
        TI_REAL short_stddev = sqrt(short_sum2 * short_div - (short_sum * short_div) * (short_sum * short_div));
        TI_REAL long_stddev = sqrt(long_sum2 * long_div - (long_sum * long_div) * (long_sum * long_div));
        TI_REAL k = short_stddev ? short_stddev / long_stddev : 0;
        if (k != k) k = 0; /* In some conditions it works out that we take the sqrt(-0.0), which gives NaN.
                              That implies that k should be zero. */
        k *= alpha;
        val = (input[long_period-1]-val) * k + val;
        *output++ = val;
    }

    for (i = long_period; i < size; ++i) {
        long_sum += input[i];
        long_sum2 += input[i] * input[i];

        short_sum += input[i];
        short_sum2 += input[i] * input[i];

        long_sum -= input[i-long_period];
        long_sum2 -= input[i-long_period] * input[i-long_period];

        short_sum -= input[i-short_period];
        short_sum2 -= input[i-short_period] * input[i-short_period];

        {
            TI_REAL short_stddev = sqrt(short_sum2 * short_div - (short_sum * short_div) * (short_sum * short_div));
            TI_REAL long_stddev = sqrt(long_sum2 * long_div - (long_sum * long_div) * (long_sum * long_div));
            TI_REAL k = short_stddev ? short_stddev / long_stddev : 0;
            if (k != k) k = 0;
            k *= alpha;
            val = (input[i]-val) * k + val;

            *output++ = val;
        }
    }

    assert(output - outputs[0] == size - ti_vidya_start(options));
    return TI_OKAY;
}
