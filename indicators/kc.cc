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
#include <algorithm>

#include "../indicators.h"
#include "../utils/ringbuf.hh"
#include "../utils/log.h"

int ti_kc_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    const TI_REAL multiple = options[1];

    return 0;
}

int ti_kc(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];
    const TI_REAL period = options[0];
    const TI_REAL multiple = options[1];
    TI_REAL *kc_lower = outputs[0];
    TI_REAL *kc_middle = outputs[1];
    TI_REAL *kc_upper = outputs[2];

    // moving average + multiple * atr
    // moving average
    // moving average - multiple * atr
    // where atr is ema of trueranges

    if (period < 1) { return TI_INVALID_OPTION; }
    if (multiple < 0) { return TI_INVALID_OPTION; }

    const TI_REAL per = 2 / ((TI_REAL)period + 1);

    TI_REAL price_ema;
    TI_REAL tr_ema;

    int i = 0;
    for (; i < 1 && i < size; ++i) {
        price_ema = close[i];
        tr_ema = high[i] - low[i];

        *kc_lower++ = price_ema - multiple * tr_ema;
        *kc_middle++ = price_ema;
        *kc_upper++ = price_ema + multiple * tr_ema;
    }
    for (; i < size; ++i) {
        price_ema = (close[i] - price_ema) * per + price_ema;

        const TI_REAL truerange = std::max(high[i], close[i-1]) - std::min(low[i], close[i-1]);
        tr_ema = (truerange - tr_ema) * per + tr_ema;

        *kc_lower++ = price_ema - multiple * tr_ema;
        *kc_middle++ = price_ema;
        *kc_upper++ = price_ema + multiple * tr_ema;
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

/// 1. close used instead of a generic input
/// 2. no ref since atr's ema and plain ema do not agree in the start amount, the plain ema way is chosen

struct ti_kc_stream : ti_stream {
    struct {
        TI_REAL period;
        TI_REAL multiple;
    } options;

    struct {
        TI_REAL price_ema;
        TI_REAL tr_ema;
        TI_REAL prev_close;
    } state;

    struct {
        TI_REAL per;
    } constants;
};

int ti_kc_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];
    const TI_REAL multiple = options[1];

    if (period < 0) { return TI_INVALID_OPTION; }
    if (multiple < 0) { return TI_INVALID_OPTION; }

    ti_kc_stream *ptr = new ti_kc_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_KC_INDEX;
    ptr->progress = -ti_kc_start(options);

    ptr->options.period = period;
    ptr->options.multiple = multiple;

    ptr->constants.per = 2. / (period + 1.);

    return TI_OKAY;
}

void ti_kc_stream_free(ti_stream *stream) {
    delete static_cast<ti_kc_stream*>(stream);
}

int ti_kc_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_kc_stream *ptr = static_cast<ti_kc_stream*>(stream);

    int progress = ptr->progress;
    const TI_REAL period = ptr->options.period;
    const TI_REAL multiple = ptr->options.multiple;

    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];
    TI_REAL *kc_lower = outputs[0];
    TI_REAL *kc_middle = outputs[1];
    TI_REAL *kc_upper = outputs[2];

    const TI_REAL per = ptr->constants.per;

    TI_REAL price_ema = ptr->state.price_ema;
    TI_REAL tr_ema = ptr->state.tr_ema;
    TI_REAL prev_close = ptr->state.prev_close;

    int i = 0;
    for (; i < size && progress < 1; ++i, ++progress) {
        price_ema = close[i];
        tr_ema = high[i] - low[i];

        *kc_lower++ = price_ema - multiple * tr_ema;
        *kc_middle++ = price_ema;
        *kc_upper++ = price_ema + multiple * tr_ema;

        prev_close = close[i];
    }
    for (i; i < size; ++i, ++progress) {
        price_ema = (close[i] - price_ema) * per + price_ema;

        const TI_REAL truerange = std::max(high[i], prev_close) - std::min(low[i], prev_close);
        tr_ema = (truerange - tr_ema) * per + tr_ema;

        *kc_lower++ = price_ema - multiple * tr_ema;
        *kc_middle++ = price_ema;
        *kc_upper++ = price_ema + multiple * tr_ema;

        prev_close = close[i];
    }

    ptr->progress = progress;
    ptr->state.price_ema = price_ema;
    ptr->state.tr_ema = tr_ema;
    ptr->state.prev_close = prev_close;

    return TI_OKAY;
}
