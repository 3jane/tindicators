#include "../indicators.h"
#include "../utils/ringbuf.hh"
#include "../utils/log.h"

#include <vector>
#include <set>
#include <algorithm>
#include <new>

int ti_hfema_start(TI_REAL const *options) {
    int ema_period = options[0];
    int k = options[1];
    TI_REAL threshold = options[2];

    return 2*k;
}

int ti_hfema(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *const real = inputs[0];
    int ema_period = options[0];
    int k = options[1];
    TI_REAL threshold = options[2];
    TI_REAL *hfema = outputs[0];

    if (ema_period < 1) { return TI_INVALID_OPTION; }
    if (k < 1) { return TI_INVALID_OPTION; }
    if (threshold < 0) { return TI_INVALID_OPTION; }

    TI_REAL ema;
    std::multiset<TI_REAL> rankedprice;
    ringbuf<0> price(2*k+1);
    std::vector<TI_REAL> a(2*k+1);

    int i = 0;
    for (; i < 1 && i < size; ++i, step(price)) {
        ema = real[i];
        price = ema;
        rankedprice.insert(price);
    }
    for (; i < 2*k && i < size; ++i, step(price)) {
        ema = (real[i] - ema) * 2. / (1. + ema_period) + ema;
        price = ema;
        rankedprice.insert(price);
    }
    for (; i < size; ++i, step(price)) {
        ema = (real[i] - ema) * 2. / (1. + ema_period) + ema;
        price = ema;
        rankedprice.insert(price);

        TI_REAL median_price = *std::next(rankedprice.begin(), k);
        
        int j = 0;
        for (auto it = rankedprice.begin(); it != rankedprice.end(); ++it, ++j) {
            a[j] = fabs(*it - median_price);
        }
        std::nth_element(a.begin(), a.begin()+k, a.end());
        TI_REAL median_deviation = a[k];
        TI_REAL candidate = price[k];
        *hfema++ = fabs(candidate - median_price) <= threshold * 1.4826 * median_deviation ? candidate : median_price;

        rankedprice.erase(price[2*k]);
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int lessthan(void *one, void *two) {
    return *(TI_REAL*)one < *(TI_REAL*)two;
}

int ti_hfema_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    int ema_period = options[0];
    int k = options[1];
    TI_REAL threshold = options[2];
    TI_REAL *hfema = outputs[0];

    if (ema_period < 1) { return TI_INVALID_OPTION; }
    if (k < 1) { return TI_INVALID_OPTION; }
    if (threshold < 0) { return TI_INVALID_OPTION; }

    TI_REAL *ema = new TI_REAL[size];
    ti_ema(size, &real, options, &ema);

    TI_REAL *data = new TI_REAL[2*k+1];

    for (int i = 2*k; i < size; ++i) {
        for (int j = 0; j < 2*k+1; ++j) {
            data[j] = ema[i-2*(int)k+j];
        }
        std::sort(data, data+2*k+1);
        TI_REAL median_price = data[(int)k];

        for (int j = 0; j < 2*k+1; ++j) {
            data[j] = fabs(data[j] - median_price);
        }
        std::sort(data, data+2*k+1);
        TI_REAL median_deviation = data[(int)k];

        TI_REAL candidate = ema[i-(int)k];
        *hfema++ = fabs(candidate - median_price) <= threshold * 1.4826 * median_deviation ? candidate : median_price;
    }

    delete[] data;
    delete[] ema;

    return TI_OKAY;
}

struct ti_stream {
    int index;
    int progress;

    struct {
        int ema_period;
        int k;
        TI_REAL threshold;
    } options;

    struct {
        TI_REAL ema;
        ringbuf<0> price;
        std::multiset<TI_REAL> rankedprice;
        std::vector<TI_REAL> a;
    } state;

    struct {

    } constants;
};

int ti_hfema_stream_new(TI_REAL const *options, ti_stream **stream) {
    int ema_period = options[0];
    int k = options[1];
    TI_REAL threshold = options[2];

    if (ema_period < 1) { return TI_INVALID_OPTION; }
    if (k < 1) { return TI_INVALID_OPTION; }
    if (threshold < 0) { return TI_INVALID_OPTION; }

    *stream = new(std::nothrow) ti_stream();
    if (!*stream) { return TI_OUT_OF_MEMORY; }

    (*stream)->index = TI_INDICATOR_HFEMA_INDEX;
    (*stream)->progress = -ti_hfema_start(options);

    (*stream)->options.ema_period = ema_period;
    (*stream)->options.k = k;
    (*stream)->options.threshold = threshold;

    try {
        (*stream)->state.a.resize(2*k+1);
        (*stream)->state.price.resize(2*k+1);
    } catch (std::bad_alloc& e) {
        delete *stream;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_hfema_stream_free(ti_stream *stream) {
    delete stream;
}

int ti_hfema_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    TI_REAL *hfema = outputs[0];
    int progress = stream->progress;
    int ema_period = stream->options.ema_period;
    int k = stream->options.k;
    TI_REAL threshold = stream->options.threshold;

    TI_REAL ema = stream->state.ema;
    auto &price = stream->state.price;
    auto &rankedprice = stream->state.rankedprice;
    auto &a = stream->state.a;

    int i = 0;
    for (; progress < -2*k+1 && i < size; ++i, ++progress, step(price)) {
        ema = real[i];
        price = ema;
        rankedprice.insert(ema);
    }
    for (; progress < 0 && i < size; ++i, ++progress, step(price)) {
        ema = (real[i] - ema) * 2. / (ema_period + 1.) + ema;
        price = ema;
        rankedprice.insert(ema);
    }
    for (; i < size; ++i, ++progress, step(price)) {
        ema = (real[i] - ema) * 2. / (ema_period + 1.) + ema;
        price = ema;
        rankedprice.insert(ema);
        TI_REAL median_price = *std::next(rankedprice.begin(), k);
        int j = 0;
        for (auto it = rankedprice.begin(); it != rankedprice.end(); ++it, ++j) {
            a[j] = fabs(*it - median_price);
        }
        std::nth_element(a.begin(), a.begin()+k, a.end());
        TI_REAL median_deviation = a[k];
        TI_REAL candidate = price[k];
        *hfema++ = fabs(candidate - median_price) <= threshold * 1.4826 * median_deviation ? candidate : median_price;

        rankedprice.erase(price[2*k]);
    }

    stream->progress = progress;
    stream->state.ema = ema;

    return TI_OKAY;
}