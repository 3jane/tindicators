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

int ti_rvi_start(TI_REAL const *options) {
    const TI_REAL ema_period = options[0];
    const TI_REAL stddev_period = options[1];
    return stddev_period-1;
}

int ti_rvi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *series = inputs[0];
    const int ema_period = options[0];
    const int stddev_period = options[1];
    TI_REAL *rvi = outputs[0];

    if (ema_period < 1) { return TI_INVALID_OPTION; }
    if (stddev_period < 1) { return TI_INVALID_OPTION; }

    TI_REAL y_sum = 0.;
    TI_REAL xy_sum = 0.;

    const TI_REAL x_sum = stddev_period * (stddev_period + 1) / 2.;
    const TI_REAL xsq_sum = stddev_period * (stddev_period + 1) * (2*stddev_period + 1) / 6.;

    TI_REAL gains_ema = 0.;
    TI_REAL losses_ema = 0.;

    int i = 0;
    for (; i < stddev_period - 1 && i < size; ++i) {
        xy_sum += series[i] * (i + 1);
        y_sum += series[i];
    }
    for (; i < size; ++i) {
        xy_sum += series[i]*stddev_period;
        y_sum += series[i];

        // y = a + bx
        TI_REAL b = (xy_sum / stddev_period - x_sum / stddev_period * y_sum / stddev_period) / (xsq_sum / stddev_period - (x_sum / stddev_period)*(x_sum / stddev_period));
        TI_REAL a = y_sum / stddev_period - b * x_sum / stddev_period;

        TI_REAL above = series[i] - (a + b * stddev_period);
        if (above > 0) {
            gains_ema = (above * above / stddev_period - gains_ema) * 2. / (ema_period + 1) + gains_ema;
        } else {
            losses_ema = (above * above / stddev_period - losses_ema) * 2. / (ema_period + 1) + losses_ema;
        }

        *rvi++ = gains_ema ? gains_ema / (gains_ema + losses_ema) * 100. : 50.;

        xy_sum -= y_sum;
        y_sum -= series[i-stddev_period+1];
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

struct ti_rvi_stream : ti_stream {
    struct {
        int ema_period;
        int stddev_period;
    } options;

    struct {
        TI_REAL y_sum;
        TI_REAL xy_sum;
        TI_REAL gains_ema;
        TI_REAL losses_ema;

        ringbuf<0> price;
    } state;

    struct {
        TI_REAL x_sum;
        TI_REAL xsq_sum;
    } constants;
};

int ti_rvi_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL ema_period = options[0];
    const TI_REAL stddev_period = options[1];
    if (ema_period < 1) { return TI_INVALID_OPTION; }
    if (stddev_period < 1) { return TI_INVALID_OPTION; }

    ti_rvi_stream *ptr = new ti_rvi_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_RVI_INDEX;
    ptr->progress = -ti_rvi_start(options);

    ptr->options.ema_period = ema_period;
    ptr->options.stddev_period = stddev_period;

    ptr->state.y_sum = 0.;
    ptr->state.xy_sum = 0.;
    ptr->state.gains_ema = 0.;
    ptr->state.losses_ema = 0.;

    ptr->constants.x_sum = stddev_period * (stddev_period + 1) / 2.;
    ptr->constants.xsq_sum = stddev_period * (stddev_period + 1) * (2*stddev_period + 1) / 6.;

    try {
        ptr->state.price.resize(stddev_period+1);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_rvi_stream_free(ti_stream *stream) {
    delete static_cast<ti_rvi_stream*>(stream);
}

int ti_rvi_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_rvi_stream *ptr = static_cast<ti_rvi_stream*>(stream);

    TI_REAL const *series = inputs[0];
    TI_REAL *rvi = outputs[0];

    int progress = ptr->progress;

    int ema_period = ptr->options.ema_period;
    int stddev_period = ptr->options.stddev_period;

    TI_REAL y_sum = ptr->state.y_sum;
    TI_REAL xy_sum = ptr->state.xy_sum;
    TI_REAL gains_ema = ptr->state.gains_ema;
    TI_REAL losses_ema = ptr->state.losses_ema;

    TI_REAL x_sum = ptr->constants.x_sum;
    TI_REAL xsq_sum = ptr->constants.xsq_sum;

    auto &price = ptr->state.price;

    int i = 0;
    TI_REAL var1;
    for (; i < size && progress < 0; ++i, ++progress, step(price)) {
        price = series[i];

        xy_sum += series[i] * (progress - (-stddev_period+1) + 1);
        y_sum += series[i];
    }
    for (; i < size; ++i, ++progress, step(price)) {
        price = series[i];
        
        xy_sum += series[i] * stddev_period;
        y_sum += series[i];

        // y = a + bx
        TI_REAL b = (xy_sum / stddev_period - x_sum / stddev_period * y_sum / stddev_period) / (xsq_sum / stddev_period - (x_sum / stddev_period)*(x_sum / stddev_period));
        TI_REAL a = y_sum / stddev_period - b * x_sum / stddev_period;

        TI_REAL above = series[i] - (a + b * stddev_period);
        if (above > 0) {
            gains_ema = (above * above / stddev_period - gains_ema) * 2. / (ema_period + 1) + gains_ema;
        } else {
            losses_ema = (above * above / stddev_period - losses_ema) * 2. / (ema_period + 1) + losses_ema;
        }

        *rvi++ = gains_ema ? gains_ema / (gains_ema + losses_ema) * 100. : 50.;

        xy_sum -= y_sum;
        y_sum -= price[stddev_period-1];
    }

    ptr->progress = progress;

    ptr->state.y_sum = y_sum;
    ptr->state.xy_sum = xy_sum;
    ptr->state.gains_ema = gains_ema;
    ptr->state.losses_ema = losses_ema;

    ptr->constants.x_sum = x_sum;
    ptr->constants.xsq_sum = xsq_sum;

    return TI_OKAY;
}
