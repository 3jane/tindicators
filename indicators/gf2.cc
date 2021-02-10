#include <new>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

struct {
    TI_REAL B0[20] = {0.834615,0.722959,0.578300,0.457577,0.365017,0.295336,0.242632,0.202250,0.170835,0.146017,0.126125,0.109966,0.096680,0.085633,0.076357,0.068496,0.061779,0.055996,0.050984,0.046612};
    TI_REAL A1[20] = {0.172854,0.299460,0.479080,0.647112,0.791668,0.913103,1.014847,1.100556,1.173357,1.235757,1.289719,1.336777,1.378133,1.414738,1.447346,1.476567,1.502894,1.526729,1.548408,1.568205};
    TI_REAL A2[20] = {-0.007470,-0.022419,-0.057379,-0.104688,-0.156684,-0.208439,-0.257479,-0.302806,-0.344192,-0.381774,-0.415844,-0.446743,-0.474813,-0.500371,-0.523703,-0.545063,-0.564672,-0.582726,-0.599392,-0.614817};
} static tables;

int ti_gf2_start(TI_REAL const *options) {
    const int period = options[0];

    return 0;
}

int ti_gf2(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int period = options[0];
    TI_REAL *gf2 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    ringbuf<3> f;

    const int idx = period/2-1;

    const TI_REAL B0 = tables.B0[idx];
    const TI_REAL A1 = tables.A1[idx];
    const TI_REAL A2 = tables.A2[idx];

    const TI_REAL csum_recipr = 1. / (B0 + A1 + A2);

    int i = 0;
    for (; i < size; ++i, step(f)) {
        f = (B0*series[i] + A1*f[1] + A2*f[2]) * csum_recipr;
        *gf2++ = f;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_gf2_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int period = options[0];
    TI_REAL *gf2 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    ringbuf<3> f;

    const int idx = period/2-1;

    const TI_REAL B0 = tables.B0[idx];
    const TI_REAL A1 = tables.A1[idx];
    const TI_REAL A2 = tables.A2[idx];

    const TI_REAL csum_recipr = 1. / (B0 + A1 + A2);

    int i = 0;
    for (; i < size; ++i, step(f)) {
        f = (B0*series[i] + A1*f[1] + A2*f[2]) * csum_recipr;
        *gf2++ = f;
    }

    return TI_OKAY;
}

struct ti_gf2_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        ringbuf<3> f;
    } state;

    struct {
        TI_REAL B0;
        TI_REAL A1;
        TI_REAL A2;
        TI_REAL csum_recipr;
    } constants;
};

int ti_gf2_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    ti_gf2_stream *ptr = new(std::nothrow) ti_gf2_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_GF2_INDEX;
    ptr->progress = -ti_gf2_start(options);

    ptr->options.period = period;

    const int idx = period/2-1;
    ptr->constants.B0 = tables.B0[idx];
    ptr->constants.A1 = tables.A1[idx];
    ptr->constants.A2 = tables.A2[idx];
    ptr->constants.csum_recipr = 1. / (ptr->constants.B0 + ptr->constants.A1 + ptr->constants.A2);

    return TI_OKAY;
}

void ti_gf2_stream_free(ti_stream *stream) {
    delete static_cast<ti_gf2_stream*>(stream);
}

int ti_gf2_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_gf2_stream *ptr = static_cast<ti_gf2_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *gf2 = outputs[0];
    int progress = ptr->progress;
    int period = ptr->options.period;
    auto& f = ptr->state.f;

    const TI_REAL B0 = ptr->constants.B0;
    const TI_REAL A1 = ptr->constants.A1;
    const TI_REAL A2 = ptr->constants.A2;
    const TI_REAL csum_recipr = ptr->constants.csum_recipr;

    int i = 0;
    for (; i < size; ++i, ++progress, step(f)) {
        f = (B0*series[i] + A1*f[1] + A2*f[2]) * csum_recipr;
        *gf2++ = f;
    }

    ptr->progress = progress;

    return TI_OKAY;
}
