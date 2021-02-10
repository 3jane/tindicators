/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019-2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include <new>

#include "../indicators.h"
#include "../utils/ringbuf.hh"


int ti_sma_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_sma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];
    const TI_REAL scale = 1.0 / period;

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_sma_start(options)) return TI_OKAY;

    TI_REAL sum = 0;

    int i;
    for (i = 0; i < period; ++i) {
        sum += input[i];
    }

    *output++ = sum * scale;

    for (i = period; i < size; ++i) {
        sum += input[i];
        sum -= input[i-period];
        *output++ = sum * scale;
    }

    assert(output - outputs[0] == size - ti_sma_start(options));
    return TI_OKAY;
}


struct ti_sma_stream : ti_stream {
    struct {
        int period;
    } options;

    struct {
        TI_REAL sum;
        ringbuf<0> price;
    } state;

    struct {
        TI_REAL per;
    } constants;
};


int ti_sma_stream_new(TI_REAL const *options, ti_stream **stream) {
    int period = options[0];
    if (period < 1) return TI_INVALID_OPTION;

    ti_sma_stream *ptr = new(std::nothrow) ti_sma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_SMA_INDEX;
    ptr->progress = -ti_sma_start(options);

    ptr->options.period = period;

    ptr->constants.per = 1. / period;

    try {
        ptr->state.price.resize(period);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_sma_stream_free(ti_stream *stream) {
    delete static_cast<ti_sma_stream*>(stream);
}

int ti_sma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_sma_stream *ptr = static_cast<ti_sma_stream*>(stream);
    int progress = ptr->progress;

    const TI_REAL *series = inputs[0];
    int period = ptr->options.period;
    TI_REAL *sma = outputs[0];

    const TI_REAL per = ptr->constants.per;

    TI_REAL sum = ptr->state.sum;
    auto &price = ptr->state.price;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(price)) { // warm up
        price = series[i];
        sum += price;
    }
    for (; i < size; ++i, ++progress, step(price)) { // continue in normal mode
        price = series[i];
        sum += price;

        *sma++ = sum * per;

        sum -= price[period-1];
    }

    ptr->progress = progress;
    ptr->state.sum = sum;

    return TI_OKAY;
}
