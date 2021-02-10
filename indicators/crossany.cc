/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include "../indicators.h"


int ti_crossany_start(TI_REAL const *options) {
    (void)options;
    return 1;
}


int ti_crossany(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *a = inputs[0];
    const TI_REAL *b = inputs[1];

    (void)options;

    TI_REAL *output = outputs[0];

    int i;
    for (i = 1; i < size; ++i) {
        *output++ = (a[i] > b[i] && a[i-1] <= b[i-1])
                 || (a[i] < b[i] && a[i-1] >= b[i-1]);
    }

    return TI_OKAY;
}
