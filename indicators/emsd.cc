#include <new>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_emsd_start(TI_REAL const *options) {
    const int period = options[0];
    const int ma_period = options[1];

    return period-1;
}

int ti_emsd(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int period = options[0];
    const int ma_period = options[1];
    TI_REAL *emsd = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (ma_period < 1) { return TI_INVALID_OPTION; }

    TI_REAL sum = 0;
    TI_REAL sum2 = 0;
    TI_REAL ema;

    int i = 0;
    for (; i < period-1 && i < size; ++i) {
        sum += series[i];
        sum2 += series[i]*series[i];
    }
    for (; i < period && i < size; ++i) {
        sum += series[i];
        sum2 += series[i]*series[i];

        TI_REAL variance = sum2/period - pow(sum/period, 2);
        ema = variance;
        *emsd++ = sqrt(variance);

        sum -= series[i-period+1];
        sum2 -= series[i-period+1]*series[i-period+1];
    }
    for (; i < size; ++i) {
        sum += series[i];
        sum2 += series[i]*series[i];

        TI_REAL variance = sum2/period - (sum/period)*(sum/period);
        ema = (variance - ema) * 2 / (ma_period + 1) + ema;
        *emsd++ = sqrt(variance);

        sum -= series[i-period+1];
        sum2 -= series[i-period+1]*series[i-period+1];
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_emsd_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const TI_REAL period = options[0];
    const TI_REAL ma_period = options[1];
    TI_REAL *emsd = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (ma_period < 1) { return TI_INVALID_OPTION; }

    TI_REAL sum = 0;
    TI_REAL sum2 = 0;
    TI_REAL ema;

    int i = 0;
    for (; i < period-1 && i < size; ++i) {
        sum += series[i];
        sum2 += series[i]*series[i];
    }
    for (; i < period && i < size; ++i) {
        sum += series[i];
        sum2 += series[i]*series[i];

        TI_REAL variance = sum2/period - pow(sum/period, 2);
        ema = variance;
        *emsd++ = sqrt(variance);

        sum -= series[i-(int)period+1];
        sum2 -= series[i-(int)period+1]*series[i-(int)period+1];
    }
    for (; i < size; ++i) {
        sum += series[i];
        sum2 += series[i]*series[i];

        TI_REAL variance = sum2/period - (sum/period)*(sum/period);
        ema = (variance - ema) * 2 / (ma_period + 1) + ema;
        *emsd++ = sqrt(variance);

        sum -= series[i-(int)period+1];
        sum2 -= series[i-(int)period+1]*series[i-(int)period+1];
    }

    return TI_OKAY;
}

struct ti_emsd_stream : ti_stream {

    struct {
        int period;
        int ma_period;
    } options;

    struct {
        TI_REAL sum = 0;
        TI_REAL sum2 = 0;
        TI_REAL ema;
        ringbuf<0> price;
    } state;

    struct {
        TI_REAL period_reciprocal;
        TI_REAL ma_period_plus1_reciprocal;
    } constants;
};

int ti_emsd_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];
    const int ma_period = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (ma_period < 1) { return TI_INVALID_OPTION; }

    ti_emsd_stream *ptr = new(std::nothrow) ti_emsd_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_EMSD_INDEX;
    ptr->progress = -ti_emsd_start(options);

    ptr->options.period = period;
    ptr->options.ma_period = ma_period;

    ptr->state.price.resize(period);

    ptr->constants.period_reciprocal = 1. / period;
    ptr->constants.ma_period_plus1_reciprocal = 1. / (ma_period + 1.);

    return TI_OKAY;
}

void ti_emsd_stream_free(ti_stream *stream) {
    delete static_cast<ti_emsd_stream*>(stream);
}

int ti_emsd_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_emsd_stream *ptr = static_cast<ti_emsd_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *emsd = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;
    const int ma_period = ptr->options.ma_period;
    const TI_REAL period_reciprocal = ptr->constants.period_reciprocal;
    const TI_REAL ma_period_plus1_reciprocal = ptr->constants.ma_period_plus1_reciprocal;
    TI_REAL sum = ptr->state.sum;
    TI_REAL sum2 = ptr->state.sum2;
    TI_REAL ema = ptr->state.ema;
    auto &price = ptr->state.price;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(price)) {
        price = series[i];

        sum += series[i];
        sum2 += series[i]*series[i];
    }
    for (; progress < 1 && i < size; ++i, ++progress, step(price)) {
        price = series[i];

        sum += series[i];
        sum2 += series[i]*series[i];

        TI_REAL variance = sum2*period_reciprocal - pow(sum*period_reciprocal, 2);
        ema = variance;
        *emsd++ = sqrt(variance);

        sum -= price[period-1];
        sum2 -= price[period-1]*price[period-1];
    }
    for (; i < size; ++i, ++progress, step(price)) {
        price = series[i];

        sum += series[i];
        sum2 += series[i]*series[i];

        TI_REAL variance = sum2*period_reciprocal - (sum*period_reciprocal)*(sum*period_reciprocal);
        ema = (variance - ema) * 2 * ma_period_plus1_reciprocal + ema;
        *emsd++ = sqrt(variance);

        sum -= price[period-1];
        sum2 -= price[period-1]*price[period-1];
    }

    ptr->progress = progress;
    ptr->state.sum = sum;
    ptr->state.sum2 = sum2;
    ptr->state.ema = ema;

    return TI_OKAY;
}
