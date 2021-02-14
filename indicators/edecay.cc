/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include "../indicators.h"


int ti_edecay_start(TI_REAL const *options) {
    (void)options;
    return 0;
}


int ti_edecay(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];
    const TI_REAL scale = 1.0 - 1.0 / period;

    int i = 0;
    for (; i < 1 && i < size; ++i) {
        *output++ = input[0];
    }
    for (; i < size; ++i) {
        TI_REAL d = output[-1] * scale;
        *output++ = input[i] > d ? input[i] : d;
    }

    return TI_OKAY;

}
