/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019-2021
 */


#include <new>
#include <vector>
#include <set>
#include <algorithm>

#include "../indicators.h"
#include "../utils/ringbuf.hh"
#include "../utils/log.h"

int ti_hfema_start(TI_REAL const *options) {
    int ema_period = options[0];
    int k = options[1];
    TI_REAL threshold = options[2];

    return 2*k;
}

int ti_hfema(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *const series = inputs[0];
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
        ema = series[i];
        price = ema;
        rankedprice.insert(price);
    }
    for (; i < 2*k && i < size; ++i, step(price)) {
        ema = (series[i] - ema) * 2. / (1. + ema_period) + ema;
        price = ema;
        rankedprice.insert(price);
    }
    for (; i < size; ++i, step(price)) {
        ema = (series[i] - ema) * 2. / (1. + ema_period) + ema;
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

        rankedprice.erase(rankedprice.find(price[2*k]));
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int lessthan(void *one, void *two) {
    return *(TI_REAL*)one < *(TI_REAL*)two;
}

int ti_hfema_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    int ema_period = options[0];
    int k = options[1];
    TI_REAL threshold = options[2];
    TI_REAL *hfema = outputs[0];

    if (ema_period < 1) { return TI_INVALID_OPTION; }
    if (k < 1) { return TI_INVALID_OPTION; }
    if (threshold < 0) { return TI_INVALID_OPTION; }

    TI_REAL *ema = new TI_REAL[size];
    ti_ema(size, &series, options, &ema);

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

struct ti_hfema_stream : ti_stream {

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

    ti_hfema_stream *ptr = new(std::nothrow) ti_hfema_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_HFEMA_INDEX;
    ptr->progress = -ti_hfema_start(options);

    ptr->options.ema_period = ema_period;
    ptr->options.k = k;
    ptr->options.threshold = threshold;

    try {
        ptr->state.a.resize(2*k+1);
        ptr->state.price.resize(2*k+1);
    } catch (std::bad_alloc& e) {
        delete *stream;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_hfema_stream_free(ti_stream *stream) {
    delete static_cast<ti_hfema_stream*>(stream);
}

int ti_hfema_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_hfema_stream *ptr = static_cast<ti_hfema_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *hfema = outputs[0];
    int progress = ptr->progress;
    int ema_period = ptr->options.ema_period;
    int k = ptr->options.k;
    TI_REAL threshold = ptr->options.threshold;

    TI_REAL ema = ptr->state.ema;
    auto &price = ptr->state.price;
    auto &rankedprice = ptr->state.rankedprice;
    auto &a = ptr->state.a;

    int i = 0;
    for (; progress < -2*k+1 && i < size; ++i, ++progress, step(price)) {
        ema = series[i];
        price = ema;
        rankedprice.insert(ema);
    }
    for (; progress < 0 && i < size; ++i, ++progress, step(price)) {
        ema = (series[i] - ema) * 2. / (ema_period + 1.) + ema;
        price = ema;
        rankedprice.insert(ema);
    }
    for (; i < size; ++i, ++progress, step(price)) {
        ema = (series[i] - ema) * 2. / (ema_period + 1.) + ema;
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

        rankedprice.erase(rankedprice.find(price[2*k]));
    }

    ptr->progress = progress;
    ptr->state.ema = ema;

    return TI_OKAY;
}
