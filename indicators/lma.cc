#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

#include <vector>

int ti_lma_start(TI_REAL const *options) {
    const TI_REAL period = options[0];

    return period-1;
}

int ti_lma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *lma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    TI_REAL wsum_price = 0;
    TI_REAL sum_price = 0;
    TI_REAL wsum_lma = 0;
    TI_REAL sum_lma = 0;

    TI_REAL denom = 0;
    for (int i = 0; i < period; ++i) {
        denom += i+1;
    }

    int i = 0;
    for (; i < period-1 && i < size; ++i) {
        wsum_price += (i+1) * real[i];
        sum_price += real[i];
    }
    for (; i < size; ++i) {
        wsum_price += period * real[i];
        sum_price += real[i];

        *lma++ = 2*wsum_price/denom - sum_price/period;

        wsum_price -= sum_price;
        sum_price -= real[i-period+1];
    }

    return TI_OKAY;
}

int DONTOPTIMIZE ti_lma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const TI_REAL period = options[0];
    TI_REAL *lma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    std::vector<TI_REAL> wma_price(size-period+1);
    std::vector<TI_REAL> sma_price(size-period+1);

    TI_REAL* arr[1];
 
    arr[0] = wma_price.data();
    ti_wma(size, &real, &period, arr);
    arr[0] = sma_price.data();
    ti_sma(size, &real, &period, arr);

    for (int i = 0; i < size-period+1; ++i) {
        lma[i] = 2*wma_price[i] - sma_price[i];
    }

    return TI_OKAY;
}

struct ti_lma_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        TI_REAL wsum_price = 0;
        TI_REAL sum_price = 0;
        ringbuf<0> price;
    } state;

    struct {
        TI_REAL denom_recipr;
        TI_REAL period_recipr;
    } constants;
};

int ti_lma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    ti_lma_stream *ptr = new(std::nothrow) ti_lma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_LMA_INDEX;
    ptr->progress = -ti_lma_start(options);

    ptr->options.period = period;

    try {
        ptr->state.price.resize(period);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    ptr->constants.denom_recipr = 1. / ((1 + period) / 2. * period);
    ptr->constants.period_recipr = 1. / period;

    return TI_OKAY;
}

void ti_lma_stream_free(ti_stream *stream) {
    delete static_cast<ti_lma_stream*>(stream);
}

int ti_lma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_lma_stream *ptr = static_cast<ti_lma_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *lma = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;

    TI_REAL wsum_price = ptr->state.wsum_price;
    TI_REAL sum_price = ptr->state.sum_price;
    auto &price = ptr->state.price;

    const TI_REAL denom_recipr = ptr->constants.denom_recipr;
    const TI_REAL period_recipr = ptr->constants.period_recipr;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(price)) {
        wsum_price += (progress+(period-1)+1) * real[i];
        sum_price += real[i];
        price = real[i];
    }
    for (; i < size; ++i, ++progress, step(price)) {
        wsum_price += period * real[i];
        sum_price += real[i];
        price = real[i];

        *lma++ = 2*wsum_price*denom_recipr - sum_price*period_recipr;

        wsum_price -= sum_price;
        sum_price -= price[period-1];
    }

    ptr->progress = progress;
    ptr->state.wsum_price = wsum_price;
    ptr->state.sum_price = sum_price;

    return TI_OKAY;
}