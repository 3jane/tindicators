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
#include "../utils/log.h"

#include <new>

int ti_pfe_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    const TI_REAL ema_period = options[1];

    return period;
}

/* Name: Polarized Fractal Effeciency
 * Source: Robert W. Colby. The Encyclopedia of Technical Market Indicators, 2002, p. 520
 */

int ti_pfe(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *real = inputs[0];
    const int period = options[0];
    const int ema_period = options[1];
    TI_REAL *pfe = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (ema_period < 1) { return TI_INVALID_OPTION; }
    if (size <= ti_pfe_start(options)) { return TI_OKAY; }

    #define SIGN(expr) ((expr) > 0 ? 1. : -1.)
    #define EMA_NEXT(val) (((val) - ema) * per + ema);
    const TI_REAL per = 2. / (ema_period + 1.);

    TI_REAL sum = 0;
    TI_REAL ema;

    // ti_buffer *denom = ti_buffer_new(period);
    ringbuf<0> denom(period);

    int i = 1;
    for (; i < period; ++i, step(denom)) {
        denom = sqrt(pow(real[i] - real[i-1], 2) + 1.);
        sum += denom;
    }
    for (; i < period+1; ++i, step(denom)) {
        denom = sqrt(pow(real[i] - real[i-1], 2) + 1.);
        sum += denom;
        TI_REAL numer = SIGN(real[i] - real[i-period]) * 100. * sqrt(pow(real[i] - real[i-period], 2) + 100.);
        ema = numer / sum;
        *pfe++ = ema;
        sum -= denom[period-1];
    }
    for (; i < size; ++i, step(denom)) {
        denom = sqrt(pow(real[i] - real[i-1], 2) + 1.);
        sum += denom;
        TI_REAL numer = SIGN(real[i] - real[i-period]) * 100. * sqrt(pow(real[i] - real[i-period], 2) + 100.);

        ema = EMA_NEXT(numer / sum);
        *pfe++ = ema;

        sum -= denom[period-1];
    }

    assert(pfe - outputs[0] == size - ti_pfe_start(options));
    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

/* Essentially, EMA 5 of
 * sign(c[i] - c[i-period]) * 100 *
 *  sqrt((c[i] - c[i-period])^2 + 100) /
 *  sum(sqrt((c[j] - c[j-1])^2 + 1) for j in range(i-period+1, i+1))
 */

int ti_pfe_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *real = inputs[0];
    const TI_REAL period = options[0];
    const TI_REAL ema_period = options[1];
    TI_REAL *pfe = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (size <= ti_pfe_start(options)) { return TI_OKAY; }

    for (int i = period; i < size; ++i) {
        TI_REAL div = 0.;
        for (int j = i - period + 1; j <= i; ++j) {
            div += sqrt(pow(real[j] - real[j-1], 2) + 1);
        }
        TI_REAL numer = SIGN(real[i] - real[i-(int)period]) * 100. * sqrt(pow(real[i] - real[i-(int)period], 2) + 100.);
        *pfe++ = SIGN(real[i] - real[i-(int)period]) * 100. * sqrt(pow(real[i] - real[i-(int)period], 2) + 100.) / div;
    }
    ti_ema(size - period, outputs, &ema_period, outputs);

    assert(pfe - outputs[0] == size - ti_pfe_start(options));
    return TI_OKAY;
}

struct ti_pfe_stream : ti_stream {
    struct {
        int period;
        int ema_period;
    } options;

    struct {
        TI_REAL ema;
        TI_REAL numer;
        TI_REAL sum;

        ringbuf<0> price;
        ringbuf<0> denom;
    } state;

    struct {
        TI_REAL per;
    } constants;
};

int ti_pfe_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];
    const TI_REAL ema_period = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (ema_period < 1) { return TI_INVALID_OPTION; }

    ti_pfe_stream *ptr = new(std::nothrow) ti_pfe_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_PFE_INDEX;
    ptr->progress = -ti_pfe_start(options);

    ptr->options.period = options[0];
    ptr->options.ema_period = options[1];

    ptr->constants.per = 2. / (ema_period + 1.);

    try {
        ptr->state.price.resize(period+1);
        ptr->state.denom.resize(period);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_pfe_stream_free(ti_stream *stream) {
    delete static_cast<ti_pfe_stream*>(stream);
}

int ti_pfe_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_pfe_stream *ptr = static_cast<ti_pfe_stream*>(stream);

    TI_REAL const *real = inputs[0];
    const int period = ptr->options.period;
    const int ema_period = ptr->options.ema_period;
    TI_REAL *pfe = outputs[0];

    TI_REAL ema = ptr->state.ema;
    TI_REAL numer = ptr->state.numer;
    TI_REAL sum = ptr->state.sum;
    // TI_REAL last_removed = ptr->last_removed;


    #define SIGN(expr) ((expr) > 0 ? 1. : -1.)
    #define EMA_NEXT(val) (((val) - ema) * per + ema);
    const TI_REAL per = ptr->constants.per;

    auto &price = ptr->state.price;
    auto &denom = ptr->state.denom;

    int progress = ptr->progress;

    int i = 0;
    for (; progress < -period + 1 && i < size; ++i, ++progress, step(price, denom)) {
        price = real[i];
    }
    for (; progress < 0 && i < size; ++i, ++progress, step(price, denom)) {
        price = real[i];
        denom = sqrt(pow(price - price[1], 2) + 1.);
        sum += denom;
    }
    for (; progress < 1 && i < size; ++i, ++progress, step(price, denom)) {
        price = real[i];
        denom = sqrt(pow(price - price[1], 2) + 1.);
        sum += denom;
        numer = SIGN(price - price[period]) * 100. * sqrt(pow(price - price[period], 2) + 100.);
        ema = numer / sum;
        *pfe++ = ema;

        sum -= denom[period-1];
    }
    for (; i < size; ++i, ++progress, step(price, denom)) {
        price = real[i];
        denom = sqrt(pow(price - price[1], 2) + 1.);
        sum += denom;
        numer = SIGN(price - price[period]) * 100. * sqrt(pow(price - price[period], 2) + 100.);
        ema = EMA_NEXT(numer / sum);
        *pfe++ = ema;

        sum -= denom[period-1];
    }

    ptr->progress = progress;
    ptr->state.ema = ema;
    ptr->state.numer = numer;
    ptr->state.sum = sum;

    return TI_OKAY;
}
