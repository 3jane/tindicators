#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/ringbuf.hh"

struct {
    TI_REAL B0[20] = {0.828427,0.732051,0.618034,0.526602,0.455887,0.400720,0.356896,0.321416,0.292186,0.267730,0.246990,0.229192,0.213760,0.200256,0.188343,0.177759,0.168294,0.159780,0.152082,0.145089};
    TI_REAL A1[20] = {0.171573,0.267949,0.381966,0.473398,0.544113,0.599280,0.643104,0.678584,0.707814,0.732270,0.753010,0.770808,0.786240,0.799744,0.811657,0.822241,0.831706,0.840220,0.847918,0.854911};
} static tables;

int ti_gf1_start(TI_REAL const *options) {
    const int period = options[0];

    return 0;
}

int ti_gf1(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *gf1 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    TI_REAL f = 0;

    const TI_REAL B0 = tables.B0[period/2-1];
    const TI_REAL A1 = tables.A1[period/2-1];

    int i = 0;
    for (; i < size; ++i) {
        f = (B0*real[i] + A1*f) / (B0 + A1);
        *gf1++ = f;
    }

    return TI_OKAY;
}

int DONTOPTIMIZE ti_gf1_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *gf1 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    TI_REAL f = 0;

    const TI_REAL B0 = tables.B0[period/2-1];
    const TI_REAL A1 = tables.A1[period/2-1];

    int i = 0;
    for (; i < size; ++i) {
        f = (B0*real[i] + A1*f) / (B0 + A1);
        *gf1++ = f;
    }

    return TI_OKAY;
}

struct ti_gf1_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        TI_REAL f = 0;
    } state;

    struct {
        TI_REAL B0;
        TI_REAL A1;
        TI_REAL csum_recipr;
    } constants;
};

int ti_gf1_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    ti_gf1_stream *ptr = new(std::nothrow) ti_gf1_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_GF1_INDEX;
    ptr->progress = -ti_gf1_start(options);

    ptr->options.period = period;

    ptr->constants.B0 = tables.B0[period/2-1];
    ptr->constants.A1 = tables.A1[period/2-1];
    ptr->constants.csum_recipr = 1. / (ptr->constants.B0 + ptr->constants.A1);

    return TI_OKAY;
}

void ti_gf1_stream_free(ti_stream *stream) {
    delete static_cast<ti_gf1_stream*>(stream);
}

int ti_gf1_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_gf1_stream *ptr = static_cast<ti_gf1_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *gf1 = outputs[0];
    int progress = ptr->progress;
    int period = ptr->options.period;
    TI_REAL f = ptr->state.f;

    const TI_REAL B0 = ptr->constants.B0;
    const TI_REAL A1 = ptr->constants.A1;
    const TI_REAL csum_recipr = ptr->constants.csum_recipr;

    int i = 0;
    for (; i < size; ++i, ++progress) {
        f = (tables.B0[period/2-1]*real[i] + tables.A1[period/2-1]*f) * csum_recipr;
        *gf1++ = f;
    }

    ptr->progress = progress;
    ptr->state.f = f;

    return TI_OKAY;
}