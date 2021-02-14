/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include <new>
#include "../indicators.h"


int ti_md_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_md(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    const TI_REAL scale = 1.0 / period;

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_md_start(options)) return TI_OKAY;


    TI_REAL sum = 0;

    int i, j;
    for (i = 0; i < size; ++i) {
        const TI_REAL today = input[i];
        sum += today;
        if (i >= period) sum -= input[i-period];

        const TI_REAL avg = sum * scale;

        if (i >= period - 1) {
            TI_REAL acc = 0;
            for (j = 0; j < period; ++j) {
                acc += fabs(avg - input[i-j]);
            }

            *output++ = acc * scale;
        }
    }

    assert(output - outputs[0] == size - ti_md_start(options));
    return TI_OKAY;

}
