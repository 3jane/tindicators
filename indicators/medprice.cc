/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"


int ti_medprice_start(TI_REAL const *options) {
    (void)options;
    return 0;
}


int ti_medprice(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];

    (void)options;

    TI_REAL *output = outputs[0];

    int i;
    for (i = 0; i < size; ++i) {
        output[i] = (high[i] + low[i]) * 0.5;
    }


    return TI_OKAY;
}
