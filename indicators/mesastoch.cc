#include "../indicators.h"
#include "../utils/localbuffer.h"
#include "../utils/log.h"
#include "../utils/ringbuf.hh"

#include <deque>
#include <algorithm>
#include <new>

int ti_mesastoch_start(TI_REAL const *options) {
    int period = options[0];
    int max_cycle_considered = options[1];

    return period + 2;
}

/* renamed: length -> period
 * added: const 48 -> max_cycle_considered
*/
#define mcc max_cycle_considered
#define DEG2RAD(x) ((x)*3.141592/180.)

int ti_mesastoch(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *const real = inputs[0];
    int period = options[0];
    int max_cycle_considered = options[1];
    TI_REAL *mesastoch = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (max_cycle_considered < 1) { return TI_INVALID_OPTION; }

    TI_REAL highestC = -std::numeric_limits<TI_REAL>::infinity();
    TI_REAL lowestC = std::numeric_limits<TI_REAL>::infinity();
    int highestC_idx;
    int lowestC_idx;

    ringbuf<0> filt(period);
    ringbuf<3> HP;
    ringbuf<2> stoc;
    ringbuf<3> result;

    const TI_REAL alpha1 = (cos(DEG2RAD(.707*360 / mcc)) + sin(DEG2RAD(.707*360 / mcc)) - 1) / cos(DEG2RAD(.707*360 / mcc));
    const TI_REAL a1 = exp(-1.414*3.141459/10.);
    const TI_REAL b1 = 2*a1*cos(DEG2RAD(1.414*180./10.));
    const TI_REAL c2 = b1;
    const TI_REAL c3 = -a1*a1;
    const TI_REAL c1 = 1 - c2 - c3;

    int i = 2;
    for (; i < ti_mesastoch_start(options) && i < size; ++i, step(HP, filt)) {
        HP = (1 - alpha1 / 2.)*(1 - alpha1 / 2.)*(real[i] - 2*real[i-1] + real[i-2]) +
            2.*(1. - alpha1)*HP[1] - (1 - alpha1) * (1 - alpha1)*HP[2];
        filt = c1 * (HP + HP[1]) / 2. + c2 * filt[1] + c3 * filt[2];

        if (highestC <= filt) {
            highestC = filt;
            highestC_idx = i;
        }
        if (lowestC >= filt) {
            lowestC = filt;
            lowestC_idx = i;
        }
    }

    for (; i < size; ++i, step(HP, filt, stoc, result)) {
        HP = (1 - alpha1 / 2.)*(1 - alpha1 / 2.)*(real[i] - 2*real[i-1] + real[i-2]) +
            2.*(1. - alpha1)*HP[1] - (1 - alpha1) * (1 - alpha1)*HP[2];
        filt = c1 * (HP + HP[1]) / 2. + c2 * filt[1] + c3 * filt[2];

        if (highestC <= filt) {
            highestC_idx = i;
            highestC = filt;
        } else if (highestC_idx == i - period) {
            TI_REAL* it = std::max_element(filt.phbegin(), filt.phend());
            highestC = *it;
            highestC_idx = i - filt.iterator_to_age(it);
        }
        if (lowestC >= filt) {
            lowestC_idx = i;
            lowestC = filt;
        } else if (lowestC_idx == i - period) {
            TI_REAL* it = std::min_element(filt.phbegin(), filt.phend());
            lowestC = *it;
            lowestC_idx = i - filt.iterator_to_age(it);
        }

        stoc = (filt - lowestC) ? (filt - lowestC) / (highestC - lowestC) : 0;
        result = c1 * (stoc + stoc[1]) / 2. + c2 * result[1] + c3 * result[2];

        *mesastoch++ = result;
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int ti_mesastoch_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    int period = options[0];
    int max_cycle_considered = options[1];
    TI_REAL *mesastoch = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (max_cycle_considered < 1) { return TI_INVALID_OPTION; }

    std::deque<TI_REAL> filt(period);
    std::deque<TI_REAL> HP(3);
    std::deque<TI_REAL> stoc(2);
    std::deque<TI_REAL> result(3);
    TI_REAL alpha1;
    TI_REAL a1;
    TI_REAL b1;
    TI_REAL c1;
    TI_REAL c2;
    TI_REAL c3;
    TI_REAL highestC;
    TI_REAL lowestC;

    int i = 2;
    for (; i < ti_mesastoch_start(options); ++i) {
        filt.pop_back(); filt.push_front(0);
        HP.pop_back(); HP.push_front(0);
        stoc.pop_back(); stoc.push_front(0);
        result.pop_back(); result.push_front(0);

        alpha1 = (cos(DEG2RAD(.707*360 / mcc)) + sin(DEG2RAD(.707*360 / mcc)) - 1) / cos(DEG2RAD(.707*360 / mcc));
        HP[0] = (1 - alpha1 / 2.)*(1 - alpha1 / 2.)*(real[i] - 2*real[i-1] + real[i-2]) +
            2.*(1. - alpha1)*HP[1] - (1 - alpha1) * (1 - alpha1)*HP[2];
        a1 = exp(-1.414*3.141459/10.);
        b1 = 2*a1*cos(DEG2RAD(1.414*180./10.));
        c2 = b1;
        c3 = -a1*a1;
        c1 = 1 - c2 - c3;
        filt[0] = c1 * (HP[0] + HP[1]) / 2. + c2 * filt[1] + c3 * filt[2];
    }

    for (; i < size; ++i) {
        filt.pop_back(); filt.push_front(0);
        HP.pop_back(); HP.push_front(0);
        stoc.pop_back(); stoc.push_front(0);
        result.pop_back(); result.push_front(0);

        alpha1 = (cos(DEG2RAD(.707*360 / mcc)) + sin(DEG2RAD(.707*360 / mcc)) - 1) / cos(DEG2RAD(.707*360 / mcc));
        HP[0] = (1 - alpha1 / 2.)*(1 - alpha1 / 2.)*(real[i] - 2*real[i-1] + real[i-2]) +
            2.*(1. - alpha1)*HP[1] - (1 - alpha1) * (1 - alpha1)*HP[2];
        a1 = exp(-1.414*3.141459/10.);
        b1 = 2*a1*cos(DEG2RAD(1.414*180./10.));
        c2 = b1;
        c3 = -a1*a1;
        c1 = 1 - c2 - c3;
        filt[0] = c1 * (HP[0] + HP[1]) / 2. + c2 * filt[1] + c3 * filt[2];

        highestC = *std::max_element(filt.begin(), filt.end());
        lowestC = *std::min_element(filt.begin(), filt.end());

        stoc[0] = (filt[0] - lowestC) / (highestC - lowestC);
        result[0] = c1 * (stoc[0] + stoc[1]) / 2. + c2 * result[1] + c3 * result[2];

        *mesastoch++ = result[0];
    }

    return TI_OKAY;
}

struct ti_mesastoch_stream : ti_stream {
    struct {
        int period;
        int max_cycle_considered;
    } options;

    struct {
        TI_REAL highestC = -std::numeric_limits<TI_REAL>::infinity();
        TI_REAL lowestC = std::numeric_limits<TI_REAL>::infinity();
        int highestC_idx;
        int lowestC_idx;
    } state;

    struct {
        ringbuf<0> filt;
        ringbuf<3> HP;
        ringbuf<2> stoc;
        ringbuf<3> result;
        ringbuf<3> price;
    } store;

    struct {
        TI_REAL alpha1;
        TI_REAL a1;
        TI_REAL b1;
        TI_REAL c1;
        TI_REAL c2;
        TI_REAL c3;
    } constants;
};

int ti_mesastoch_stream_new(TI_REAL const *options, ti_stream **stream) {
    int period = options[0];
    int max_cycle_considered = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (max_cycle_considered < 1) { return TI_INVALID_OPTION; }

    ti_mesastoch_stream *ptr = new(std::nothrow) ti_mesastoch_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_MESASTOCH_INDEX;
    ptr->progress = -ti_mesastoch_start(options);

    ptr->options.period = period;
    ptr->options.max_cycle_considered = max_cycle_considered;

    ptr->constants.alpha1 = (cos(DEG2RAD(.707*360 / mcc)) + sin(DEG2RAD(.707*360 / mcc)) - 1) / cos(DEG2RAD(.707*360 / mcc));
    ptr->constants.a1 = exp(-1.414*3.141459/10.);
    ptr->constants.b1 = 2*(ptr->constants.a1)*cos(DEG2RAD(1.414*180./10.));
    ptr->constants.c2 = ptr->constants.b1;
    ptr->constants.c3 = -(ptr->constants.a1)*(ptr->constants.a1);
    ptr->constants.c1 = 1 - (ptr->constants.c2) - (ptr->constants.c3);

    try {
        ptr->store.filt.resize(period);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }
    
    return TI_OKAY;
}

void ti_mesastoch_stream_free(ti_stream *stream) {
    delete static_cast<ti_mesastoch_stream*>(stream);
}

int ti_mesastoch_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_mesastoch_stream *ptr = static_cast<ti_mesastoch_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *mesastoch = outputs[0];
    int progress = ptr->progress;
    int period = ptr->options.period;
    TI_REAL max_cycle_considered = ptr->options.max_cycle_considered;

    const TI_REAL alpha1 = ptr->constants.alpha1;
    const TI_REAL a1 = ptr->constants.a1;
    const TI_REAL b1 = ptr->constants.b1;
    const TI_REAL c1 = ptr->constants.c1;
    const TI_REAL c2 = ptr->constants.c2;
    const TI_REAL c3 = ptr->constants.c3;
    TI_REAL highestC = ptr->state.highestC;
    TI_REAL lowestC = ptr->state.lowestC;
    TI_REAL highestC_idx = ptr->state.highestC_idx;
    TI_REAL lowestC_idx = ptr->state.lowestC_idx;

    auto& filt = ptr->store.filt;
    auto& HP = ptr->store.HP;
    auto& stoc = ptr->store.stoc;
    auto& result = ptr->store.result;
    auto& price = ptr->store.price;

    int i = 0;
    for (; i < size && progress < -period; ++i, ++progress, step(price)) {
        price = real[i];
    }
    for (; i < size && progress < 0; ++i, ++progress, step(price, HP, filt)) {
        price = real[i];

        HP = (1 - alpha1 / 2.)*(1 - alpha1 / 2.)*(real[i] - 2*price[1] + price[2]) +
            2.*(1. - alpha1)*HP[1] - (1 - alpha1) * (1 - alpha1)*HP[2];

        filt = c1 * (HP + HP[1]) / 2. + c2 * filt[1] + c3 * filt[2];

        if (highestC <= filt) {
            highestC = filt;
            highestC_idx = progress;
        }
        if (lowestC >= filt) {
            lowestC = filt;
            lowestC_idx = progress;
        }
    }
    for (; i < size; ++i, ++progress, step(price, HP, filt, stoc, result)) {
        price = real[i];

        HP = (1 - alpha1 / 2.)*(1 - alpha1 / 2.)*(real[i] - 2*price[1] + price[2]) +
            2.*(1. - alpha1)*HP[1] - (1 - alpha1) * (1 - alpha1)*HP[2];


        filt = c1 * (HP + HP[1]) / 2. + c2 * filt[1] + c3 * filt[2];

        if (highestC <= filt) {
            highestC_idx = progress;
            highestC = filt;
        } else if (highestC_idx == progress - period) {
            TI_REAL* it = filt.find_max(period);
            highestC = *it;
            highestC_idx = progress - filt.iterator_to_age(it);
        }
        if (lowestC >= filt) {
            lowestC_idx = progress;
            lowestC = filt;
        } else if (lowestC_idx == progress - period) {
            TI_REAL* it = filt.find_min(period);
            lowestC = *it;
            lowestC_idx = progress - filt.iterator_to_age(it);
        }

        stoc = (filt - lowestC) ? (filt - lowestC) / (highestC - lowestC) : 0;
        result = c1 * (stoc + stoc[1]) / 2. + c2 * result[1] + c3 * result[2];

        *mesastoch++ = result;
    }

    ptr->progress = progress;
    ptr->state.highestC = highestC;
    ptr->state.lowestC = lowestC;
    ptr->state.highestC_idx = highestC_idx;
    ptr->state.lowestC_idx = lowestC_idx;

    return TI_OKAY;
}
