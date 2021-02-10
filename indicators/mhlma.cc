#include <new>
#include <algorithm>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_mhlma_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    const TI_REAL ma_period = options[1];

    return ma_period-1;
}

int ti_mhlma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int period = options[0];
    const int ma_period = options[1];
    TI_REAL *mhlsma = outputs[0];
    TI_REAL *mhlema = outputs[1];

    if (period < 1 || ma_period < 1) { return TI_INVALID_OPTION; }

    TI_REAL sum = 0;
    TI_REAL ema = 0;

    TI_REAL hh = -std::numeric_limits<TI_REAL>::infinity();
    TI_REAL ll = std::numeric_limits<TI_REAL>::infinity();
    int hh_idx = 0;
    int ll_idx = 0;

    ringbuf<0> mhl(ma_period);

    int i = 0;
    for (; i < 1 && i < size; ++i, step(mhl)) {
        hh = series[i];
        hh_idx = 0;
        ll = series[i];
        ll_idx = 0;

        mhl = (hh + ll) / 2;

        sum += mhl;
        ema = mhl;
    }
    for (; i < period && i < ma_period-1 && i < size; ++i, step(mhl)) {
        if (hh <= series[i]) {
            hh = series[i];
            hh_idx = i;
        }
        if (ll >= series[i]) {
            ll = series[i];
            ll_idx = i;
        }

        mhl = (hh + ll) / 2.;

        sum += mhl;
        ema = (mhl - ema) * 2. / (ma_period + 1) + ema;
    }
    for (; i < ma_period-1 && i < size; ++i, step(mhl)) {
        if (hh_idx == i - period) {
            auto it = std::max_element(series+i-period+1, series+i+1);
            hh = *it;
            hh_idx = it - series;
        } else if (series[i] >= hh) {
            hh = series[i];
            hh_idx = i;
        }
        if (ll_idx == i - period) {
            auto it = std::min_element(series+i-period+1, series+i+1);
            ll = *it;
            ll_idx = it - series;
        } else if (series[i] <= ll) {
            ll = series[i];
            ll_idx = i;
        }

        mhl = (hh + ll) / 2;

        sum += mhl;
        ema = (mhl - ema) * 2. / (ma_period + 1) + ema;
    }
    for (; i < size; ++i, step(mhl)) {
        if (hh_idx == i - period) {
            auto it = std::max_element(series+i-period+1, series+i+1);
            hh = *it;
            hh_idx = it - series;
        } else if (series[i] >= hh) {
            hh = series[i];
            hh_idx = i;
        }
        if (ll_idx == i - period) {
            auto it = std::min_element(series+i-period+1, series+i+1);
            ll = *it;
            ll_idx = it - series;
        } else if (series[i] <= ll) {
            ll = series[i];
            ll_idx = i;
        }

        mhl = (hh + ll) / 2.;

        sum += mhl;
        ema = (mhl - ema) * 2. / (ma_period + 1) + ema;

        *mhlsma++ = sum / ma_period;
        *mhlema++ = ema;

        sum -= mhl[ma_period-1];
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_mhlma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const TI_REAL period = options[0];
    const TI_REAL ma_period = options[1];
    TI_REAL *mhlsma = outputs[0];
    TI_REAL *mhlema = outputs[1];

    if (period < 1 || ma_period < 1) { return TI_INVALID_OPTION; }

    TI_REAL *mhl = new TI_REAL[size];

    for (int i = 0; i < size; ++i) {
        TI_REAL hh = *std::max_element(std::max<TI_REAL const*>(series+i-(int)period+1, series), series+i+1);
        TI_REAL ll = *std::min_element(std::max<TI_REAL const*>(series+i-(int)period+1, series), series+i+1);
        mhl[i] = (hh + ll) / 2.;
    }

    TI_REAL *ema_buf = new TI_REAL[size];

    ti_sma(size, &mhl, &ma_period, &mhlsma);

    ti_ema(size, &mhl, &ma_period, &ema_buf);
    for (int i = ma_period-1; i < size; ++i) {
        mhlema[i-(int)ma_period+1] = ema_buf[i];
    }

    delete[] mhl;
    delete[] ema_buf;

    return TI_OKAY;
}

struct ti_mhlma_stream : ti_stream {

    struct {
        int period;
        int ma_period;
    } options;

    struct {
        TI_REAL sum = 0;
        TI_REAL ema = 0;

        TI_REAL hh = -std::numeric_limits<TI_REAL>::infinity();
        TI_REAL ll = std::numeric_limits<TI_REAL>::infinity();
        int hh_idx = 0;
        int ll_idx = 0;

        ringbuf<0> mhl;
        ringbuf<0> price;
    } state;

    struct {

    } constants;
};

int ti_mhlma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];
    const int ma_period = options[1];

    if (period < 1 || ma_period < 1) { return TI_INVALID_OPTION; }

    ti_mhlma_stream *ptr = new(std::nothrow) ti_mhlma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_MHLMA_INDEX;
    ptr->progress = -ti_mhlma_start(options);

    ptr->options.period = period;
    ptr->options.ma_period = ma_period;

    ptr->state.mhl.resize(ma_period);
    ptr->state.price.resize(period);

    return TI_OKAY;
}

void ti_mhlma_stream_free(ti_stream *stream) {
    delete static_cast<ti_mhlma_stream*>(stream);
}

int ti_mhlma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_mhlma_stream *ptr = static_cast<ti_mhlma_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *mhlsma = outputs[0];
    TI_REAL *mhlema = outputs[1];
    int progress = ptr->progress;
    const int period = ptr->options.period;
    const int ma_period = ptr->options.ma_period;

    TI_REAL sum = ptr->state.sum;
    TI_REAL ema = ptr->state.ema;
    TI_REAL hh = ptr->state.hh;
    TI_REAL ll = ptr->state.ll;
    int hh_idx = ptr->state.hh_idx;
    int ll_idx = ptr->state.ll_idx;
    auto &mhl = ptr->state.mhl;
    auto &price = ptr->state.price;

    int i = 0;
    for (; progress < -ma_period+2 && i < size; ++i, ++progress, step(mhl, price)) {
        price = series[i];

        hh = series[i];
        hh_idx = progress;
        ll = series[i];
        ll_idx = progress;

        mhl = (hh + ll) / 2;

        sum += mhl;
        ema = mhl;
    }
    for (; progress < -ma_period+1 + period && progress < 0 && i < size; ++i, ++progress, step(mhl, price)) {
        price = series[i];

        if (hh <= series[i]) {
            hh = series[i];
            hh_idx = progress;
        }
        if (ll >= series[i]) {
            ll = series[i];
            ll_idx = progress;
        }

        mhl = (hh + ll) / 2.;

        sum += mhl;
        ema = (mhl - ema) * 2. / (ma_period + 1) + ema;
    }
    for (; progress < 0 && i < size; ++i, ++progress, step(mhl, price)) {
        price = series[i];

        if (hh_idx == progress - period) {
            auto it = std::max_element(price.phbegin(), price.phend());
            hh = *it;
            hh_idx = progress - price.iterator_to_age(it);
        } else if (series[i] >= hh) {
            hh = series[i];
            hh_idx = progress;
        }
        if (ll_idx == progress - period) {
            auto it = std::min_element(price.phbegin(), price.phend());
            ll = *it;
            ll_idx = progress - price.iterator_to_age(it);
        } else if (series[i] <= ll) {
            ll = series[i];
            ll_idx = progress;
        }

        mhl = (hh + ll) / 2;

        sum += mhl;
        ema = (mhl - ema) * 2. / (ma_period + 1) + ema;
    }
    for (; i < size; ++i, ++progress, step(mhl, price)) {
        price = series[i];

        if (hh_idx == progress - period) {
            auto it = std::max_element(price.phbegin(), price.phend());
            hh = *it;
            hh_idx = progress - price.iterator_to_age(it);
        } else if (series[i] >= hh) {
            hh = series[i];
            hh_idx = progress;
        }
        if (ll_idx == progress - period) {
            auto it = std::min_element(price.phbegin(), price.phend());
            ll = *it;
            ll_idx = progress - price.iterator_to_age(it);
        } else if (series[i] <= ll) {
            ll = series[i];
            ll_idx = progress;
        }

        mhl = (hh + ll) / 2.;

        sum += mhl;
        ema = (mhl - ema) * 2. / (ma_period + 1) + ema;

        *mhlsma++ = sum / ma_period;
        *mhlema++ = ema;

        sum -= mhl[ma_period-1];
    }

    ptr->progress = progress;
    ptr->state.sum = sum;
    ptr->state.ema = ema;
    ptr->state.hh = hh;
    ptr->state.ll = ll;
    ptr->state.hh_idx = hh_idx;
    ptr->state.ll_idx = ll_idx;

    return TI_OKAY;
}
