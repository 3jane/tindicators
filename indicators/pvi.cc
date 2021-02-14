/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include "../indicators.h"
#include "../utils/log.h"


int ti_pvi_start(TI_REAL const *options) {
    (void)options;
    return 0;
}


int ti_pvi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *close = inputs[0];
    const TI_REAL *volume = inputs[1];

    (void)options;

    TI_REAL *output = outputs[0];

    if (size <= ti_pvi_start(options)) return TI_OKAY;

    TI_REAL pvi = 1000;
    *output++ = pvi;

    int i;
    for (i = 1; i < size; ++i) {

        if (volume[i] > volume[i-1] && volume[i-1] > 0) {
            pvi += ((close[i] - close[i-1])/close[i-1]) * pvi;
        }
        *output++ = pvi;
    }

    assert(output - outputs[0] == size - ti_pvi_start(options));
    return TI_OKAY;
}
