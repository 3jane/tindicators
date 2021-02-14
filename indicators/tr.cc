/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include <new>

#include "../indicators.h"
#include "truerange.h"



int ti_tr_start(TI_REAL const *options) {
    (void)options;
    return 0;
}


int ti_tr(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];

    (void)options;

    TI_REAL *output = outputs[0];
    TI_REAL truerange;


    int i = 0;
    for (; i < 1 && i < size; ++i) {
        output[i] = high[i] - low[i];
    }
    for (; i < size; ++i) {
        CALC_TRUERANGE();
        output[i] = truerange;
    }


    return TI_OKAY;
}
