#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

#include <vector>

int ti_dwma_start(TI_REAL const *options) {
    const TI_REAL period = options[0];

    return 2*(period-1);
}

int ti_dwma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *dwma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    const TI_REAL denom = (1. + period) * period / 2.;

    TI_REAL numer1 = 0;
    TI_REAL sum1 = 0;
    ringbuf<0> filt1(period);

    TI_REAL numer2 = 0;
    TI_REAL sum2 = 0;

    int i = 0;
    for (; i < period-1 && i < size; ++i) {
        numer1 += (i+1)*real[i];
        sum1 += real[i];
    }
    for (; i < 2*(period-1) && i < size; ++i, step(filt1)) {
        numer1 += period*real[i];
        sum1 += real[i];

        filt1 = numer1/denom;

        numer1 -= sum1;
        sum1 -= real[i-period+1];

        numer2 += (i-(period-1)+1)*filt1;
        sum2 += filt1;
    }
    for (; i < size; ++i, step(filt1)) {
        numer1 += period*real[i];
        sum1 += real[i];

        filt1 = numer1/denom;

        numer1 -= sum1;
        sum1 -= real[i-period+1];

        numer2 += period*filt1;
        sum2 += filt1;

        *dwma++ = numer2 / denom;

        numer2 -= sum2;
        sum2 -= filt1[period-1];
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int DONTOPTIMIZE ti_dwma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const TI_REAL period = options[0];
    TI_REAL *dwma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    std::vector<TI_REAL> smooth1(size-period+1);
    TI_REAL *arr[1] = {smooth1.data()};
    ti_wma(size, inputs, options, arr);

    ti_wma(size-period+1, arr, options, outputs);

    return TI_OKAY;
}

struct ti_dwma_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        ringbuf<0> price;

        TI_REAL numer1 = 0;
        TI_REAL sum1 = 0;
        ringbuf<0> filt1;

        TI_REAL numer2 = 0;
        TI_REAL sum2 = 0;
    } state;

    struct {
        TI_REAL denom;
    } constants;
};

int ti_dwma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    ti_dwma_stream *ptr = new(std::nothrow) ti_dwma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_DWMA_INDEX;
    ptr->progress = -ti_dwma_start(options);

    ptr->options.period = period;

    ptr->constants.denom = (1. + period) * period / 2.;

    ptr->state.price.resize(period);
    
    ptr->state.numer1 = 0;
    ptr->state.sum1 = 0;

    try {
        ptr->state.filt1.resize(period);
    } catch (std::bad_alloc& e) {
        return TI_OUT_OF_MEMORY;
    }

    ptr->state.numer2 = 0;
    ptr->state.sum2 = 0;

    return TI_OKAY;
}

void ti_dwma_stream_free(ti_stream *stream) {
    delete static_cast<ti_dwma_stream*>(stream);
}

int ti_dwma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_dwma_stream *ptr = static_cast<ti_dwma_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *dwma = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;

    TI_REAL numer1 = ptr->state.numer1;
    TI_REAL sum1 = ptr->state.sum1;
    TI_REAL numer2 = ptr->state.numer2;
    TI_REAL sum2 = ptr->state.sum2;
    auto &filt1 = ptr->state.filt1;
    auto &price = ptr->state.price;

    const TI_REAL denom = ptr->constants.denom;

    int i = 0;
    for (; progress < -(period-1) && i < size; ++i, ++progress, step(price)) {
        price = real[i];
        numer1 += (progress+2*(period-1)+1)*real[i];
        sum1 += real[i];
    }
    for (; progress < 0 && i < size; ++i, ++progress, step(filt1, price)) {
        price = real[i];

        numer1 += period*real[i];
        sum1 += real[i];

        filt1 = numer1/denom;

        numer1 -= sum1;
        sum1 -= price[period-1];

        numer2 += (progress+(period-1)+1)*filt1;
        sum2 += filt1;
    }
    for (; i < size; ++i, ++progress, step(filt1, price)) {
        price = real[i];

        numer1 += period*real[i];
        sum1 += real[i];

        filt1 = numer1/denom;

        numer1 -= sum1;
        sum1 -= price[period-1];

        numer2 += period*filt1;
        sum2 += filt1;

        *dwma++ = numer2 / denom;

        numer2 -= sum2;
        sum2 -= filt1[period-1];
    }

    ptr->progress = progress;
    ptr->state.numer1 = numer1;
    ptr->state.sum1 = sum1;
    ptr->state.numer2 = numer2;
    ptr->state.sum2 = sum2;

    return TI_OKAY;
}