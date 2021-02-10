#include <new>
#include <numeric>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_evwma_start(TI_REAL const *options) {
    const int period = options[0];
    const TI_REAL gamma = options[1];

    return period-1;
}

int ti_evwma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const close = inputs[0];
    TI_REAL const *const volume = inputs[1];
    const int period = options[0];
    const TI_REAL gamma = options[1];
    TI_REAL *evwma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (gamma <= 0) { return TI_INVALID_OPTION; }

    TI_REAL filt;
    TI_REAL N = 0;

    int i = 0;
    for (; i < period-1 && i < size; ++i) {
        N += volume[i];
    }
    for (; i < period && i < size; ++i) {
        N += volume[i];
        filt = close[i];

        *evwma++ = filt;
    }
    for (; i < size; ++i) {
        N += volume[i] - volume[i-period];
        filt = N ? filt * (1 - volume[i] / (gamma * N)) + volume[i] / (gamma * N) * close[i] : 0;

        *evwma++ = filt;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_evwma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const close = inputs[0];
    TI_REAL const *const volume = inputs[1];
    const int period = options[0];
    const TI_REAL gamma = options[1];
    TI_REAL *evwma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (gamma <= 0) { return TI_INVALID_OPTION; }

    TI_REAL filt;

    int i = 0;
    for (i = period - 1; i < period && i < size; ++i) {
        filt = close[i];

        *evwma++ = filt;
    }
    for (; i < size; ++i) {
        const TI_REAL N = std::accumulate(volume + i - period + 1, volume + i + 1, 0);

        filt = filt * (1 - volume[i] / (gamma * N)) + volume[i] / (gamma * N) * close[i];

        *evwma++ = filt;
    }

    return TI_OKAY;
}

struct ti_evwma_stream : ti_stream {

    struct {
        int period;
        TI_REAL gamma;
    } options;

    struct {
        TI_REAL filt;
        TI_REAL N;
        ringbuf<0> volume;
    } state;

    struct {

    } constants;
};

int ti_evwma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];
    const TI_REAL gamma = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (gamma <= 0) { return TI_INVALID_OPTION; }

    ti_evwma_stream *ptr = new(std::nothrow) ti_evwma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_EVWMA_INDEX;
    ptr->progress = -ti_evwma_start(options);

    ptr->options.period = period;
    ptr->options.gamma = gamma;

    try {
        ptr->state.volume.resize(period+1);
    } catch (std::bad_alloc& e) {
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_evwma_stream_free(ti_stream *stream) {
    delete static_cast<ti_evwma_stream*>(stream);
}

int ti_evwma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_evwma_stream *ptr = static_cast<ti_evwma_stream*>(stream);
    TI_REAL const *const close = inputs[0];
    TI_REAL const *const volume = inputs[1];
    TI_REAL *evwma = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;
    const TI_REAL gamma = ptr->options.gamma;
    TI_REAL filt = ptr->state.filt;
    TI_REAL N = ptr->state.N;
    auto &volume_buf = ptr->state.volume;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(volume_buf)) {
        N += volume[i];
        volume_buf = volume[i];
    }
    for (; progress < 1 && i < size; ++i, ++progress, step(volume_buf)) {
        N += volume[i];
        filt = close[i];
        volume_buf = volume[i];

        *evwma++ = filt;
    }
    for (; i < size; ++i, ++progress, step(volume_buf)) {
        volume_buf = volume[i];

        N += volume_buf - volume_buf[period];
        filt = N ? filt * (1 - volume[i] / (gamma * N)) + volume[i] / (gamma * N) * close[i] : 0;

        *evwma++ = filt;
    }

    ptr->progress = progress;
    ptr->state.filt = filt;
    ptr->state.N = N;

    return TI_OKAY;
}
