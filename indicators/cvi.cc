/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include "../indicators.h"
#include "../utils/buffer.h"


int ti_cvi_start(TI_REAL const *options) {
    const int n = (int)options[0];
    return n*2-1;
}


int ti_cvi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];

    const int period = (int)options[0];

    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_cvi_start(options)) return TI_OKAY;

    const TI_REAL per = 2 / ((TI_REAL)period + 1);

    ti_buffer *lag = ti_buffer_new(period);

    TI_REAL val = high[0]-low[0];

    int i;
    for (i = 1; i < period*2-1; ++i) {
        val = ((high[i]-low[i])-val) * per + val;
        ti_buffer_qpush(lag, val);
    }

    for (i = period*2-1; i < size; ++i) {
        val = ((high[i]-low[i])-val) * per + val;
        const TI_REAL old = lag->vals[lag->index];
        *output++ = val - old ? 100.0 * (val - old) / old : 0;
        ti_buffer_qpush(lag, val);
    }

    ti_buffer_free(lag);

    assert(output - outputs[0] == size - ti_cvi_start(options));
    return TI_OKAY;
}
