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
#include "../utils/minmax.h"

#include <new>
#include <algorithm>

int ti_mama_start(TI_REAL const *options) {
    return 6;
}

int ti_mama(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *series = inputs[0];
    TI_REAL *out_mama = outputs[0];
    TI_REAL *out_fama = outputs[1];

    const TI_REAL fastlimit = options[0];
    const TI_REAL slowlimit = options[1];

    if (!(0 <= fastlimit && fastlimit <= 1)) { return TI_INVALID_OPTION; }
    if (!(0 <= slowlimit && slowlimit <= 1)) { return TI_INVALID_OPTION; }

    ringbuf<7> smooth;
    ringbuf<7> detrender;
    ringbuf<7> I1;
    ringbuf<7> Q1;
    ringbuf<2> I2;
    ringbuf<2> Q2;
    ringbuf<2> Re;
    ringbuf<2> Im;
    ringbuf<2> period;
    ringbuf<2> smoothperiod;
    ringbuf<2> phase;
    ringbuf<2> mama;
    ringbuf<2> fama;

    for (int i = 6; i < size; ++i, step(smooth,detrender,I1,Q1,I2,Q2,Re,Im,period,smoothperiod,phase,mama,fama)) {
        smooth = (4 * series[i] + 3 * series[i-1] + 2 * series[i-2] + series[i-3]) / 10.;
        detrender = (.0962*smooth + .5769*smooth[2] - .5769*smooth[4] - .0962*smooth[6]) * (.075*period[1] + .54);

        Q1 = (.0962*detrender + .5769*detrender[2] - .5769*detrender[4] - .0962*detrender[6]) * (.075*period[1] + .54);
        I1 = detrender[3];

        const TI_REAL jI = (.0962*I1 + .5769*I1[2] - .5769*I1[4] - .0962*I1[6]) * (.075 * period[1] + 0.54);
        const TI_REAL jQ = (.0962*Q1 + .5769*Q1[2] - .5769*Q1[4] - .0962*Q1[6]) * (.075 * period[1] + 0.54);

        I2 = I1 - jQ;
        Q2 = Q1 + jI;

        I2 = .2*I2 + .8*I2[1];
        Q2 = .2*Q2 + .8*Q2[1];

        Re = I2*I2[1] + Q2*Q2[1];
        Im = I2*Q2[1] - Q2*I2[1];

        Re = .2*Re + .8*Re[1];
        Im = .2*Im + .8*Im[1];

        if (Im != 0. && Re != 0.) { period = 360. / atan(Im / Re); }
        if (period > 1.5 * period[1]) { period = 1.5 * period[1]; }
        if (period < 0.67 * period[1]) { period = 0.67 * period[1]; }
        if (period < 6.) { period = 6.; }
        if (period > 50.) { period = 50.; }
        period = .2*period + .8*period[1];
        smoothperiod = .33*period + .67*smoothperiod[1];

        if (I1 != 0.) { phase = atan(Q1 / I1); }
        else { phase = 0; }

        const TI_REAL deltaphase = std::max(1., phase[1] - phase);
        const TI_REAL alpha = std::max(slowlimit, fastlimit / deltaphase);

        mama = alpha * series[i] + (1. - alpha) * mama[1];
        fama = .5 * alpha * mama + (1. - .5 * alpha) * fama[1];

        *out_mama++ = mama;
        *out_fama++ = fama;
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int ti_mama_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *series = inputs[0];
    const TI_REAL fastlimit = options[0];
    const TI_REAL slowlimit = options[1];
    TI_REAL *mama = outputs[0];
    TI_REAL *fama = outputs[1];

    for (int i = 0; i < 2; ++i) {
        if (options[i] < 0. || options[i] > 1.) {
            return TI_INVALID_OPTION;
        }
    }

    // Straightforward translation of the original definition in EasyLanguage
    TI_REAL const *price = series;

    TI_REAL *smooth = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *detrender = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *I1 = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *Q1 = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *jI = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *jQ = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *I2 = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *Q2 = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *Re = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *Im = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *period = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *smoothperiod = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *phase = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *deltaphase = (TI_REAL*)calloc(size, sizeof(TI_REAL));
    TI_REAL *alpha = (TI_REAL*)calloc(size, sizeof(TI_REAL));

    for (int i = 6; i < size; ++i) {
        smooth[i] = (4 * price[i] + 3 * price[i-1] + 2 * price[i-2] + price[i-3]) / 10.;
        detrender[i] = (.0962*smooth[i] + .5769*smooth[i-2] - .5769*smooth[i-4] - .0962*smooth[i-6]) * (.075*period[i-1] + .54);

        Q1[i] = (.0962*detrender[i] + .5769*detrender[i-2] - .5769*detrender[i-4] - .0962*detrender[i-6]) * (.075*period[i-1] + .54);
        I1[i] = detrender[i-3];

        jI[i] = (.0962*I1[i] + .5769*I1[i-2] - .5769*I1[i-4] - .0962*I1[i-6]) * (.075 * period[i-1] + .54);
        jQ[i] = (.0962*Q1[i] + .5769*Q1[i-2] - .5769*Q1[i-4] - .0962*Q1[i-6]) * (.075 * period[i-1] + .54);

        I2[i] = I1[i] - jQ[i];
        Q2[i] = Q1[i] + jI[i];

        I2[i] = .2*I2[i] + .8*I2[i-1];
        Q2[i] = .2*Q2[i] + .8*Q2[i-1];

        Re[i] = I2[i] * I2[i-1] + Q2[i] * Q2[i-1];
        Im[i] = I2[i] * Q2[i-1] - Q2[i] * I2[i-1];
        Re[i] = .2*Re[i] + .8*Re[i-1];
        Im[i] = .2*Im[i] + .8*Im[i-1];

        if (Im[i] != 0. && Re[i] != 0.) {
            period[i] = 360. / atan(Im[i] / Re[i]);
        }
        if (period[i] > 1.5 * period[i-1]) {
            period[i] = 1.5 * period[i-1];
        }
        if (period[i] < .67 * period[i-1]) {
            period[i] = .67 * period[i-1];
        }
        if (period[i] < 6.) {
            period[i] = 6.;
        }
        if (period[i] > 50.) {
            period[i] = 50.;
        }
        period[i] = .2 * period[i] + .8 * period[i-1];
        smoothperiod[i] = .33 * period[i] + .67 * smoothperiod[i-1];
        if (I1[i] != 0.) {
            phase[i] = atan(Q1[i] / I1[i]);
        }
        deltaphase[i] = phase[i-1] - phase[i];
        if (deltaphase[i] < 1.) {
            deltaphase[i] = 1.;
        }
        alpha[i] = fastlimit / deltaphase[i];
        if (alpha[i] < slowlimit) { alpha[i] = slowlimit; }
        *mama = alpha[i] * price[i] + (1. - alpha[i]) * (i > 6 ? mama[-1] : 0);
        ++mama;
        *fama = .5 * alpha[i] * mama[-1] + (1. - .5 * alpha[i]) * (i > 6 ? fama[-1] : 0);
        ++fama;
    }

    free(smooth);
    free(detrender);
    free(I1);
    free(Q1);
    free(jI);
    free(jQ);
    free(I2);
    free(Q2);
    free(Re);
    free(Im);
    free(period);
    free(smoothperiod);
    free(phase);
    free(deltaphase);
    free(alpha);

    return TI_OKAY;
}

struct ti_mama_stream : ti_stream {
    struct {
        TI_REAL fastlimit;
        TI_REAL slowlimit;
    } options;

    struct {
        ringbuf<4> price;
        ringbuf<7> smooth;
        ringbuf<7> detrender;
        ringbuf<7> I1;
        ringbuf<7> Q1;
        ringbuf<2> I2;
        ringbuf<2> Q2;
        ringbuf<2> Re;
        ringbuf<2> Im;
        ringbuf<2> period;
        ringbuf<2> smoothperiod;
        ringbuf<2> phase;
        ringbuf<2> mama;
        ringbuf<2> fama;
    } state;

    struct {

    } constants;
};

int ti_mama_stream_new(TI_REAL const *options, ti_stream **stream) {
    TI_REAL fastlimit = options[0];
    TI_REAL slowlimit = options[1];

    if (!(0 <= fastlimit && fastlimit <= 1)) { return TI_INVALID_OPTION; }
    if (!(0 <= slowlimit && slowlimit <= 1)) { return TI_INVALID_OPTION; }

    ti_mama_stream *ptr = new(std::nothrow) ti_mama_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_MAMA_INDEX;
    ptr->progress = -ti_mama_start(options);

    ptr->options.fastlimit = fastlimit;
    ptr->options.slowlimit = slowlimit;

    return TI_OKAY;
}

void ti_mama_stream_free(ti_stream *stream) {
    delete static_cast<ti_mama_stream*>(stream);
}

int ti_mama_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_mama_stream *ptr = static_cast<ti_mama_stream*>(stream);
    int progress = ptr->progress;

    TI_REAL const *series = inputs[0];
    const TI_REAL fastlimit = ptr->options.fastlimit;
    const TI_REAL slowlimit = ptr->options.slowlimit;
    TI_REAL *out_mama = outputs[0];
    TI_REAL *out_fama = outputs[1];

    auto &price = ptr->state.price;
    auto &smooth = ptr->state.smooth;
    auto &detrender = ptr->state.detrender;
    auto &I1 = ptr->state.I1;
    auto &Q1 = ptr->state.Q1;
    auto &I2 = ptr->state.I2;
    auto &Q2 = ptr->state.Q2;
    auto &Re = ptr->state.Re;
    auto &Im = ptr->state.Im;
    auto &period = ptr->state.period;
    auto &smoothperiod = ptr->state.smoothperiod;
    auto &phase = ptr->state.phase;
    auto &mama = ptr->state.mama;
    auto &fama = ptr->state.fama;

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress, step(price,smooth,detrender,I1,Q1,I2,Q2,Re,Im,period,smoothperiod,phase,mama,fama)) {
        price = series[i];
    }
    for (i; i < size; ++i, ++progress, step(price,smooth,detrender,I1,Q1,I2,Q2,Re,Im,period,smoothperiod,phase,mama,fama)) {
        price = series[i];

        smooth = (4*price + 3*price[1] + 2*price[2] + price[3]) / 10.;
        detrender = (.0962*smooth + .5769*smooth[2] - .5769*smooth[4] - .0962*smooth[6]) * (.075*period[1] + .54);

        Q1 = (.0962*detrender + .5769*detrender[2] - .5769*detrender[4] - .0962*detrender[6]) * (.075*period[1] + .54);
        I1 = detrender[3];

        const TI_REAL jI = (.0962*I1 + .5769*I1[2] - .5769*I1[4] - .0962*I1[6]) * (.075 * period[1] + 0.54);
        const TI_REAL jQ = (.0962*Q1 + .5769*Q1[2] - .5769*Q1[4] - .0962*Q1[6]) * (.075 * period[1] + 0.54);

        I2 = .2*(I1 - jQ) + .8*I2[1];
        Q2 = .2*(Q1 + jI) + .8*Q2[1];

        Re = .2*(I2*I2[1] + Q2*Q2[1]) + .8*Re[1];
        Im = .2*(I2*Q2[1] - Q2*I2[1]) + .8*Im[1];

        if (Im != 0. && Re != 0.) { period = 360. / atan(Im / Re); }
        if (period > 1.5 * period[1]) { period = 1.5 * period[1]; }
        if (period < 0.67 * period[1]) { period = 0.67 * period[1]; }
        if (period < 6.) { period = 6.; }
        if (period > 50.) { period = 50.; }
        period = .2*period + .8*period[1];
        smoothperiod = .33*period + .67*smoothperiod[1];

        if (I1 != 0.) { phase = atan(Q1 / I1); }
        else { phase = 0; }

        const TI_REAL deltaphase = std::max(1., phase[1] - phase);
        const TI_REAL alpha = std::max(slowlimit, fastlimit / deltaphase);

        mama = alpha * price + (1. - alpha) * mama[1];
        fama = .5 * alpha * mama + (1. - .5 * alpha) * fama[1];

        *out_mama++ = mama;
        *out_fama++ = fama;
    }

    ptr->progress = progress;

    return TI_OKAY;
}
