#include "../indicators.h"
#include "../utils/localbuffer.h"
#include "../utils/log.h"

#include <new>
#include <utility>

int ti_rema_start(TI_REAL const *options) {
    TI_REAL period = options[0];
    TI_REAL lambda = options[1];

    return 0;
}

int ti_rema(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
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
        rema_val = real[i];
        *rema++ = rema_val;
    }
    for (; i < size && i == 1; ++i) {
        rema_val1 = std::exchange(rema_val, (real[i] - rema_val) * alpha + rema_val);
        *rema++ = rema_val;
    }
    for (; i < size; ++i) {
        rema_val1 = std::exchange(
            rema_val,
            (rema_val*(1. + 2.*lambda) + alpha*(real[i] - rema_val) - lambda*rema_val1) / (1. + lambda)
        );
        *rema++ = rema_val;
    }

    return TI_OKAY;
}

struct ti_stream {
    int index;
    int progress;

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

    *stream = new(std::nothrow) ti_stream();
    if (!*stream) { return TI_OUT_OF_MEMORY; }

    (*stream)->index = TI_INDICATOR_REMA_INDEX;
    (*stream)->progress = -ti_rema_start(options);

    (*stream)->options.period = period;
    (*stream)->options.lambda = lambda;

    (*stream)->constants.alpha = 2. / (1. + period);

    return TI_OKAY;
}

void ti_rema_stream_free(ti_stream *stream) {
    delete stream;
}

int ti_rema_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    TI_REAL *rema = outputs[0];
    int progress = stream->progress;
    TI_REAL period = stream->options.period;
    TI_REAL lambda = stream->options.lambda;
    TI_REAL alpha = stream->constants.alpha;

    TI_REAL rema_val = stream->state.rema_val;
    TI_REAL rema_val1 = stream->state.rema_val1;

    int i = 0;
    for (; i < size && progress == 0; ++i, ++progress) {
        rema_val = real[i];
        *rema++ = rema_val;
    }
    for (; i < size && progress == 1; ++i, ++progress) {
        rema_val1 = std::exchange(rema_val, (real[i] - rema_val) * alpha + rema_val);
        *rema++ = rema_val;
    }
    for (; i < size; ++i, ++progress) {
        rema_val1 = std::exchange(
            rema_val,
            (rema_val*(1. + 2.*lambda) + alpha*(real[i] - rema_val) - lambda*rema_val1) / (1. + lambda)
        );
        *rema++ = rema_val;
    }

    stream->progress = progress;
    stream->state.rema_val = rema_val;
    stream->state.rema_val1 = rema_val1;

    return TI_OKAY;
}