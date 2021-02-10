#include <new>
#include <utility>

#include "../indicators.h"
#include "../utils/localbuffer.h"
#include "../utils/log.h"

int ti_rema_start(TI_REAL const *options) {
    TI_REAL period = options[0];
    TI_REAL lambda = options[1];

    return 0;
}

int ti_rema(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    TI_REAL period = options[0];
    TI_REAL lambda = options[1];
    TI_REAL *rema = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (lambda < 0) { return TI_INVALID_OPTION; }

    TI_REAL alpha = 2. / (1. + period);
    TI_REAL rema_val;
    TI_REAL rema_val1;

    int i = 0;
    for (; i < size && i == 0; ++i) {
        rema_val = series[i];
        *rema++ = rema_val;
    }
    for (; i < size && i == 1; ++i) {
        rema_val1 = std::exchange(rema_val, (series[i] - rema_val) * alpha + rema_val);
        *rema++ = rema_val;
    }
    for (; i < size; ++i) {
        rema_val1 = std::exchange(
            rema_val,
            (rema_val*(1. + 2.*lambda) + alpha*(series[i] - rema_val) - lambda*rema_val1) / (1. + lambda)
        );
        *rema++ = rema_val;
    }

    return TI_OKAY;
}

struct ti_rema_stream : ti_stream {

    struct {
        TI_REAL period;
        TI_REAL lambda;
    } options;

    struct {
        TI_REAL rema_val;
        TI_REAL rema_val1;
    } state;

    struct {
        TI_REAL alpha;
    } constants;
};

int ti_rema_stream_new(TI_REAL const *options, ti_stream **stream) {
    TI_REAL period = options[0];
    TI_REAL lambda = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (lambda < 0) { return TI_INVALID_OPTION; }

    ti_rema_stream *ptr = new(std::nothrow) ti_rema_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_REMA_INDEX;
    ptr->progress = -ti_rema_start(options);

    ptr->options.period = period;
    ptr->options.lambda = lambda;

    ptr->constants.alpha = 2. / (1. + period);

    return TI_OKAY;
}

void ti_rema_stream_free(ti_stream *stream) {
    delete static_cast<ti_rema_stream*>(stream);
}

int ti_rema_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_rema_stream *ptr = static_cast<ti_rema_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *rema = outputs[0];
    int progress = ptr->progress;
    TI_REAL period = ptr->options.period;
    TI_REAL lambda = ptr->options.lambda;
    TI_REAL alpha = ptr->constants.alpha;

    TI_REAL rema_val = ptr->state.rema_val;
    TI_REAL rema_val1 = ptr->state.rema_val1;

    int i = 0;
    for (; i < size && progress == 0; ++i, ++progress) {
        rema_val = series[i];
        *rema++ = rema_val;
    }
    for (; i < size && progress == 1; ++i, ++progress) {
        rema_val1 = std::exchange(rema_val, (series[i] - rema_val) * alpha + rema_val);
        *rema++ = rema_val;
    }
    for (; i < size; ++i, ++progress) {
        rema_val1 = std::exchange(
            rema_val,
            (rema_val*(1. + 2.*lambda) + alpha*(series[i] - rema_val) - lambda*rema_val1) / (1. + lambda)
        );
        *rema++ = rema_val;
    }

    ptr->progress = progress;
    ptr->state.rema_val = rema_val;
    ptr->state.rema_val1 = rema_val1;

    return TI_OKAY;
}
