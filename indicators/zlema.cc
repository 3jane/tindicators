/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"


int ti_zlema_start(TI_REAL const *options) {
    return ((int)options[0] - 1) / 2 - 1;
}


int ti_zlema(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];

    const int period = (int)options[0];
    const int lag = (period - 1) / 2;

    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_zlema_start(options)) return TI_OKAY;

    const TI_REAL per = 2 / ((TI_REAL)period + 1);

    TI_REAL val = input[lag-1];
    *output++ = val;

    int i;
    for (i = lag; i < size; ++i) {
        TI_REAL c = input[i];
        TI_REAL l = input[i-lag];

        val = ((c + (c-l))-val) * per + val;
        *output++ = val;
    }


    assert(output - outputs[0] == size - ti_zlema_start(options));
    return TI_OKAY;
}
