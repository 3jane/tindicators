/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 */


#include <new>
#include <algorithm>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_cmi_start(TI_REAL const *options) {
    const TI_REAL period = options[0];

    return period-1;
}

int ti_cmi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const high = inputs[0];
    TI_REAL const *const low = inputs[1];
    TI_REAL const *const close = inputs[2];
    const int period = options[0];
    TI_REAL *cmi = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    TI_REAL sum = 0;
    TI_REAL hh;
    TI_REAL ll;
    TI_REAL hh_idx;
    TI_REAL ll_idx;

    ringbuf<0> tr(period);

    int i = 0;

    for (; i < 1 && i < size; ++i, step(tr)) {
        tr = high[i] - low[i];
        sum += tr;
        hh = tr;
        ll = tr;
        hh_idx = 0;
        ll_idx = 0;
    }
    for (; i < period-1 && i < size; ++i, step(tr)) {
        tr = std::max(high[i], close[i-1]) - std::min(low[i], close[i-1]);
        sum += tr;
        if (hh <= tr) {
            hh = tr;
            hh_idx = i;
        }
        if (ll >= tr) {
            ll = tr;
            ll_idx = i;
        }
    }
    for (; i < size; ++i, step(tr)) {
        tr = std::max(high[i], close[i-1]) - std::min(low[i], close[i-1]);
        sum += tr;
        if (hh_idx == i - period) {
            auto it = std::max_element(tr.phbegin(), tr.phend());
            hh = *it;
            hh_idx = i - tr.iterator_to_age(it);
        } else if (hh <= tr) {
            hh = tr;
            hh_idx = i;
        }
        if (ll_idx == i - period) {
            auto it = std::min_element(tr.phbegin(), tr.phend());
            ll = *it;
            ll_idx = i - tr.iterator_to_age(it);
        } else if (ll >= tr) {
            ll = tr;
            ll_idx = i;
        }

        *cmi++ = log((hh - ll) / sum) / log(period) * 100;

        sum -= tr[period-1];
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_cmi_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const high = inputs[0];
    TI_REAL const *const low = inputs[1];
    TI_REAL const *const close = inputs[2];
    const TI_REAL period = options[0];
    TI_REAL *cmi = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    TI_REAL *tr = new TI_REAL[size];
    TI_REAL *sum = new TI_REAL[size-(int)period+1];

    ti_tr(size, inputs, NULL, &tr);
    ti_sum(size, &tr, &period, &sum);

    for (int i = period-1; i < size; ++i) {
        TI_REAL hh = *std::max_element(tr+i-(int)period+1, tr+i+1);
        TI_REAL ll = *std::min_element(tr+i-(int)period+1, tr+i+1);
        TI_REAL ink = sum[i-(int)period+1];

        *cmi++ = log((hh - ll) / ink) / log(period) * 100;
    }

    delete[] tr;
    delete[] sum;
    return TI_OKAY;
}

struct ti_cmi_stream : ti_stream {

    struct {
        TI_REAL period;
    } options;

    struct {
        TI_REAL sum = 0;
        TI_REAL hh;
        TI_REAL ll;
        TI_REAL hh_idx;
        TI_REAL ll_idx;

        ringbuf<0> tr;
        ringbuf<0> price_close;
    } state;

    struct {

    } constants;
};

int ti_cmi_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    ti_cmi_stream *ptr = new(std::nothrow) ti_cmi_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_CMI_INDEX;
    ptr->progress = -ti_cmi_start(options);

    ptr->options.period = period;

    ptr->state.tr.resize(period);
    ptr->state.price_close.resize(2);

    return TI_OKAY;
}

void ti_cmi_stream_free(ti_stream *stream) {
    delete stream;
}

int ti_cmi_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_cmi_stream *ptr = static_cast<ti_cmi_stream*>(stream);

    TI_REAL const *const high = inputs[0];
    TI_REAL const *const low = inputs[1];
    TI_REAL const *const close = inputs[2];
    TI_REAL *cmi = outputs[0];
    int progress = ptr->progress;
    const TI_REAL period = ptr->options.period;
    TI_REAL sum = ptr->state.sum;
    TI_REAL hh = ptr->state.hh;
    TI_REAL ll = ptr->state.ll;
    TI_REAL hh_idx = ptr->state.hh_idx;
    TI_REAL ll_idx = ptr->state.ll_idx;

    auto &tr = ptr->state.tr;
    auto &price_close = ptr->state.price_close;

    int i = 0;
    for (; progress < -period+2 && i < size; ++i, ++progress, step(tr, price_close)) {
        price_close = close[i];
        tr = high[i] - low[i];
        sum += tr;
        hh = tr;
        ll = tr;
        hh_idx = progress;
        ll_idx = progress;
    }
    for (; progress < 0 && i < size; ++i, ++progress, step(tr, price_close)) {
        price_close = close[i];
        tr = std::max(high[i], price_close[1]) - std::min(low[i], price_close[1]);
        sum += tr;
        if (hh <= tr) {
            hh = tr;
            hh_idx = progress;
        }
        if (ll >= tr) {
            ll = tr;
            ll_idx = progress;
        }
    }
    for (; i < size; ++i, ++progress, step(tr, price_close)) {
        price_close = close[i];
        tr = std::max(high[i], price_close[1]) - std::min(low[i], price_close[1]);
        sum += tr;
        if (hh_idx == progress - period) {
            auto it = std::max_element(tr.phbegin(), tr.phend());
            hh = *it;
            hh_idx = progress - tr.iterator_to_age(it);
        } else if (hh <= tr) {
            hh = tr;
            hh_idx = progress;
        }
        if (ll_idx == progress - period) {
            auto it = std::min_element(tr.phbegin(), tr.phend());
            ll = *it;
            ll_idx = progress - tr.iterator_to_age(it);
        } else if (ll >= tr) {
            ll = tr;
            ll_idx = progress;
        }

        *cmi++ = log((hh - ll) / sum) / log(period) * 100;

        sum -= tr[period-1];
    }

    ptr->progress = progress;
    ptr->state.sum = sum;
    ptr->state.hh = hh;
    ptr->state.ll = ll;
    ptr->state.hh_idx = hh_idx;
    ptr->state.ll_idx = ll_idx;

    return TI_OKAY;
}
