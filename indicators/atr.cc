#include <new>

#include "../indicators.h"
#include "truerange.h"
#include "../utils/minmax.h"



int ti_atr_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_atr(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];

    const int period = (int)options[0];

    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_atr_start(options)) return TI_OKAY;

    const TI_REAL per = 1.0 / ((TI_REAL)period);

    TI_REAL sum = 0;
    TI_REAL truerange;

    sum += high[0] - low[0];
    int i;
    for (i = 1; i < period; ++i) {
        CALC_TRUERANGE();
        sum += truerange;
    }


    TI_REAL val = sum / period;
    *output++ = val;

    for (i = period; i < size; ++i) {
        CALC_TRUERANGE();
        val = (truerange-val) * per + val;
        *output++ = val;
    }


    assert(output - outputs[0] == size - ti_atr_start(options));
    return TI_OKAY;
}


int ti_atr_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {

    //atr = ti_wilders(ti_tr)

    //First calculate true range.
    const int tr_start = ti_tr_start(0);
    const int tr_size = size - tr_start;
    TI_REAL *truerange = (TI_REAL*)malloc((unsigned int)tr_size * sizeof(TI_REAL));
    if (!truerange) {return TI_OUT_OF_MEMORY;}

    TI_REAL *tr_outputs[1] = {truerange};
    const int tr_ret = ti_tr(size, inputs, 0, tr_outputs);
    if (tr_ret != TI_OKAY) {
        free(truerange);
        return tr_ret;
    }


    //Then wilders.
    const TI_REAL *wilders_inputs[1] = {truerange};
    const int wilders_ret = ti_wilders(tr_size, wilders_inputs, options, outputs);

    free(truerange);


    assert(size - ti_atr_start(options) == size - ti_wilders_start(options));

    return wilders_ret;
}


struct ti_atr_stream : ti_stream {
    /* required */

    /* indicator specific */
    int period;
    TI_REAL sum;
    TI_REAL last;
    TI_REAL last_close;
};


int ti_atr_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = (int)options[0];
    if (period < 1) return TI_INVALID_OPTION;

    ti_atr_stream *ptr = new(std::nothrow) ti_atr_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_ATR_INDEX;
    ptr->progress = -ti_atr_start(options);
    ptr->period = period;
    ptr->sum = 0.0;

    return TI_OKAY;
}

int ti_atr_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_atr_stream *ptr = static_cast<ti_atr_stream*>(stream);

    #undef CALC_TRUERANGE
    #define CALC_TRUERANGE(var, h, l, c) do { \
        const TI_REAL ych = fabs((h) - (c)); \
        const TI_REAL ycl = fabs((l) - (c)); \
        TI_REAL v = (h) - (l); \
        if (ych > v) v = ych; \
        if (ycl > v) v = ycl; \
        var = v; \
    } while (0)

    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];

    TI_REAL *output = outputs[0];

    const TI_REAL per = 1.0 / ((TI_REAL)ptr->period);

    const int start = -(ptr->period-1);
    int i = 0; /* place in input */


    if (ptr->progress < 1) {
        if (ptr->progress == start && i < size) {
            /* first bar of input */
            ptr->sum = high[0] - low[0];
            ptr->last_close = close[0];
            ++ptr->progress; ++i;
        }

        /* still calculating first output */
        while (ptr->progress <= 0 && i < size) {
            TI_REAL truerange; CALC_TRUERANGE(truerange, high[i], low[i], ptr->last_close);
            ptr->sum += truerange;
            ptr->last_close = close[i];
            ++ptr->progress; ++i;
        }

        if (ptr->progress == 1) {
            const TI_REAL val = ptr->sum * per;
            ptr->last = val;
            *output++ = val;
        }
    }

    if (ptr->progress >= 1) {
        /* steady state */
        TI_REAL val = ptr->last;
        while (i < size) {
            TI_REAL truerange; CALC_TRUERANGE(truerange, high[i], low[i], ptr->last_close);
            val = (truerange-val) * per + val;
            *output++ = val;
            ptr->last_close = close[i];
            ++ptr->progress; ++i;
        }

        ptr->last = val;
    }

    return TI_OKAY;
}


void ti_atr_stream_free(ti_stream *stream) {
    delete static_cast<ti_atr_stream*>(stream);
}
