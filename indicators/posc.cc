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

#include "../indicators.h"
#include "../utils/ringbuf.hh"

#include <new>
#include <algorithm>

int ti_posc_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    const TI_REAL ema_period = options[1];
    return period-1;
}


int ti_posc(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    const int period = options[0];
    const int ema_period = options[1];
    TI_REAL *posc = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (ema_period < 1) { return TI_INVALID_OPTION; }
    if (size <= ti_posc_start(options)) { return TI_OKAY; }

    TI_REAL y_sum = 0.;
    TI_REAL xy_sum = 0.;
    TI_REAL ema;

    const TI_REAL x_sum = period * (period + 1) / 2.;
    const TI_REAL xsq_sum = period * (period + 1) * (2*period + 1) / 6.;

    int i = 0;
    for (; i < period-1 && i < size; ++i) {
        xy_sum += close[i] * (i + 1);
        y_sum += close[i];
    }
    for (; i < period && i < size; ++i) {
        xy_sum += close[i] * period;
        y_sum += close[i];
        // y = a + bx
        TI_REAL b = (xy_sum / period - x_sum / period * y_sum / period) / (xsq_sum / period - (x_sum / period)*(x_sum / period));

        TI_REAL the_max = high[i];
        for (int j = 1; j < period; ++j) {
            the_max = std::max(the_max, high[i-j] + j * b);
        }
        TI_REAL the_min = low[i];
        for (int j = 1; j < period; ++j) {
            the_min = std::min(the_min, low[i-j] + j * b);
        }

        ema = (close[i] - the_min) ? (close[i] - the_min) / (the_max - the_min) * 100. : 0;
        *posc++ = ema;

        xy_sum -= y_sum;
        y_sum -= close[i-period+1];
    }
    for (; i < size; ++i) {
        xy_sum += close[i] * period;
        y_sum += close[i];

        // y = a + bx
        TI_REAL b = (xy_sum / period - x_sum / period * y_sum / period) / (xsq_sum / period - (x_sum / period)*(x_sum / period));

        TI_REAL the_max = high[i];
        for (int j = 1; j < period; ++j) {
            the_max = std::max(the_max, high[i-j] + j * b);
        }
        TI_REAL the_min = low[i];
        for (int j = 1; j < period; ++j) {
            the_min = std::min(the_min, low[i-j] + j * b);
        }

        TI_REAL osc = (close[i] - the_min) ? (close[i] - the_min) / (the_max - the_min) * 100. : 0;
        ema = (osc - ema) * 2. / (1 + ema_period) + ema;
        *posc++ = ema;

        xy_sum -= y_sum;
        y_sum -= close[i-(int)period+1];
    }
    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int ti_posc_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    const TI_REAL period = options[0];
    const TI_REAL ema_period = options[1];
    TI_REAL *posc = outputs[0];

    int start = ti_linregslope_start(options);
    TI_REAL *b = (TI_REAL*)malloc(sizeof(TI_REAL) * (size - start));

    ti_linregslope(size, &close, &period, &b);

    for (int i = start; i < size; ++i) {
        TI_REAL the_max = high[i];
        for (int j = 1; j < period; ++j) {
            if (the_max < high[i-j] + j * b[i-start]) {
                the_max = high[i-j] + j * b[i-start];
            }
        }
        TI_REAL the_min = low[i];
        for (int j = 1; j < period; ++j) {
            if (the_min > low[i-j] + j * b[i-start]) {
                the_min = low[i-j] + j * b[i-start];
            }
        }

        posc[i-start] = (close[i] - the_min) / (the_max - the_min) * 100.;
    }
    ti_ema(size-start, &posc, &ema_period, &posc);

    free(b);

    return TI_OKAY;
}


struct ti_posc_stream : ti_stream {
    struct {
        TI_REAL period;
        TI_REAL ema_period;
    } options;

    struct {
        TI_REAL y_sum;
        TI_REAL xy_sum;
        TI_REAL ema;

        ringbuf<0> price_high;
        ringbuf<0> price_low;
        ringbuf<0> price_close;
    } state;

    struct {
        TI_REAL x_sum;
        TI_REAL xsq_sum;
    } constants;
};

int ti_posc_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];
    const TI_REAL ema_period = options[1];
    if (period < 1) { return TI_INVALID_OPTION; }
    if (ema_period < 1) { return TI_INVALID_OPTION; }

    ti_posc_stream *ptr = new ti_posc_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_POSC_INDEX;
    ptr->progress = -ti_posc_start(options);

    ptr->options.period = period;
    ptr->options.ema_period = ema_period;

    ptr->state.y_sum = 0.;
    ptr->state.xy_sum = 0.;

    ptr->constants.x_sum = period * (period + 1) / 2.;
    ptr->constants.xsq_sum = period * (period + 1) * (2*period + 1) / 6.;

    try {
        ptr->state.price_high.resize(period+1);
        ptr->state.price_low.resize(period+1);
        ptr->state.price_close.resize(period+1);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_posc_stream_free(ti_stream *stream) {
    delete static_cast<ti_posc_stream*>(stream);
}

int ti_posc_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_posc_stream *ptr = static_cast<ti_posc_stream*>(stream);

    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    TI_REAL *posc = outputs[0];

    int progress = ptr->progress;

    const TI_REAL period = ptr->options.period;
    const TI_REAL ema_period = ptr->options.ema_period;

    TI_REAL y_sum = ptr->state.y_sum;
    TI_REAL xy_sum = ptr->state.xy_sum;
    TI_REAL ema = ptr->state.ema;

    const TI_REAL xsq_sum = ptr->constants.xsq_sum;
    const TI_REAL x_sum = ptr->constants.x_sum;

    auto &price_high = ptr->state.price_high;
    auto &price_low = ptr->state.price_low;
    auto &price_close = ptr->state.price_close;

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress, step(price_high, price_low, price_close)) {
        price_high = high[i];
        price_low = low[i];
        price_close = close[i];

        xy_sum += close[i] * (progress - (-period+1) + 1);
        y_sum += close[i];
    }
    for (; i < size && progress < 1; ++i, ++progress, step(price_high, price_low, price_close)) {
        price_high = high[i];
        price_low = low[i];
        price_close = close[i];

        xy_sum += close[i] * period;
        y_sum += close[i];

        // y = a + bx
        TI_REAL b = (xy_sum / period - x_sum / period * y_sum / period) / (xsq_sum / period - (x_sum / period)*(x_sum / period));

        TI_REAL the_max = price_high;
        for (int j = 1; j < period; ++j) {
            the_max = std::max(the_max, price_high[j] + j * b);
        }
        TI_REAL the_min = price_low;
        for (int j = 1; j < period; ++j) {
            the_min = std::min(the_min, price_low[j] + j * b);
        }
        ema = (close[i] - the_min) ? (close[i] - the_min) / (the_max - the_min) * 100. : 0;
        *posc++ = ema;

        xy_sum -= y_sum;
        y_sum -= price_close[period-1];
    }
    for (; i < size; ++i, ++progress, step(price_high, price_low, price_close)) {
        price_high = high[i];
        price_low = low[i];
        price_close = close[i];

        xy_sum += close[i] * period;
        y_sum += close[i];

        // y = a + bx
        TI_REAL b = (xy_sum / period - x_sum / period * y_sum / period) / (xsq_sum / period - (x_sum / period)*(x_sum / period));

        TI_REAL the_max = price_high;
        for (int j = 1; j < period; ++j) {
            the_max = std::max(the_max, price_high[j] + j * b);
        }
        TI_REAL the_min = price_low;
        for (int j = 1; j < period; ++j) {
            the_min = std::min(the_min, price_low[j] + j * b);
        }
        TI_REAL osc = (close[i] - the_min) ? (close[i] - the_min) / (the_max - the_min) * 100. : 0;
        ema = (osc - ema) * 2. / (1 + ema_period) + ema;
        *posc++ = ema;

        xy_sum -= y_sum;
        y_sum -= price_close[period-1];
    }

    ptr->progress = progress;

    ptr->state.y_sum = y_sum;
    ptr->state.xy_sum = xy_sum;
    ptr->state.ema = ema;

    return TI_OKAY;
}
