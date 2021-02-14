/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include "../indicators.h"
#include <new>


int ti_kama_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_kama(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_kama_start(options)) return TI_OKAY;

    /* The caller selects the period used in the efficiency ratio.
     * The fast and slow periods are hard set by the algorithm. */
    const TI_REAL short_per = 2 / (2.0 + 1);
    const TI_REAL long_per = 2 / (30.0 + 1);

    TI_REAL sum = 0;

    int i;
    for (i = 1; i < period; ++i) {
        sum += fabs(input[i] - input[i-1]);
    }

    TI_REAL kama = input[period-1];
    *output++ = kama;
    TI_REAL er, sc;

    for (i = period; i < size; ++i) {
        sum += fabs(input[i] - input[i-1]);

        if (i > period) {
            sum -= fabs(input[i-period] - input[i-period-1]);
        }

        if (sum != 0.0) {
            er = fabs(input[i] - input[i-period]) / sum;
        } else {
            er = 1.0;
        }
        sc = pow(er * (short_per - long_per) + long_per, 2);

        kama = kama + sc * (input[i] - kama);
        *output++ = kama;
    }

    assert(output - outputs[0] == size - ti_kama_start(options));
    return TI_OKAY;
}
