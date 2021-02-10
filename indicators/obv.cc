/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include "../indicators.h"


int ti_obv_start(TI_REAL const *options) {
    (void)options;
    return 0;
}


int ti_obv(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *close = inputs[0];
    const TI_REAL *volume = inputs[1];

    (void)options;

    TI_REAL *output = outputs[0];

    TI_REAL sum = 0;
    TI_REAL prev;

    int i = 0;
    for (; i < 1 && i < size; ++i) {
        prev = close[i];
        *output++ = sum;
    }
    for (; i < size; ++i) {
        if (close[i] > prev) {
            sum += volume[i];
        } else if (close[i] < prev) {
            sum -= volume[i];
        } else {
            /* No change. */
        }

        prev = close[i];
        *output++ = sum;
    }

    return TI_OKAY;
}
