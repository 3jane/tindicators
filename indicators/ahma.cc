#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_ahma_start(TI_REAL const *options) {
    const TI_REAL period = options[0];

    return 0;
}

int ti_ahma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *ahma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    int i = 0;
    for (; i < period && i < size; ++i) {
        ahma[i] = real[i];
    }
    for (; i < size; ++i) {
        ahma[i] = ahma[i-1] + (real[i] - (ahma[i-1] + ahma[i-period]) / 2) / period;
    }

    return TI_OKAY;
}

int DONTOPTIMIZE ti_ahma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const TI_REAL period = options[0];
    TI_REAL *ahma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    int i = 0;
    for (; i < period && i < size; ++i) {
        ahma[i] = real[i];
    }
    for (; i < size; ++i) {
        ahma[i] = ahma[i-1] + (real[i] - (ahma[i-1] + ahma[i-(int)period]) / 2) / period;
    }

    return TI_OKAY;
}

struct ti_ahma_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        ringbuf<0> filt;
    } state;

    struct {

    } constants;
};

int ti_ahma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    ti_ahma_stream *ptr = new(std::nothrow) ti_ahma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_AHMA_INDEX;
    ptr->progress = -ti_ahma_start(options);

    ptr->options.period = period;

    ptr->state.filt.resize(period+1);

    return TI_OKAY;
}

void ti_ahma_stream_free(ti_stream *stream) {
    delete static_cast<ti_ahma_stream*>(stream);
}

int ti_ahma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_ahma_stream *ptr = static_cast<ti_ahma_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *ahma = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;
    auto &filt = ptr->state.filt;

    int i = 0;
    for (; progress < period && i < size; ++i, ++progress, step(filt)) {
        filt = real[i];

        *ahma++ = filt;
    }
    for (; i < size; ++i, ++progress, step(filt)) {
        filt = filt[1] + (real[i] - (filt[1] + filt[period]) / 2) / period;

        *ahma++ = filt;
    }

    ptr->progress = progress;

    return TI_OKAY;
}