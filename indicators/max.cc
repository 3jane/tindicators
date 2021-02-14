/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019-2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"


int ti_max_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_max(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_max_start(options)) return TI_OKAY;

    int trail = 0, maxi = -1;
    TI_REAL max = input[0];
    int i, j;
    for (i = period-1; i < size; ++i, ++trail) {
        TI_REAL bar = input[i];

        if (maxi < trail) {
            maxi = trail;
            max = input[maxi];
            j = trail;
            while(++j <= i) {
                bar = input[j];
                if (bar >= max) {
                    max = bar;
                    maxi = j;
                }
            }
        } else if (bar >= max) {
            maxi = i;
            max = bar;
        }

        *output++ = max;
    }


    assert(output - outputs[0] == size - ti_max_start(options));
    return TI_OKAY;
}


int ti_max_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_max_start(options)) return TI_OKAY;

    #define MAX(a, b) ((a) > (b) ? (a) : (b))

    for (int i = period-1; i < size; ++i) {
        TI_REAL max = input[i-period+1];
        for (int j = i-period+2; j <= i; ++j) {
            max = MAX(max, input[j]);
        }
        *output++ = max;
    }

    return TI_OKAY;
}
