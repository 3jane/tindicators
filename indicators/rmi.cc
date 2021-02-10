#include <new>

#include "../indicators.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_rmi_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    const TI_REAL lookback_period = options[1];

    return lookback_period;
}


int ti_rmi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *series = inputs[0];
    const TI_REAL period = options[0];
    const TI_REAL lookback_period = options[1];
    TI_REAL *rmi = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (lookback_period < 1) { return TI_INVALID_OPTION; }
    if (size < lookback_period + 1) { return TI_OKAY; }

    TI_REAL gains_ema;
    TI_REAL losses_ema;

    int i = lookback_period;
    {
        gains_ema = MAX(0, series[i] - series[i-(int)lookback_period]);
        losses_ema = MAX(0, series[i-(int)lookback_period] - series[i]);
        ++i;
    }
    {
        *rmi++ = gains_ema ? gains_ema / (gains_ema + losses_ema) * 100. : 0;
    }
    for (; i < size; ++i) {
        gains_ema = (MAX(0, series[i] - series[i-(int)lookback_period]) - gains_ema) * 2. / (1 + period) + gains_ema;
        losses_ema = (MAX(0, series[i-(int)lookback_period] - series[i]) - losses_ema) * 2. / (1 + period) + losses_ema;
        *rmi++ = gains_ema ? gains_ema / (gains_ema + losses_ema) * 100. : 0;
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int ti_rmi_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *series = inputs[0];
    const TI_REAL period = options[0];
    const TI_REAL lookback_period = options[1];
    TI_REAL *rmi = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (lookback_period < 1) { return TI_INVALID_OPTION; }

    int start = ti_rmi_start(options);
    TI_REAL *gains = (TI_REAL*)malloc(sizeof(TI_REAL) * (size-start));
    TI_REAL *losses = (TI_REAL*)malloc(sizeof(TI_REAL) * (size-start));

    for (int i = lookback_period; i < size; ++i) {
        gains[i-start] = MAX(0, series[i] - series[i-(int)lookback_period]);
        losses[i-start] = MAX(0, series[i-(int)lookback_period] - series[i]);
    }
    ti_ema(size-start, &gains, &period, &gains);
    ti_ema(size-start, &losses, &period, &losses);

    TI_REAL *inputs_[] = {gains, losses};
    ti_add(size-start, inputs_, 0, &losses);
    for (int i = 0; i < size-start; ++i) {
        *rmi++ = gains[i] / losses[i] * 100.;
    }

    free(gains);
    free(losses);

    return TI_OKAY;
}

struct ti_rmi_stream : ti_stream {
    struct {
        TI_REAL period;
        TI_REAL lookback_period;
    } options;

    struct {
        TI_REAL gains_ema;
        TI_REAL losses_ema;
        ringbuf<0> price;
    } state;

    struct {

    } constants;
};

int ti_rmi_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];
    const TI_REAL lookback_period = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (lookback_period < 1) { return TI_INVALID_OPTION; }

    ti_rmi_stream *ptr = new(std::nothrow) ti_rmi_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_RMI_INDEX;
    ptr->progress = -ti_rmi_start(options);

    ptr->options.period = period;
    ptr->options.lookback_period = lookback_period;

    try {
        ptr->state.price.resize(lookback_period + 1);
    } catch (std::bad_alloc& e) {
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}


void ti_rmi_stream_free(ti_stream *stream) {
    delete static_cast<ti_rmi_stream*>(stream);
}

int ti_rmi_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_rmi_stream *ptr = static_cast<ti_rmi_stream*>(stream);

    TI_REAL const *series = inputs[0];
    TI_REAL *rmi = outputs[0];

    int progress = ptr->progress;

    TI_REAL period = ptr->options.period;
    TI_REAL lookback_period = ptr->options.lookback_period;

    TI_REAL gains_ema = ptr->state.gains_ema;
    TI_REAL losses_ema = ptr->state.losses_ema;

    auto &price = ptr->state.price;

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress, step(price)) {
        price = series[i];
    }
    for (; i < size && progress < 1; ++i, ++progress, step(price)) {
        price = series[i];

        gains_ema = std::max(0., series[i] - price[lookback_period]);
        losses_ema = std::max(0., price[lookback_period] - series[i]);

        *rmi++ = gains_ema ? gains_ema / (gains_ema + losses_ema) * 100. : 0;
    }
    for (; i < size; ++i, ++progress, step(price)) {
        price = series[i];

        gains_ema = (std::max(0., series[i] - price[lookback_period]) - gains_ema) * 2. / (period + 1) + gains_ema;
        losses_ema = (std::max(0., price[lookback_period] - series[i]) - losses_ema) * 2. / (period + 1) + losses_ema;

        *rmi++ = gains_ema ? gains_ema / (gains_ema + losses_ema) * 100. : 0;
    }

    ptr->progress = progress;

    ptr->options.period = period;
    ptr->options.lookback_period = lookback_period;

    ptr->state.gains_ema = gains_ema;
    ptr->state.losses_ema = losses_ema;

    return TI_OKAY;
}
