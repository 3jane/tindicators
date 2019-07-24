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

int ti_vwap_start(TI_REAL const *options) {
    return (int)options[0]-1;
}

int ti_vwap(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    TI_REAL const *volume = inputs[3];
    const TI_REAL period = options[0];
    TI_REAL *vwap = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    int progress = -period+1;

    TI_REAL num = 0;
    TI_REAL den = 0;

    int i = 0;
    for (; i < size && progress < 1; ++i, ++progress) {
        num += (high[i] + low[i] + close[i]) / 3. * volume[i];
        den += volume[i];
    }
    if (i > 0 && progress == 1) {
        *vwap++ = num ? num / den : 0;
    }
    for (; i < size; ++i, ++progress) {
        num += (high[i] + low[i] + close[i]) / 3. * volume[i]
            - (high[i-(int)period] + low[i-(int)period] + close[i-(int)period]) / 3. * volume[i-(int)period];
        den += volume[i] - volume[i-(int)period];

        *vwap++ = num ? num / den : 0;
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int ti_vwap_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    TI_REAL const *volume = inputs[3];
    const TI_REAL period = options[0];
    TI_REAL *vwap = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    int outsize = size - ti_typprice_start(&period);
    TI_REAL *typprice = (TI_REAL*)malloc(sizeof(TI_REAL) * outsize);
    ti_typprice(size, inputs, 0, &typprice);
    
    TI_REAL sum = 0;
    TI_REAL vsum = 0;

    int i;
    for (i = 0; i < size && i < period; ++i) {
        sum += typprice[i] * volume[i];
        vsum += volume[i];
    }

    *vwap++ = sum / vsum;

    for (i = period; i < size; ++i) {
        sum += typprice[i] * volume[i];
        sum -= typprice[i-(int)period] * volume[i-(int)period];
        vsum += volume[i];
        vsum -= volume[i-(int)period];

        *vwap++ = sum / vsum;
    }

    free(typprice);

    return TI_OKAY;
}

struct ti_vwap_stream : ti_stream {
    struct {
        int period;
    } options;

    struct {
        TI_REAL num;
        TI_REAL den;

        ringbuf<0> price_high;
        ringbuf<0> price_low;
        ringbuf<0> price_close;
        ringbuf<0> price_volume;
    } state;

    struct {

    } constants;
};

int ti_vwap_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];
    if (period < 1) { return TI_INVALID_OPTION; }

    ti_vwap_stream *ptr = new(std::nothrow) ti_vwap_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_VWAP_INDEX;
    ptr->progress = -ti_vwap_start(options);

    ptr->options.period = period;

    ptr->state.num = 0;
    ptr->state.den = 0;

    try {
        ptr->state.price_high.resize(period + 1);
        ptr->state.price_low.resize(period + 1);
        ptr->state.price_close.resize(period + 1);
        ptr->state.price_volume.resize(period + 1);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_vwap_stream_free(ti_stream *stream) {
    delete static_cast<ti_vwap_stream*>(stream);
}

int ti_vwap_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_vwap_stream *ptr = static_cast<ti_vwap_stream*>(stream);

    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    TI_REAL const *volume = inputs[3];
    TI_REAL *vwap = outputs[0];

    int progress = ptr->progress;

    int period = ptr->options.period;

    TI_REAL num = ptr->state.num;
    TI_REAL den = ptr->state.den;

    auto &price_high = ptr->state.price_high;
    auto &price_low = ptr->state.price_low;
    auto &price_close = ptr->state.price_close;
    auto &price_volume = ptr->state.price_volume;

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress, step(price_high, price_low, price_close, price_volume)) {
        price_high = high[i];
        price_low = low[i];
        price_close = close[i];
        price_volume = volume[i];

        num += (high[i] + low[i] + close[i]) / 3. * volume[i];
        den += volume[i];
    }
    for (; i < size; ++i, ++progress, step(price_high, price_low, price_close, price_volume)) {
        price_high = high[i];
        price_low = low[i];
        price_close = close[i];
        price_volume = volume[i];

        num += (high[i] + low[i] + close[i]) / 3. * volume[i];
        den += volume[i];

        *vwap++ = num ? num / den : 0;

        num -= (price_high[period-1] + price_low[period-1] + price_close[period-1]) / 3. * price_volume[period-1];
        den -= price_volume[period-1];
    }

    ptr->progress = progress;

    ptr->state.num = num;
    ptr->state.den = den;

    return TI_OKAY;
}
