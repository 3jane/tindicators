/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019-2021
 */


#include <new>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_shmma_start(TI_REAL const *options) {
    const TI_REAL period = options[0];

    return period-1;
}

int ti_shmma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int period = options[0];
    TI_REAL *shmma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    TI_REAL T = 0;
    TI_REAL S = 0;

    int i = 0;
    for (; i < period-1 && i < size; ++i) {
        T += series[i];
        S += (-period + 2*i+1.) / 2. * series[i];
    }
    for (; i < size; ++i) {
        T += series[i];
        S += (period-1.) / 2. * series[i];

        *shmma++ = T/period + (6.*S)/((period+1) * period);

        S -= T;
        S += (period + 1.) / 2. * series[i-period+1];
        T -= series[i-period+1];
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_shmma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const TI_REAL period = options[0];
    TI_REAL *shmma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    for (int i = period-1; i < size; ++i) {
        TI_REAL T = 0;
        for (int j = 0; j < period; ++j) {
            T += series[i-j];
        }

        TI_REAL S = 0;
        for (int j = 0; j < period; ++j) {
            S += (period - (j*2+1.)) / 2. * series[i-j];
        }

        *shmma++ = T/period + (6*S)/((period+1) * period);
    }

    return TI_OKAY;
}

struct ti_shmma_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        TI_REAL T = 0;
        TI_REAL S = 0;
        ringbuf<0> price;
    } state;

    struct {

    } constants;
};

int ti_shmma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    ti_shmma_stream *ptr = new(std::nothrow) ti_shmma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_SHMMA_INDEX;
    ptr->progress = -ti_shmma_start(options);

    ptr->options.period = period;

    ptr->state.price.resize(period);

    return TI_OKAY;
}

void ti_shmma_stream_free(ti_stream *stream) {
    delete static_cast<ti_shmma_stream*>(stream);
}

int ti_shmma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_shmma_stream *ptr = static_cast<ti_shmma_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *shmma = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;

    TI_REAL T = ptr->state.T;
    TI_REAL S = ptr->state.S;
    auto &price = ptr->state.price;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(price)) {
        price = series[i];

        T += series[i];
        S += (-period + 2*(progress+period-1)+1.) / 2. * series[i];
    }
    for (; i < size; ++i, ++progress, step(price)) {
        price = series[i];

        T += series[i];
        S += (period-1.) / 2. * series[i];

        *shmma++ = T/period + (6.*S)/((period+1) * period);

        S -= T;
        S += (period + 1.) / 2. * price[period-1];
        T -= price[period-1];
    }

    ptr->progress = progress;
    ptr->state.T = T;
    ptr->state.S = S;

    return TI_OKAY;
}
