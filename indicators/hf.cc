#include <new>
#include <algorithm>
#include <vector>
#include <set>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_hf_start(TI_REAL const *options) {
    const int k = options[0];
    const TI_REAL threshold = options[1];

    return 2*k;
}

int ti_hf(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int k = options[0];
    const TI_REAL threshold = options[1];
    TI_REAL *hf = outputs[0];

    if (k < 1) { return TI_INVALID_OPTION; }
    if (threshold < 0) { return TI_INVALID_OPTION; }

    std::multiset<TI_REAL> rankedprice;
    std::vector<TI_REAL> a(2*k+1);

    int i = 0;
    for (; i < 2*k && i < size; ++i) {
        rankedprice.insert(series[i]);
    }
    for (; i < size; ++i) {
        rankedprice.insert(series[i]);

        TI_REAL median_price = *std::next(rankedprice.begin(), k);

        int j = 0;
        for (TI_REAL price : rankedprice) {
            a[j] = fabs(price - median_price);
            ++j;
        }

        std::nth_element(a.begin(), a.begin()+k, a.end());
        TI_REAL median_deviation = a[k];

        TI_REAL candidate = series[i];
        *hf++ = fabs(candidate - median_price) < threshold * 1.4826 * median_deviation ? candidate : median_price;

        rankedprice.erase(rankedprice.find(series[i-2*k]));
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_hf_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int k = options[0];
    const TI_REAL threshold = options[1];
    TI_REAL *hf = outputs[0];

    if (k < 1) { return TI_INVALID_OPTION; }
    if (threshold < 0) { return TI_INVALID_OPTION; }

    std::multiset<TI_REAL> rankedprice;

    int i = 0;
    for (; i < 2*k && i < size; ++i) {
        rankedprice.insert(series[i]);
    }
    for (; i < size; ++i) {
        rankedprice.insert(series[i]);

        TI_REAL median_price = *std::next(rankedprice.begin(), k);

        std::vector<TI_REAL> a; a.reserve(2*k+1);
        for (TI_REAL price : rankedprice) {
            a.push_back(fabs(price - median_price));
        }

        std::nth_element(a.begin(), a.begin()+k, a.end());
        TI_REAL median_deviation = a[k];

        TI_REAL candidate = series[i];
        *hf++ = fabs(candidate - median_price) < threshold * 1.4826 * median_deviation ? candidate : median_price;

        rankedprice.erase(rankedprice.find(series[i-2*k]));;
    }

    return TI_OKAY;
}

struct ti_hf_stream : ti_stream {

    struct {
        int k;
        TI_REAL threshold;
    } options;

    struct {
        ringbuf<0> price;
        std::multiset<TI_REAL> rankedprice;
        std::vector<TI_REAL> absbuf;
    } state;

    struct {

    } constants;
};

int ti_hf_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int k = options[0];
    const TI_REAL threshold = options[1];

    if (k < 1) { return TI_INVALID_OPTION; }
    if (threshold < 0) { return TI_INVALID_OPTION; }

    ti_hf_stream *ptr = new(std::nothrow) ti_hf_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_HF_INDEX;
    ptr->progress = -ti_hf_start(options);

    ptr->options.k = k;
    ptr->options.threshold = threshold;

    ptr->state.price.resize(2*k+1);
    ptr->state.absbuf.resize(2*k+1);

    return TI_OKAY;
}

void ti_hf_stream_free(ti_stream *stream) {
    delete static_cast<ti_hf_stream*>(stream);
}

int ti_hf_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_hf_stream *ptr = static_cast<ti_hf_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *hf = outputs[0];
    int progress = ptr->progress;
    const int k = ptr->options.k;
    const TI_REAL threshold = ptr->options.threshold;

    auto &price = ptr->state.price;
    auto &rankedprice = ptr->state.rankedprice;
    auto &absbuf = ptr->state.absbuf;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(price)) {
        price = series[i];
        rankedprice.insert(series[i]);
    }
    for (; i < size; ++i, ++progress, step(price)) {
        price = series[i];
        rankedprice.insert(series[i]);

        TI_REAL median_price = *std::next(rankedprice.begin(), k);

        int j = 0;
        for (TI_REAL p : rankedprice) {
            absbuf[j] = fabs(p - median_price);
            ++j;
        }

        std::nth_element(absbuf.begin(), absbuf.begin()+k, absbuf.end());
        TI_REAL median_deviation = absbuf[k];

        TI_REAL candidate = series[i];
        *hf++ = fabs(candidate - median_price) < threshold * 1.4826 * median_deviation ? candidate : median_price;

        rankedprice.erase(rankedprice.find(price[2*k]));;
    }

    ptr->progress = progress;

    return TI_OKAY;
}
