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

int ti_ce_start(TI_REAL const *options) {
    return (int)options[0]-1;
}

/* Name: Chandelier Exit
 * Sources:
 *  - Alexander Elder. Come Into My Trading Room, 2002, pp. 180-181. CE, original description
 *    ISBN: 9780471225348
 *  - J. Welles Wilder. New Concepts in Technical Trading Systems, 1978, pp. 21-23. ATR, original description
 *    ISBN: 9780894590276
 */

int ti_ce(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    const int period = options[0];
    const TI_REAL coef = options[1];
    TI_REAL *ce_high = outputs[0];
    TI_REAL *ce_low = outputs[1];

    if (period < 1) { return TI_INVALID_OPTION; }

    const TI_REAL per = 1. / period;

    TI_REAL atr = 0;

    TI_REAL max = -std::numeric_limits<TI_REAL>::infinity();
    TI_REAL min = std::numeric_limits<TI_REAL>::infinity();
    int max_idx;
    int min_idx;

    int i = 0;
    for (; i < 1 && i < size; ++i) {
        atr = (high[i] - low[i]) * per;
        max = high[i];
        min = low[i];
        max_idx = 0;
        min_idx = 0;
    }
    for (; i < period-1 && i < size; ++i) {
        const TI_REAL truerange = std::max(high[i], close[i-1]) - std::min(low[i], close[i-1]);
        atr += truerange * per;

        if (max <= high[i]) {
            max = high[i];
            max_idx = i;
        }
        if (min >= low[i]) {
            min = low[i];
            min_idx = i;
        }
    }
    for (; i < period && i < size; ++i) {
        const TI_REAL truerange = std::max(high[i], close[i-1]) - std::min(low[i], close[i-1]);
        atr += truerange * per;

        if (max <= high[i]) {
            max = high[i];
            max_idx = i;
        }
        if (min >= low[i]) {
            min = low[i];
            min_idx = i;
        }
        *ce_high++ = max - coef * atr;
        *ce_low++ = min + coef * atr;
    }
    for (; i < size; ++i) {
        const TI_REAL truerange = std::max(high[i], close[i-1]) - std::min(low[i], close[i-1]);
        atr = (truerange - atr) * per + atr;

        if (max_idx == i - period) {
            auto it = std::max_element(std::make_reverse_iterator(high + i + 1), std::make_reverse_iterator(high + i - period + 1)).base() - 1;
            max = *it;
            max_idx = it - high;
        } else if (max <= high[i]) {
            max = high[i];
            max_idx = i;
        }
        if (min_idx == i - period) {
            auto it = std::min_element(std::make_reverse_iterator(low + i + 1), std::make_reverse_iterator(low + i - period + 1)).base() - 1;
            min = *it;
            min_idx = it - low;
        } else if (min >= low[i]) {
            min = low[i];
            min_idx = i;
        }

        *ce_high++ = max - coef * atr;
        *ce_low++ = min + coef * atr;
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int ti_ce_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    const TI_REAL period = options[0];
    const TI_REAL coef = options[1];
    TI_REAL *ce_high = outputs[0];
    TI_REAL *ce_low = outputs[1];

    if (size <= ti_ce_start(options)) { return TI_OKAY; }
    if (period < 1) { return TI_INVALID_OPTION; }

    #define MAX(a, b) (a > b ? a : b)
    #define MAX3(a, b, c) (a > b && a > c ? a : b > a && b > c ? b : c)

    TI_REAL *atr = (TI_REAL*)calloc(1, (size - ti_atr_start(&period)) * sizeof(TI_REAL));
	if (!atr) { return TI_OUT_OF_MEMORY; }
    ti_atr(size, inputs, &period, &atr);
    TI_REAL *max = (TI_REAL*)calloc(1, (size - ti_max_start(&period)) * sizeof(TI_REAL));
	if (!atr) { return TI_OUT_OF_MEMORY; }
	ti_max(size, &high, &period, &max);
    TI_REAL *min = (TI_REAL*)calloc(1, (size - ti_min_start(&period)) * sizeof(TI_REAL));
	if (!atr) { return TI_OUT_OF_MEMORY; }
	ti_min(size, &low, &period, &min);


    for (int i = 0; i < size - period + 1; ++i) {
        ce_high[i] = max[i] - coef * atr[i];
        ce_low[i] = min[i] + coef * atr[i];
    }

    free(atr);
    free(max);
    free(min);

    return TI_OKAY;
}

struct ti_ce_stream : ti_stream {
    struct {
        int period;
        TI_REAL coef;
    } options;

    struct {
        TI_REAL max = -std::numeric_limits<TI_REAL>::infinity();
        TI_REAL min = std::numeric_limits<TI_REAL>::infinity();
        int max_idx;
        int min_idx;

        TI_REAL atr;
        TI_REAL prev_close;

        ringbuf<0> price_high;
        ringbuf<0> price_low;
    } state;

    struct {
        TI_REAL smth;
        TI_REAL per;
    } constants;
};

int ti_ce_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];
    const TI_REAL coef = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    ti_ce_stream *ptr = new ti_ce_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_CE_INDEX;
    ptr->progress = -ti_ce_start(options);

    ptr->options.period = period;
    ptr->options.coef = coef;

    ptr->constants.smth = (period - 1.) / period;
    ptr->constants.per = 1. / period;

    try {
        ptr->state.price_high.resize(period);
        ptr->state.price_low.resize(period);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_ce_stream_free(ti_stream *stream) {
    delete static_cast<ti_ce_stream*>(stream);
}

int ti_ce_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_ce_stream *ptr = static_cast<ti_ce_stream*>(stream);
    int progress = ptr->progress;

    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    const int period = ptr->options.period;
    const TI_REAL coef = ptr->options.coef;
    TI_REAL *ce_high = outputs[0];
    TI_REAL *ce_low = outputs[1];

    TI_REAL smth = ptr->constants.smth;
    TI_REAL per = ptr->constants.per;

    TI_REAL max = ptr->state.max;
    TI_REAL min = ptr->state.min;
    int max_idx = ptr->state.max_idx;
    int min_idx = ptr->state.min_idx;

    TI_REAL atr = ptr->state.atr;
    TI_REAL prev_close = ptr->state.prev_close;

    auto &price_high = ptr->state.price_high;
    auto &price_low = ptr->state.price_low;

    int i = 0;
    for (; progress < -period+1 + 1 && i < size; ++i, ++progress, step(price_high, price_low)) {
        price_high = high[i];
        price_low = low[i];

        atr = (high[i] - low[i]) * per;

        max = high[i];
        min = low[i];
        max_idx = progress;
        min_idx = progress;

        prev_close = close[i];
    }
    for (i; progress < 0 && i < size; ++i, ++progress, step(price_high, price_low)) {
        price_high = high[i];
        price_low = low[i];

        if (max_idx == progress - period) {
            auto it = price_high.find_max(period);
            max = *it;
            max_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= max) {
            max = high[i];
            max_idx = progress;
        }
        if (min_idx == progress - period) {
            auto it = price_low.find_min(period);
            min = *it;
            min_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= min) {
            min = low[i];
            min_idx = progress;
        }

        const TI_REAL truerange = std::max(high[i], prev_close) - std::min(low[i], prev_close);
        atr += truerange * per;

        prev_close = close[i];
    }
    for (i; progress < 1 && i < size; ++i, ++progress, step(price_high, price_low)) {
        price_high = high[i];
        price_low = low[i];

        const TI_REAL truerange = std::max(high[i], prev_close) - std::min(low[i], prev_close);
        atr += truerange * per;

        if (max_idx == progress - period) {
            auto it = price_high.find_max(period);
            max = *it;
            max_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= max) {
            max = high[i];
            max_idx = progress;
        }
        if (min_idx == progress - period) {
            auto it = price_low.find_min(period);
            min = *it;
            min_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= min) {
            min = low[i];
            min_idx = progress;
        }

        *ce_high++ = max - coef * atr;
        *ce_low++ = min + coef * atr;

        prev_close = close[i];
    }
    for (i; i < size; ++i, ++progress, step(price_high, price_low)) {
        price_high = high[i];
        price_low = low[i];

        const TI_REAL truerange = std::max(high[i], prev_close) - std::min(low[i], prev_close);
        atr = (truerange - atr) * per + atr;

        if (max_idx == progress - period) {
            auto it = price_high.find_max(period);
            max = *it;
            max_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= max) {
            max = high[i];
            max_idx = progress;
        }
        if (min_idx == progress - period) {
            auto it = price_low.find_min(period);
            min = *it;
            min_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= min) {
            min = low[i];
            min_idx = progress;
        }

        *ce_high++ = max - coef * atr;
        *ce_low++ = min + coef * atr;

        prev_close = close[i];
    }

    ptr->progress = progress;
    ptr->state.max = max;
    ptr->state.min = min;
    ptr->state.max_idx = max_idx;
    ptr->state.min_idx = min_idx;
    ptr->state.atr = atr;
    ptr->state.prev_close = prev_close;

    return TI_OKAY;
}
