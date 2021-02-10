/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019-2021
 */


#include <new>
#include <utility>
#include <deque>
#include <set>

#include "../indicators.h"
#include "../utils/localbuffer.h"
#include "../utils/log.h"

const TI_REAL PI = acos(-1);

int ti_rmf_start(TI_REAL const *options) {
    TI_REAL critical_period = options[0];
    TI_REAL median_period = options[1];

    return median_period-1;
}

int ti_rmf(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    TI_REAL critical_period = options[0];
    TI_REAL median_period = options[1];
    TI_REAL *rmf = outputs[0];

    if (median_period < 1) { return TI_INVALID_OPTION; }
    if (critical_period < 1) { return TI_INVALID_OPTION; }
    if ((int)median_period % 2 != 1) { return TI_INVALID_OPTION; }

    TI_REAL rmf_val;
    std::multiset<TI_REAL> rankedprice;
    std::deque<TI_REAL> price;

    TI_REAL alpha = (cos(2*PI/critical_period) + sin(2*PI/critical_period) - 1.) / cos(2*PI/critical_period);
    int progress = -ti_rmf_start(options);

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress) {
        price.push_back(series[i]);
        rankedprice.insert(price.back());
    }
    for (; i < size && progress == 0; ++i, ++progress) {
        price.push_back(series[i]);
        rankedprice.insert(price.back());
        rmf_val = *std::next(rankedprice.begin(), median_period / 2);
        *rmf++ = rmf_val;

        rankedprice.erase(rankedprice.find(price.front()));
        price.pop_front();
    }
    for (; i < size; ++i, ++progress) {
        price.push_back(series[i]);
        rankedprice.insert(price.back());
        rmf_val = alpha * *std::next(rankedprice.begin(), median_period / 2) + (1. - alpha) * rmf_val;
        *rmf++ = rmf_val;

        rankedprice.erase(rankedprice.find(price.front()));
        price.pop_front();
    }

    return TI_OKAY;
}

struct ti_rmf_stream : ti_stream {

    struct {
        TI_REAL median_period;
        TI_REAL critical_period;
    } options;

    struct {
        TI_REAL rmf_val;
        std::multiset<TI_REAL> rankedprice;
        std::deque<TI_REAL> price;
    } state;

    struct {
        TI_REAL alpha;
    } constants;
};

int ti_rmf_stream_new(TI_REAL const *options, ti_stream **stream) {
    TI_REAL critical_period = options[0];
    TI_REAL median_period = options[1];

    if (median_period < 1) { return TI_INVALID_OPTION; }
    if (critical_period < 1) { return TI_INVALID_OPTION; }
    if ((int)median_period % 2 != 1) { return TI_INVALID_OPTION; }

    ti_rmf_stream *ptr = new(std::nothrow) ti_rmf_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_RMF_INDEX;
    ptr->progress = -ti_rmf_start(options);

    ptr->options.median_period = median_period;
    ptr->options.critical_period = critical_period;

    ptr->constants.alpha = (cos(2*PI/critical_period) + sin(2*PI/critical_period) - 1.) / cos(2*PI/critical_period);

    return TI_OKAY;
}

void ti_rmf_stream_free(ti_stream *stream) {
    delete static_cast<ti_rmf_stream*>(stream);
}

int ti_rmf_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_rmf_stream *ptr = static_cast<ti_rmf_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *rmf = outputs[0];
    int progress = ptr->progress;
    TI_REAL median_period = ptr->options.median_period;
    TI_REAL critical_period = ptr->options.critical_period;
    TI_REAL alpha = ptr->constants.alpha;
    TI_REAL rmf_val = ptr->state.rmf_val;
    auto &price = ptr->state.price;
    auto &rankedprice = ptr->state.rankedprice;

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress) {
        price.push_back(series[i]);
        rankedprice.insert(price.back());
    }
    for (; i < size && progress == 0; ++i, ++progress) {
        price.push_back(series[i]);
        rankedprice.insert(price.back());
        rmf_val = *std::next(rankedprice.begin(), median_period / 2);
        *rmf++ = rmf_val;

        rankedprice.erase(rankedprice.find(price.front()));
        price.pop_front();
    }
    for (; i < size; ++i, ++progress) {
        price.push_back(series[i]);
        rankedprice.insert(price.back());
        rmf_val = alpha * *std::next(rankedprice.begin(), median_period / 2) + (1. - alpha) * rmf_val;
        *rmf++ = rmf_val;

        rankedprice.erase(rankedprice.find(price.front()));
        price.pop_front();
    }

    ptr->progress = progress;
    ptr->state.rmf_val = rmf_val;

    return TI_OKAY;
}
