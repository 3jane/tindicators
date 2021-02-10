#include <new>

#include "../indicators.h"
#include "../utils/ringbuf.hh"


int ti_smi_start(TI_REAL const *options) {
    const TI_REAL q_period = options[0];
    const TI_REAL r_period = options[1];
    const TI_REAL s_period = options[2];
    return q_period-1;
}

int ti_smi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    const TI_REAL q_period = options[0];
    const TI_REAL r_period = options[1];
    const TI_REAL s_period = options[2];
    TI_REAL *smi = outputs[0];

    for (int i = 0; i < 3; ++i) { if (options[i] < 1) { return TI_INVALID_OPTION; } }

    int progress = -q_period + 1;

    TI_REAL ema_r_num;
    TI_REAL ema_s_num;
    TI_REAL ema_r_den;
    TI_REAL ema_s_den;
    TI_REAL ll;
    TI_REAL hh;
    int hh_idx;
    int ll_idx;

    TI_REAL var1;

    int i = 0;

    for (; i < size && progress == -q_period + 1; ++i, ++progress) {
        hh = high[i];
        hh_idx = progress;
        ll = low[i];
        ll_idx = progress;
    }
    for (; i < size && progress < 0; ++i, ++progress) {
        if (hh <= high[i]) {
            hh = high[i];
            hh_idx = progress;
        }
        if (ll >= low[i]) {
            ll = low[i];
            ll_idx = progress;
        }
    }
    for (; i < size && progress == 0; ++i, ++progress) {
        if (hh <= high[i]) {
            hh = high[i];
            hh_idx = progress;
        }
        if (ll >= low[i]) {
            ll = low[i];
            ll_idx = progress;
        }

        ema_r_num = ema_s_num = close[i] - 0.5 * (hh + ll);
        ema_r_den = ema_s_den = hh - ll;

        *smi++ = ema_s_den ? 100 * ema_s_num / (0.5 * ema_s_den) : 0;
    }
    for (; i < size; ++i, ++progress) {
        if (hh_idx == progress - q_period) {
            hh = high[i];
            hh_idx = progress;
            for (int j = 1; j < q_period; ++j) {
                var1 = high[i-j];
                if (var1 > hh) {
                    hh = var1;
                    hh_idx = progress-j;
                }
            }
        } else if (hh <= high[i]) {
            hh = high[i];
            hh_idx = progress;
        }
        if (ll_idx == progress - q_period) {
            ll = low[i];
            ll_idx = progress;
            for (int j = 1; j < q_period; ++j) {
                var1 = low[i-j];
                if (var1 < ll) {
                    ll = var1;
                    ll_idx = progress-j;
                }
            }
        } else if (ll >= low[i]) {
            ll = low[i];
            ll_idx = progress;
        }

        ema_r_num = ((close[i] - 0.5 * (hh + ll)) - ema_r_num) * (2. / (1. + r_period)) + ema_r_num;
        ema_s_num = (ema_r_num - ema_s_num) * (2. / (1. + s_period)) + ema_s_num;

        ema_r_den = ((hh - ll) - ema_r_den) * (2. / (1. + r_period)) + ema_r_den;
        ema_s_den = (ema_r_den - ema_s_den) * (2. / (1. + s_period)) + ema_s_den;

        *smi++ = ema_s_den ? 100 * ema_s_num / (0.5 * ema_s_den) : 0;
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int ti_smi_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    const TI_REAL q_period = options[0];
    const TI_REAL r_period = options[1];
    const TI_REAL s_period = options[2];
    TI_REAL *smi = outputs[0];

    for (int i = 0; i < 3; ++i) { if (options[i] < 1) { return TI_INVALID_OPTION; } }

    int outsize = size - ti_max_start(&q_period);

    TI_REAL *max = (TI_REAL*)malloc(sizeof(TI_REAL) * outsize);
    TI_REAL *min = (TI_REAL*)malloc(sizeof(TI_REAL) * outsize);

    ti_max(size, &high, &q_period, &max);
    ti_min(size, &low, &q_period, &min);

    TI_REAL *num = (TI_REAL*)malloc(sizeof(TI_REAL) * outsize);
    TI_REAL *den = (TI_REAL*)malloc(sizeof(TI_REAL) * outsize);

    for (int i = 0; i < outsize; ++i) {
        num[i] = close[size-outsize + i] - 0.5 * (max[i] + min[i]);
        den[i] = max[i] - min[i];
    }

    ti_ema(outsize, &num, &r_period, &num);
    ti_ema(outsize, &num, &s_period, &num);

    ti_ema(outsize, &den, &r_period, &den);
    ti_ema(outsize, &den, &s_period, &den);

    for (int i = 0; i < outsize; ++i) {
        smi[i] = 100. * num[i] / (0.5 * den[i]);
    }

    free(max);
    free(min);
    free(num);
    free(den);

    return TI_OKAY;
}

struct ti_smi_stream : ti_stream {
    struct {
        int q_period;
        int r_period;
        int s_period;
    } options;

    struct {
        TI_REAL ema_r_num;
        TI_REAL ema_s_num;
        TI_REAL ema_r_den;
        TI_REAL ema_s_den;
        TI_REAL ll = std::numeric_limits<TI_REAL>::infinity();
        TI_REAL hh = -std::numeric_limits<TI_REAL>::infinity();
        int ll_idx = 0;
        int hh_idx = 0;

        ringbuf<0> price_low;
        ringbuf<0> price_high;
    } state;

    struct {

    } constants;
};

int ti_smi_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL q_period = options[0];
    const TI_REAL r_period = options[1];
    const TI_REAL s_period = options[2];

    for (int i = 0; i < 3; ++i) { if (options[i] < 1) { return TI_INVALID_OPTION; } }

    ti_smi_stream *ptr = new ti_smi_stream;
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_SMI_INDEX;
    ptr->progress = -ti_smi_start(options);

    ptr->options.q_period = q_period;
    ptr->options.r_period = r_period;
    ptr->options.s_period = s_period;

    try {
        ptr->state.price_low.resize(q_period);
        ptr->state.price_high.resize(q_period);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}


void ti_smi_stream_free(ti_stream *stream) {
    delete static_cast<ti_smi_stream*>(stream);
}

int ti_smi_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    TI_REAL *smi = outputs[0];

    ti_smi_stream *ptr = static_cast<ti_smi_stream*>(stream);

    const int q_period = ptr->options.q_period;
    const int r_period = ptr->options.r_period;
    const int s_period = ptr->options.s_period;

    int progress = ptr->progress;

    TI_REAL ema_r_num = ptr->state.ema_r_num;
    TI_REAL ema_s_num = ptr->state.ema_s_num;
    TI_REAL ema_r_den = ptr->state.ema_r_den;
    TI_REAL ema_s_den = ptr->state.ema_s_den;
    TI_REAL ll = ptr->state.ll;
    TI_REAL hh = ptr->state.hh;
    int hh_idx = ptr->state.hh_idx;
    int ll_idx = ptr->state.ll_idx;

    auto &price_low = ptr->state.price_low;
    auto &price_high = ptr->state.price_high;

    TI_REAL var1;

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress, step(price_high, price_low)) {
        price_low = low[i];
        price_high = high[i];

        if (hh_idx == progress - q_period) {
            auto it = price_high.find_max(q_period);
            hh = *it;
            hh_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= hh) {
            hh = high[i];
            hh_idx = progress;
        }
        if (ll_idx == progress - q_period) {
            auto it = price_low.find_min(q_period);
            ll = *it;
            ll_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= ll) {
            ll = low[i];
            ll_idx = progress;
        }
    }
    for (; i < size && progress == 0; ++i, ++progress, step(price_high, price_low)) {
        price_low = low[i];
        price_high = high[i];

        if (hh_idx == progress - q_period) {
            auto it = price_high.find_max(q_period);
            hh = *it;
            hh_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= hh) {
            hh = high[i];
            hh_idx = progress;
        }
        if (ll_idx == progress - q_period) {
            auto it = price_low.find_min(q_period);
            ll = *it;
            ll_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= ll) {
            ll = low[i];
            ll_idx = progress;
        }

        ema_r_num = ema_s_num = close[i] - 0.5 * (hh + ll);
        ema_r_den = ema_s_den = hh - ll;

        *smi++ = ema_s_den ? 100 * ema_s_num / (0.5 * ema_s_den) : 0;
    }
    for (; i < size; ++i, ++progress, step(price_high, price_low)) {
        price_low = low[i];
        price_high = high[i];

        if (hh_idx == progress - q_period) {
            auto it = price_high.find_max(q_period);
            hh = *it;
            hh_idx = progress - price_high.iterator_to_age(it);
        } else if (high[i] >= hh) {
            hh = high[i];
            hh_idx = progress;
        }
        if (ll_idx == progress - q_period) {
            auto it = price_low.find_min(q_period);
            ll = *it;
            ll_idx = progress - price_low.iterator_to_age(it);
        } else if (low[i] <= ll) {
            ll = low[i];
            ll_idx = progress;
        }

        ema_r_num = ((close[i] - 0.5 * (hh + ll)) - ema_r_num) * (2. / (1. + r_period)) + ema_r_num;
        ema_s_num = (ema_r_num - ema_s_num) * (2. / (1. + s_period)) + ema_s_num;

        ema_r_den = ((hh - ll) - ema_r_den) * (2. / (1. + r_period)) + ema_r_den;
        ema_s_den = (ema_r_den - ema_s_den) * (2. / (1. + s_period)) + ema_s_den;

        *smi++ = ema_s_den ? 100. * ema_s_num / (0.5 * ema_s_den) : 0;
    }

    ptr->progress = progress;

    ptr->state.ema_r_num = ema_r_num;
    ptr->state.ema_s_num = ema_s_num;
    ptr->state.ema_r_den = ema_r_den;
    ptr->state.ema_s_den = ema_s_den;

    ptr->state.ll = ll;
    ptr->state.hh = hh;
    ptr->state.hh_idx = hh_idx;
    ptr->state.ll_idx = ll_idx;

    return TI_OKAY;
}
