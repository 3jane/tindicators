#include "../indicators.h"
#include "../utils/ringbuf.hh"

#include <vector>
#include <deque>
#include <set>
#include <algorithm>
#include <new>

int ti_hfsma_start(TI_REAL const *options) {
    int sma_period = options[0];
    int k = options[1];
    TI_REAL threshold = options[2];

    return sma_period-1 + 2*k;
}

int ti_hfsma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    const TI_REAL *const real = inputs[0];
    int sma_period = options[0];
    int k = options[1];
    TI_REAL threshold = options[2];
    TI_REAL *hfsma = outputs[0];

    if (sma_period < 1) { return TI_INVALID_OPTION; }
    if (k < 1) { return TI_INVALID_OPTION; }
    if (threshold < 0) { return TI_INVALID_OPTION; }

    TI_REAL sum = 0.;
    std::multiset<TI_REAL> rankedprice;
    ringbuf<0> smoothed_price(2*k+1);
    std::vector<TI_REAL> a(2*k+1);

    int i = 0;
    for (; i < sma_period-1 && i < size; ++i, step(smoothed_price)) {
        sum += real[i];
    }
    for (; i < sma_period-1 + 2*k && i < size; ++i, step(smoothed_price)) {
        sum += real[i];
        rankedprice.insert(sum/sma_period);
        smoothed_price = sum/sma_period;

        sum -= real[i-sma_period+1];
    }
    for (; i < size; ++i, step(smoothed_price)) {
        sum += real[i];
        rankedprice.insert(sum/sma_period);
        smoothed_price = sum/sma_period;

        TI_REAL median_price = *std::next(rankedprice.begin(), k);
        int j = 0;
        for (auto it = rankedprice.begin(); it != rankedprice.end(); ++it, ++j) {
            a[j] = fabs(*it - median_price);
        }
        std::nth_element(a.begin(), a.begin()+k, a.end());
        TI_REAL median_deviation = a[k];
        TI_REAL candidate = smoothed_price[k];
        *hfsma++ = fabs(candidate - median_price) <= threshold * 1.4826 * median_deviation ? candidate : median_price;

        sum -= real[i-sma_period+1];
        rankedprice.erase(rankedprice.find(smoothed_price[2*k]));
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int ti_hfsma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *const real = inputs[0];
    int sma_period = options[0];
    int k = options[1];
    TI_REAL threshold = options[2];
    TI_REAL *hfsma = outputs[0];

    if (sma_period < 1) { return TI_INVALID_OPTION; }
    if (k < 1) { return TI_INVALID_OPTION; }
    if (threshold < 0) { return TI_INVALID_OPTION; }

    TI_REAL *sma = new TI_REAL[size];
    ti_sma(size, &real, options, &sma);

    TI_REAL *data = new TI_REAL[2*(int)k+1];

    for (int i = 2*k; i < size - ti_sma_start(options); ++i) {
        for (int j = 0; j < 2*k+1; ++j) {
            data[j] = sma[i-2*(int)k+j];
        }
        std::sort(data, data + 2*(int)k+1);
        TI_REAL median_price = data[(int)k];

        for (int j = 0; j < 2*k+1; ++j) {
            data[j] = fabs(data[j] - median_price);
        }
        std::sort(data, data + 2*(int)k+1);
        TI_REAL median_deviation = data[(int)k];

        TI_REAL candidate = sma[i-(int)k];
        *hfsma++ = fabs(candidate - median_price) <= threshold * 1.4826 * median_deviation ? candidate : median_price;
    }

    delete[] data;
    delete[] sma;

    return TI_OKAY;
}

struct ti_hfsma_stream : ti_stream {

    struct {
        int sma_period;
        int k;
        TI_REAL threshold;
    } options;

    struct {
        TI_REAL sum;
        std::multiset<TI_REAL> rankedprice;
        ringbuf<0> price;
        ringbuf<0> smoothed_price;
        std::vector<TI_REAL> a;
    } state;

    struct {

    } constants;
};

int ti_hfsma_stream_new(TI_REAL const *options, ti_stream **stream) {
    int sma_period = options[0];
    int k = options[1];
    TI_REAL threshold = options[2];

    if (sma_period < 1) { return TI_INVALID_OPTION; }
    if (k < 1) { return TI_INVALID_OPTION; }
    if (threshold < 0) { return TI_INVALID_OPTION; }

    ti_hfsma_stream *ptr = new(std::nothrow) ti_hfsma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_HFSMA_INDEX;
    ptr->progress = -ti_hfsma_start(options);

    ptr->state.sum = 0.;

    ptr->options.sma_period = sma_period;
    ptr->options.k = k;
    ptr->options.threshold = threshold;

    try {
        ptr->state.a.resize(2*k+1);
        ptr->state.price.resize(2*k+1);
        ptr->state.smoothed_price.resize(2*k+1);
    } catch (std::bad_alloc& e) {
        delete *stream;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_hfsma_stream_free(ti_stream *stream) {
    delete static_cast<ti_hfsma_stream*>(stream);
}

int ti_hfsma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_hfsma_stream *ptr = static_cast<ti_hfsma_stream*>(stream);
    const TI_REAL *const real = inputs[0];
    TI_REAL *hfsma = outputs[0];
    int progress = ptr->progress;
    int sma_period = ptr->options.sma_period;
    int k = ptr->options.k;
    TI_REAL threshold = ptr->options.threshold;

    TI_REAL sum = ptr->state.sum;
    std::multiset<TI_REAL> &rankedprice = ptr->state.rankedprice;
    auto &price = ptr->state.price;
    auto &smoothed_price = ptr->state.smoothed_price;
    auto &a = ptr->state.a;

    int i = 0;

    for (; i < size && progress < -2*k; ++i, ++progress, step(price, smoothed_price)) {
        price = real[i];
        sum += real[i];
    }
    for (; i < size && progress < 0; ++i, ++progress, step(price, smoothed_price)) {
        price = real[i];
        sum += real[i];
        smoothed_price = sum/sma_period;
        rankedprice.insert(sum/sma_period);

        sum -= price[sma_period-1];
    }
    for (; i < size; ++i, ++progress, step(price, smoothed_price)) {
        price = real[i];
        sum += real[i];
        smoothed_price = sum/sma_period;
        rankedprice.insert(sum/sma_period);
        TI_REAL median_price = *std::next(rankedprice.begin(), k);
        int j = 0;
        for (auto it = rankedprice.begin(); it != rankedprice.end(); ++it, ++j) {
            a[j] = fabs(*it - median_price);
        }
        std::nth_element(a.begin(), a.begin()+k, a.end());
        TI_REAL median_deviation = a[k];
        TI_REAL candidate = smoothed_price[k];
        *hfsma++ = fabs(candidate - median_price) <= threshold * 1.4826 * median_deviation ? candidate : median_price;

        sum -= price[sma_period-1];
        rankedprice.erase(rankedprice.find(smoothed_price[2*k]));
    }

    ptr->progress = progress;
    ptr->state.sum = sum;

    return TI_OKAY;
}
