/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include <new>

#include "../indicators.h"
#include "../utils/buffer.h"


#define TYPPRICE(INDEX) ((high[(INDEX)] + low[(INDEX)] + close[(INDEX)]) * (1.0/3.0))


int ti_cci_start(TI_REAL const *options) {
    const int period = (int)options[0];
    return (period-1) * 2;
}


int ti_cci(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];

    const int period = (int)options[0];

    const TI_REAL scale = 1.0 / period;

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_cci_start(options)) return TI_OKAY;

    TI_REAL *output = outputs[0];

    ti_buffer *sum = ti_buffer_new(period);

    int i, j;
    for (i = 0; i < size; ++i) {
        const TI_REAL today = TYPPRICE(i);
        ti_buffer_push(sum, today);
        const TI_REAL avg = sum->sum * scale;

        if (i >= period * 2 - 2) {
            TI_REAL acc = 0;
            for (j = 0; j < period; ++j) {
                acc += fabs(avg - sum->vals[j]);
            }

            TI_REAL cci = acc * scale;
            cci *= .015;
            if (cci) { cci = (today-avg)/cci; }
            *output++ = cci;
        }
    }

    ti_buffer_free(sum);

    assert(output - outputs[0] == size - ti_cci_start(options));
    return TI_OKAY;
}
