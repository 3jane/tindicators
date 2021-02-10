/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 */


#include <new>
#include <algorithm>
#include <limits>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_ichi_start(TI_REAL const *options) {
    const int period9 = options[0];
    const int period26 = options[1];
    const int period52 = options[2];
    return std::max({
        period9-1,
        period26-1,
        period52-1 + period26,
        period9-1 + period26
    });
}

int ti_ichi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *const high = inputs[0];
    TI_REAL const *const low = inputs[1];
    const int period9 = options[0];
    const int period26 = options[1];
    const int period52 = options[2];
    TI_REAL *ichi_tenkan_sen = outputs[0];
    TI_REAL *ichi_kijun_sen = outputs[1];
    TI_REAL *ichi_senkou_span_A = outputs[2];
    TI_REAL *ichi_senkou_span_B = outputs[3];

    if (!(0 < period9 && period9 <= period26 && period26 <= period52)) {
        return TI_INVALID_OPTION;
    }

    // Tenkan Sen
    TI_REAL hh9 = -std::numeric_limits<TI_REAL>::infinity();
    TI_REAL ll9 = std::numeric_limits<TI_REAL>::infinity();
    int hh9_idx = 0;
    int ll9_idx = 0;

    // Kijun Sen
    TI_REAL hh26 = -std::numeric_limits<TI_REAL>::infinity();
    TI_REAL ll26 = std::numeric_limits<TI_REAL>::infinity();
    int hh26_idx = 0;
    int ll26_idx = 0;

    TI_REAL hh52 = -std::numeric_limits<TI_REAL>::infinity();
    TI_REAL ll52 = std::numeric_limits<TI_REAL>::infinity();
    int hh52_idx = 0;
    int ll52_idx = 0;

    ringbuf<0> buf_ichi_senkou_span_A(period26+1);
    ringbuf<0> buf_ichi_senkou_span_B(period26+1);

    int i = 0;
    for (; i < period52-1 + period26 && i < size; ++i, step(buf_ichi_senkou_span_A, buf_ichi_senkou_span_B)) {
        if (hh9_idx == i - period9) {
            auto it = std::max_element(high+i-period9+1, high+i+1);
            hh9 = *it;
            hh9_idx = it - high;
        } else if (high[i] >= hh9) {
            hh9 = high[i];
            hh9_idx = i;
        }
        if (ll9_idx == i - period9) {
            auto it = std::min_element(low+i-period9+1, low+i+1);
            ll9 = *it;
            ll9_idx = it - low;
        } else if (low[i] <= ll9) {
            ll9 = low[i];
            ll9_idx = i;
        }
        if (hh26_idx == i - period26) {
            auto it = std::max_element(high+i-period26+1, high+i+1);
            hh26 = *it;
            hh26_idx = it - high;
        } else if (high[i] >= hh26) {
            hh26 = high[i];
            hh26_idx = i;
        }
        if (ll26_idx == i - period26) {
            auto it = std::min_element(low+i-period26+1, low+i+1);
            ll26 = *it;
            ll26_idx = it - low;
        } else if (low[i] <= ll26) {
            ll26 = low[i];
            ll26_idx = i;
        }
        if (hh52_idx == i - period52) {
            auto it = std::max_element(high+i-period52+1, high+i+1);
            hh52 = *it;
            hh52_idx = it - high;
        } else if (high[i] >= hh52) {
            hh52 = high[i];
            hh52_idx = i;
        }
        if (ll52_idx == i - period52) {
            auto it = std::min_element(low+i-period52+1, low+i+1);
            ll52 = *it;
            ll52_idx = it - low;
        } else if (low[i] <= ll52) {
            ll52 = low[i];
            ll52_idx = i;
        }

        const TI_REAL tenkan_sen = (hh9 + ll9) / 2.;
        const TI_REAL kijun_sen = (hh26 + ll26) / 2.;
        buf_ichi_senkou_span_A = (tenkan_sen + kijun_sen) / 2.;
        buf_ichi_senkou_span_B = (hh52 + ll52) / 2.;
    }
    for (; i < size; ++i, step(buf_ichi_senkou_span_A, buf_ichi_senkou_span_B)) {
        if (hh9_idx == i - period9) {
            auto it = std::max_element(high+i-period9+1, high+i+1);
            hh9 = *it;
            hh9_idx = it - high;
        } else if (high[i] >= hh9) {
            hh9 = high[i];
            hh9_idx = i;
        }
        if (ll9_idx == i - period9) {
            auto it = std::min_element(low+i-period9+1, low+i+1);
            ll9 = *it;
            ll9_idx = it - low;
        } else if (low[i] <= ll9) {
            ll9 = low[i];
            ll9_idx = i;
        }
        if (hh26_idx == i - period26) {
            auto it = std::max_element(high+i-period26+1, high+i+1);
            hh26 = *it;
            hh26_idx = it - high;
        } else if (high[i] >= hh26) {
            hh26 = high[i];
            hh26_idx = i;
        }
        if (ll26_idx == i - period26) {
            auto it = std::min_element(low+i-period26+1, low+i+1);
            ll26 = *it;
            ll26_idx = it - low;
        } else if (low[i] <= ll26) {
            ll26 = low[i];
            ll26_idx = i;
        }
        if (hh52_idx == i - period52) {
            auto it = std::max_element(high+i-period52+1, high+i+1);
            hh52 = *it;
            hh52_idx = it - high;
        } else if (high[i] >= hh52) {
            hh52 = high[i];
            hh52_idx = i;
        }
        if (ll52_idx == i - period52) {
            auto it = std::min_element(low+i-period52+1, low+i+1);
            ll52 = *it;
            ll52_idx = it - low;
        } else if (low[i] <= ll52) {
            ll52 = low[i];
            ll52_idx = i;
        }

        const TI_REAL tenkan_sen = (hh9 + ll9) / 2.;
        const TI_REAL kijun_sen = (hh26 + ll26) / 2.;
        buf_ichi_senkou_span_A = (tenkan_sen + kijun_sen) / 2.;
        buf_ichi_senkou_span_B = (hh52 + ll52) / 2.;

        *ichi_tenkan_sen++ = tenkan_sen;
        *ichi_kijun_sen++ = kijun_sen;
        *ichi_senkou_span_A++ = buf_ichi_senkou_span_A[period26];
        *ichi_senkou_span_B++ = buf_ichi_senkou_span_B[period26];
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

DONTOPTIMIZE int ti_ichi_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const high = inputs[0];
    TI_REAL const *const low = inputs[1];
    const int period9 = options[0];
    const int period26 = options[1];
    const int period52 = options[2];
    TI_REAL *ichi_tenkan_sen = outputs[0];
    TI_REAL *ichi_kijun_sen = outputs[1];
    TI_REAL *ichi_senkou_span_A = outputs[2];
    TI_REAL *ichi_senkou_span_B = outputs[3];

    if (!(0 < period9 && period9 <= period26 && period26 <= period52)) {
        return TI_INVALID_OPTION;
    }

    TI_REAL *hh9 = new TI_REAL[size];
    TI_REAL *ll9 = new TI_REAL[size];
    TI_REAL *hh26 = new TI_REAL[size];
    TI_REAL *ll26 = new TI_REAL[size];
    TI_REAL *hh52 = new TI_REAL[size];
    TI_REAL *ll52 = new TI_REAL[size];

    const TI_REAL numbers[] = {(TI_REAL)period9, (TI_REAL)period26, (TI_REAL)period52};

    TI_REAL *arr[1];
    arr[0] = &hh9[period9-1];
    ti_max(size, &high, numbers, arr);
    arr[0] = &ll9[period9-1];
    ti_min(size, &low, numbers, arr);
    arr[0] = &hh26[period26-1];
    ti_max(size, &high, numbers + 1, arr);
    arr[0] = &ll26[period26-1];
    ti_min(size, &low, numbers + 1, arr);
    arr[0] = &hh52[period52-1];
    ti_max(size, &high, numbers + 2, arr);
    arr[0] = &ll52[period52-1];
    ti_min(size, &low, numbers + 2, arr);

    for (int i = period52-1 + period26; i < size; ++i) {
        *ichi_tenkan_sen++ = (hh9[i] + ll9[i]) / 2.;
        *ichi_kijun_sen++ = (hh26[i] + ll26[i]) / 2.;
        *ichi_senkou_span_A++ = (
            (hh9[i-period26] + ll9[i-period26]) / 2. +
            (hh26[i-period26] + ll26[i-period26]) / 2.
        ) / 2.;
        *ichi_senkou_span_B++ = (hh52[i-period26] + ll52[i-period26]) / 2.;
    }

    delete[] hh9;
    delete[] ll9;
    delete[] hh26;
    delete[] ll26;
    delete[] hh52;
    delete[] ll52;

    return TI_OKAY;
}

struct ti_ichi_stream : ti_stream {

    struct {
        int period9;
        int period26;
        int period52;
    } options;

    struct {
        // Tenkan Sen
        TI_REAL hh9 = -std::numeric_limits<TI_REAL>::infinity();
        TI_REAL ll9 = std::numeric_limits<TI_REAL>::infinity();
        int hh9_idx;
        int ll9_idx;

        // Kijun Sen
        TI_REAL hh26 = -std::numeric_limits<TI_REAL>::infinity();
        TI_REAL ll26 = std::numeric_limits<TI_REAL>::infinity();
        int hh26_idx;
        int ll26_idx;

        TI_REAL hh52 = -std::numeric_limits<TI_REAL>::infinity();
        TI_REAL ll52 = std::numeric_limits<TI_REAL>::infinity();
        int hh52_idx;
        int ll52_idx;

        ringbuf<0> buf_ichi_senkou_span_A;
        ringbuf<0> buf_ichi_senkou_span_B;

        ringbuf<0> price_high;
        ringbuf<0> price_low;
    } state;

    struct {

    } constants;
};

int ti_ichi_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period9 = options[0];
    const int period26 = options[1];
    const int period52 = options[2];

    if (!(0 < period9 && period9 <= period26 && period26 <= period52)) {
        return TI_INVALID_OPTION;
    }

    ti_ichi_stream *ptr = new(std::nothrow) ti_ichi_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_ICHI_INDEX;
    ptr->progress = -ti_ichi_start(options);

    ptr->options.period9 = period9;
    ptr->options.period26 = period26;
    ptr->options.period52 = period52;

    try {
        ptr->state.buf_ichi_senkou_span_A.resize(period26+1);
        ptr->state.buf_ichi_senkou_span_B.resize(period26+1);
        ptr->state.price_high.resize(period52);
        ptr->state.price_low.resize(period52);
    } catch (std::bad_alloc& e) {
        delete *stream;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_ichi_stream_free(ti_stream *stream) {
    delete static_cast<ti_ichi_stream*>(stream);
}

int ti_ichi_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_ichi_stream *ptr = static_cast<ti_ichi_stream*>(stream);
    TI_REAL const *const high = inputs[0];
    TI_REAL const *const low = inputs[1];
    TI_REAL *ichi_tenkan_sen = outputs[0];
    TI_REAL *ichi_kijun_sen = outputs[1];
    TI_REAL *ichi_senkou_span_A = outputs[2];
    TI_REAL *ichi_senkou_span_B = outputs[3];
    const int period9 = ptr->options.period9;
    const int period26 = ptr->options.period26;
    const int period52 = ptr->options.period52;

    int progress = ptr->progress;

    // Tenkan Sen
    TI_REAL hh9 = ptr->state.hh9;
    TI_REAL ll9 = ptr->state.ll9;
    int hh9_idx = ptr->state.hh9_idx;
    int ll9_idx = ptr->state.ll9_idx;

    // Kijun Sen
    TI_REAL hh26 = ptr->state.hh26;
    TI_REAL ll26 = ptr->state.ll26;
    int hh26_idx = ptr->state.hh26_idx;
    int ll26_idx = ptr->state.ll26_idx;

    TI_REAL hh52 = ptr->state.hh52;
    TI_REAL ll52 = ptr->state.ll52;
    int hh52_idx = ptr->state.hh52_idx;
    int ll52_idx = ptr->state.ll52_idx;

    auto &buf_ichi_senkou_span_A = ptr->state.buf_ichi_senkou_span_A;
    auto &buf_ichi_senkou_span_B = ptr->state.buf_ichi_senkou_span_B;
    auto &price_high = ptr->state.price_high;
    auto &price_low = ptr->state.price_low;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(buf_ichi_senkou_span_A, buf_ichi_senkou_span_B, price_high, price_low)) {
        price_high = high[i];
        price_low = low[i];

        if (hh9_idx == progress - period9) {
            auto it = price_high.find_max(period9);
            hh9 = *it;
            hh9_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= hh9) {
            hh9 = high[i];
            hh9_idx = progress;
        }
        if (ll9_idx == progress - period9) {
            auto it = price_low.find_min(period9);
            ll9 = *it;
            ll9_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= ll9) {
            ll9 = low[i];
            ll9_idx = progress;
        }
        if (hh26_idx == progress - period26) {
            auto it = price_high.find_max(period26);
            hh26 = *it;
            hh26_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= hh26) {
            hh26 = high[i];
            hh26_idx = progress;
        }
        if (ll26_idx == progress - period26) {
            auto it = price_low.find_min(period26);
            ll26 = *it;
            ll26_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= ll26) {
            ll26 = low[i];
            ll26_idx = progress;
        }
        if (hh52_idx == progress - period52) {
            auto it = price_high.find_max(period52);
            hh52 = *it;
            hh52_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= hh52) {
            hh52 = high[i];
            hh52_idx = progress;
        }
        if (ll52_idx == progress - period52) {
            auto it = price_low.find_min(period52);
            ll52 = *it;
            ll52_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= ll52) {
            ll52 = low[i];
            ll52_idx = progress;
        }

        const TI_REAL tenkan_sen = (hh9 + ll9) / 2.;
        const TI_REAL kijun_sen = (hh26 + ll26) / 2.;
        buf_ichi_senkou_span_A = (tenkan_sen + kijun_sen) / 2.;
        buf_ichi_senkou_span_B = (hh52 + ll52) / 2.;
    }
    for (; i < size; ++i, ++progress, step(buf_ichi_senkou_span_A, buf_ichi_senkou_span_B, price_high, price_low)) {
        price_high = high[i];
        price_low = low[i];

        if (hh9_idx == progress - period9) {
            auto it = price_high.find_max(period9);
            hh9 = *it;
            hh9_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= hh9) {
            hh9 = high[i];
            hh9_idx = progress;
        }
        if (ll9_idx == progress - period9) {
            auto it = price_low.find_min(period9);
            ll9 = *it;
            ll9_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= ll9) {
            ll9 = low[i];
            ll9_idx = progress;
        }
        if (hh26_idx == progress - period26) {
            auto it = price_high.find_max(period26);
            hh26 = *it;
            hh26_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= hh26) {
            hh26 = high[i];
            hh26_idx = progress;
        }
        if (ll26_idx == progress - period26) {
            auto it = price_low.find_min(period26);
            ll26 = *it;
            ll26_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= ll26) {
            ll26 = low[i];
            ll26_idx = progress;
        }
        if (hh52_idx == progress - period52) {
            auto it = price_high.find_max(period52);
            hh52 = *it;
            hh52_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= hh52) {
            hh52 = high[i];
            hh52_idx = progress;
        }
        if (ll52_idx == progress - period52) {
            auto it = price_low.find_min(period52);
            ll52 = *it;
            ll52_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= ll52) {
            ll52 = low[i];
            ll52_idx = progress;
        }

        const TI_REAL tenkan_sen = (hh9 + ll9) / 2.;
        const TI_REAL kijun_sen = (hh26 + ll26) / 2.;
        buf_ichi_senkou_span_A = (tenkan_sen + kijun_sen) / 2.;
        buf_ichi_senkou_span_B = (hh52 + ll52) / 2.;

        *ichi_tenkan_sen++ = tenkan_sen;
        *ichi_kijun_sen++ = kijun_sen;
        *ichi_senkou_span_A++ = buf_ichi_senkou_span_A[period26];
        *ichi_senkou_span_B++ = buf_ichi_senkou_span_B[period26];
    }

    ptr->progress = progress;
    ptr->state.hh9 = hh9;
    ptr->state.ll9 = ll9;
    ptr->state.hh9_idx = hh9_idx;
    ptr->state.ll9_idx = ll9_idx;
    ptr->state.hh26 = hh26;
    ptr->state.ll26 = ll26;
    ptr->state.hh26_idx = hh26_idx;
    ptr->state.ll26_idx = ll26_idx;
    ptr->state.hh52 = hh52;
    ptr->state.ll52 = ll52;
    ptr->state.hh52_idx = hh52_idx;
    ptr->state.ll52_idx = ll52_idx;

    return TI_OKAY;
}
