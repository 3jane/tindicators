/*
 * Tulip Indicators
 * https://tulipindicators.org/
 * Copyright (c) 2010-2019 Tulip Charts LLC
 * Lewis Van Winkle (LV@tulipcharts.org)
 *
 * This file is part of Tulip Indicators.
 *
 * Tulip Indicators is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Tulip Indicators is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Tulip Indicators.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <new>

#include "../indicators.h"
#include "../utils/ringbuf.hh"
#include "../utils/log.h"

int ti_copp_start(TI_REAL const *options) {
    const TI_REAL roc_shorter_period = options[0];
    const TI_REAL roc_longer_period = options[1];
    const TI_REAL wma_period = options[2];
    #define START (roc_longer_period + wma_period-1)
    return START;
}


int ti_copp(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *series = inputs[0];
    const TI_REAL roc_shorter_period = options[0];
    const TI_REAL roc_longer_period = options[1];
    const TI_REAL wma_period = options[2];
    TI_REAL *copp = outputs[0];

    for (int i = 0; i < 3; ++i) {
        if (options[i] < 1) { return TI_INVALID_OPTION; }
    }
    if (roc_longer_period < roc_shorter_period) { return TI_INVALID_OPTION; }
    if (size <= ti_copp_start(options)) { return TI_OKAY; }

    ringbuf<0> price(roc_longer_period+1);
    ringbuf<0> rocs(wma_period+1);

    TI_REAL denominator = 1. / (wma_period * (wma_period + 1.) / 2.);
    TI_REAL flat_rocs_sum = 0.;
    TI_REAL weighted_rocs_sum = 0.;

    int i = 0;
    for (; i < roc_longer_period; ++i, step(price, rocs)) {
        price = series[i];
    }
    for (; i < roc_longer_period + wma_period-1; ++i, step(price, rocs)) {
        price = series[i];

        rocs = price[roc_shorter_period] && price[roc_longer_period]
            ? ((price / price[roc_shorter_period] - 1) + (price / price[roc_longer_period] - 1)) / 2. * 100.
            : 0.;

        flat_rocs_sum += rocs;
        weighted_rocs_sum += rocs * (i + 1 - roc_longer_period);
    }
    for (; i < size; ++i, step(price, rocs)) {
        price = series[i];

        rocs = price[roc_shorter_period] && price[roc_longer_period]
            ? ((price / price[roc_shorter_period] - 1) + (price / price[roc_longer_period] - 1)) / 2. * 100.
            : 0.;

        weighted_rocs_sum += rocs * wma_period;
        flat_rocs_sum += rocs;
        *copp++ = weighted_rocs_sum * denominator;

        weighted_rocs_sum -= flat_rocs_sum;
        flat_rocs_sum -= rocs[wma_period-1];
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int ti_copp_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *series = inputs[0];
    const TI_REAL roc_shorter_period = options[0];
    const TI_REAL roc_longer_period = options[1];
    const TI_REAL wma_period = options[2];
    TI_REAL *copp = outputs[0];

    for (int i = 0; i < 3; ++i) {
        if (options[i] < 1) { return TI_INVALID_OPTION; }
    }
    if (roc_longer_period < roc_shorter_period) { return TI_INVALID_OPTION; }
    if (size < ti_copp_start(options)) { return TI_OKAY; }

    // wma of (roc1 + roc2) / 2 * 100

    int roc_short_len = size-ti_roc_start(&roc_shorter_period);
    TI_REAL *roc_short = (TI_REAL*)malloc(sizeof(TI_REAL) * roc_short_len);
    int roc_long_len = size-ti_roc_start(&roc_longer_period);
    TI_REAL *roc_long = (TI_REAL*)malloc(sizeof(TI_REAL) * roc_long_len);

    TI_REAL *interm = (TI_REAL*)malloc(sizeof(TI_REAL) * roc_long_len);

    // int wma_len = roc_long_len-ti_wma_start(&wma_period);
    // TI_REAL *wma = malloc(sizeof(TI_REAL[wma_len]));

    ti_roc(size, &series, &roc_shorter_period, &roc_short);
    ti_roc(size, &series, &roc_longer_period, &roc_long);

    for (int i = 0; i < roc_long_len; ++i) {
        interm[i] = (roc_long[i] + roc_short[i+(roc_short_len-roc_long_len)]) * 100. / 2.;
    }

    ti_wma(roc_long_len, &interm, &wma_period, &copp);

    free(roc_short);
    free(roc_long);
    free(interm);

    return TI_OKAY;
}

struct ti_copp_stream : ti_stream {
    struct {
        int roc_shorter_period;
        int roc_longer_period;
        int wma_period;
        int sma_period;
    } options;

    struct {
        TI_REAL price_sum;
        TI_REAL flat_rocs_sum;
        TI_REAL weighted_rocs_sum;

        ringbuf<0> price;
        ringbuf<0> rocs;
    } state;

    struct {
        TI_REAL denominator;
    } constants;
};

int ti_copp_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int roc_shorter_period = options[0];
    const int roc_longer_period = options[1];
    const int wma_period = options[2];

    for (int i = 0; i < 3; ++i) {
        if (options[i] < 1) { return TI_INVALID_OPTION; }
    }
    if (roc_longer_period < roc_shorter_period) { return TI_INVALID_OPTION; }

    ti_copp_stream *ptr = new ti_copp_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }

    *stream = ptr;

    ptr->index = TI_INDICATOR_COPP_INDEX;
    ptr->progress = -ti_copp_start(options);

    ptr->options.roc_shorter_period = roc_shorter_period;
    ptr->options.roc_longer_period = roc_longer_period;
    ptr->options.wma_period = wma_period;

    ptr->constants.denominator = 1. / (wma_period * (wma_period + 1) / 2);

    ptr->state.flat_rocs_sum = 0.;
    ptr->state.weighted_rocs_sum = 0.;
    try {
        ptr->state.price.resize(roc_longer_period+1);
        ptr->state.rocs.resize(wma_period+1);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_copp_stream_free(ti_stream *stream) {
    delete static_cast<ti_copp_stream*>(stream);
}

int ti_copp_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_copp_stream *ptr = static_cast<ti_copp_stream*>(stream);
    int progress = ptr->progress;

    TI_REAL const *series = inputs[0];
    TI_REAL *copp = outputs[0];

    const int roc_shorter_period = ptr->options.roc_shorter_period;
    const int roc_longer_period = ptr->options.roc_longer_period;
    const int wma_period = ptr->options.wma_period;

    const TI_REAL denominator = ptr->constants.denominator;

    TI_REAL flat_rocs_sum = ptr->state.flat_rocs_sum;
    TI_REAL weighted_rocs_sum = ptr->state.weighted_rocs_sum;

    auto &price = ptr->state.price;
    auto &rocs = ptr->state.rocs;

    int i = 0;
    for (; i < size && progress < -START + roc_longer_period; ++i, ++progress, step(price, rocs)) {
        price = series[i];
    }
    for (; i < size && progress < -START + roc_longer_period + wma_period-1; ++i, ++progress, step(price, rocs)) {
        price = series[i];

        rocs = price[roc_shorter_period] && price[roc_longer_period]
            ? ((price / price[roc_shorter_period] - 1) + (price / price[roc_longer_period] - 1)) / 2. * 100.
            : 0.;

        flat_rocs_sum += rocs;
        weighted_rocs_sum += rocs * (progress + 1 - (-START + roc_longer_period));
    }
    for (; i < size; ++i, ++progress, step(price, rocs)) {
        price = series[i];

        rocs = price[roc_shorter_period] && price[roc_longer_period]
            ? ((price / price[roc_shorter_period] - 1) + (price / price[roc_longer_period] - 1)) / 2. * 100.
            : 0.;

        weighted_rocs_sum += rocs * wma_period;
        flat_rocs_sum += rocs;
        *copp++ = weighted_rocs_sum * denominator;

        weighted_rocs_sum -= flat_rocs_sum;
        flat_rocs_sum -= rocs[wma_period-1];
    }

    ptr->progress = progress;
    ptr->state.flat_rocs_sum = flat_rocs_sum;
    ptr->state.weighted_rocs_sum = weighted_rocs_sum;

    return TI_OKAY;
}
