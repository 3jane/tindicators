#include "../indicators.h"
#include "../utils/localbuffer.h"
#include "../utils/log.h"

#include <new>

int ti_lf_start(TI_REAL const *options) {
    TI_REAL gamma = options[0];

    return 0;
}

int ti_lf(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    TI_REAL gamma = options[0];
    TI_REAL *lf = outputs[0];

    if (gamma < 0) { return TI_INVALID_OPTION; }

    TI_REAL L0 = 0.;
    TI_REAL L1 = 0.;
    TI_REAL L2 = 0.;
    TI_REAL L3 = 0.;

    int i = 0;
    for (; i < size; ++i) {
        TI_REAL L0_new = (1. - gamma) * real[i] + gamma*L0;
        TI_REAL L1_new = -gamma*L0_new + L0 + gamma*L1;
        TI_REAL L2_new = -gamma*L1_new + L1 + gamma*L2;
        TI_REAL L3_new = -gamma*L2_new + L2 + gamma*L3;

        L0 = L0_new;
        L1 = L1_new;
        L2 = L2_new;
        L3 = L3_new;

        *lf++ = (L0 + 2.*L1 + 2.*L2 + L3) / 6.;
    }

    return TI_OKAY;
}

struct ti_stream {
    int index;
    int progress;

    struct {
        TI_REAL gamma;
    } options;

    struct {
        TI_REAL L0;
        TI_REAL L1;
        TI_REAL L2;
        TI_REAL L3;
    } state;

    struct {

    } constants;
};

int ti_lf_stream_new(TI_REAL const *options, ti_stream **stream) {
    TI_REAL gamma = options[0];

    if (gamma < 0) { return TI_INVALID_OPTION; }

    *stream = new(std::nothrow) ti_stream();
    if (!*stream) { return TI_OUT_OF_MEMORY; }

    (*stream)->index = TI_INDICATOR_LF_INDEX;
    (*stream)->progress = -ti_lf_start(options);

    (*stream)->options.gamma = gamma;

    return TI_OKAY;
}

void ti_lf_stream_free(ti_stream *stream) {
    delete stream;
}

int ti_lf_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    TI_REAL *lf = outputs[0];
    int progress = stream->progress;
    TI_REAL gamma = stream->options.gamma;

    TI_REAL L0 = stream->state.L0;
    TI_REAL L1 = stream->state.L1;
    TI_REAL L2 = stream->state.L2;
    TI_REAL L3 = stream->state.L3;

    int i = 0;
    for (; i < size; ++i, ++progress) {
        TI_REAL L0_new = (1. - gamma) * real[i] + gamma*L0;
        TI_REAL L1_new = -gamma*L0_new + L0 + gamma*L1;
        TI_REAL L2_new = -gamma*L1_new + L1 + gamma*L2;
        TI_REAL L3_new = -gamma*L2_new + L2 + gamma*L3;

        L0 = L0_new;
        L1 = L1_new;
        L2 = L2_new;
        L3 = L3_new;

        *lf++ = (L0 + 2.*L1 + 2.*L2 + L3) / 6.;
    }

    stream->progress = progress;
    stream->state.L0 = L0;
    stream->state.L1 = L1;
    stream->state.L2 = L2;
    stream->state.L3 = L3;

    return TI_OKAY;
}