#include <new>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

struct {
    TI_REAL B0[20] = {0.837747,0.716200,0.547128,0.400596,0.289459,0.209659,0.153408,0.113779,0.085632,0.065397,0.050648,0.039744,0.031571,0.025363,0.020589,0.016875,0.013953,0.011632,0.009770,0.008263};
    TI_REAL A1[20] = {0.173178,0.320247,0.559812,0.817734,1.066023,1.293310,1.496649,1.676861,1.836187,1.977213,2.102418,2.214012,2.313903,2.403709,2.484797,2.558316,2.625237,2.686378,2.742435,2.794000};
    TI_REAL A2[20] = {-0.011247,-0.038459,-0.117521,-0.250758,-0.426152,-0.627244,-0.839984,-1.054449,-1.264344,-1.466015,-1.657560,-1.838193,-2.007804,-2.166681,-2.315331,-2.454368,-2.584450,-2.706235,-2.820356,-2.927413};
    TI_REAL A3[20] = {0.000325,0.002053,0.010965,0.034176,0.075715,0.135204,0.209527,0.294694,0.386929,0.483104,0.580814,0.678297,0.774311,0.868012,0.958854,1.046508,1.130799,1.211662,1.289107,1.363199};
    TI_REAL A4[20] = {0.000004,0.000041,0.000384,0.001747,0.005045,0.010929,0.019599,0.030885,0.044405,0.059700,0.076320,0.093860,0.111980,0.130403,0.148910,0.167331,0.185538,0.203436,0.220956,0.238049};
} static tables;

int ti_gf4_start(TI_REAL const *options) {
    const int period = options[0];

    return 0;
}

/* ALERT:
 * This implementation is based on "Gaussian and Other Low Lag Filters"
 * by John Ehlers and the constants are suspected to be bogus.
 * Consider using a composition of EMA instead. */

int ti_gf4(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int period = options[0];
    TI_REAL *gf4 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    ringbuf<5> f;

    const int idx = period/2-1;
    const TI_REAL B0 = tables.B0[idx];
    const TI_REAL A1 = tables.A1[idx];
    const TI_REAL A2 = tables.A2[idx];
    const TI_REAL A3 = tables.A3[idx];
    const TI_REAL A4 = tables.A4[idx];

    TI_REAL csum_reciproc = 1./(B0 + A1 + A2 + A3 + A4);

    int i = 0;
    for (; i < 5; ++i, step(f)) {
        f = series[i];
        *gf4++ = f;
    }
    for (; i < size; ++i, step(f)) {
        f = (B0*series[i] + A1*f[1] + A2*f[2] + A3*f[3] + A4*f[4]) * csum_reciproc;
        *gf4++ = f;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_gf4_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int period = options[0];
    TI_REAL *gf4 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    ringbuf<5> f;

    const int idx = period/2-1;
    const TI_REAL B0 = tables.B0[idx];
    const TI_REAL A1 = tables.A1[idx];
    const TI_REAL A2 = tables.A2[idx];
    const TI_REAL A3 = tables.A3[idx];
    const TI_REAL A4 = tables.A4[idx];
    TI_REAL csum_reciproc = 1./(B0 + A1 + A2 + A3 + A4);

    int i = 0;
    for (; i < 5; ++i, step(f)) {
        f = series[i];
        *gf4++ = f;
    }
    for (; i < size; ++i, step(f)) {
        f = (B0*series[i] + A1*f[1] + A2*f[2] + A3*f[3] + A4*f[4]) * csum_reciproc;
        *gf4++ = f;
    }

    return TI_OKAY;
}

struct ti_gf4_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        ringbuf<5> f;
    } state;

    struct {
        TI_REAL B0;
        TI_REAL A1;
        TI_REAL A2;
        TI_REAL A3;
        TI_REAL A4;
        TI_REAL csum_reciproc;
    } constants;
};

int ti_gf4_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    ti_gf4_stream *ptr = new(std::nothrow) ti_gf4_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_GF4_INDEX;
    ptr->progress = -ti_gf4_start(options);

    ptr->options.period = period;

    const int idx = period/2-1;
    ptr->constants.B0 = tables.B0[idx];
    ptr->constants.A1 = tables.A1[idx];
    ptr->constants.A2 = tables.A2[idx];
    ptr->constants.A3 = tables.A3[idx];
    ptr->constants.A4 = tables.A4[idx];
    ptr->constants.csum_reciproc = 1. / (
        tables.B0[idx]
        + tables.A1[idx]
        + tables.A2[idx]
        + tables.A3[idx]
        + tables.A4[idx]
    );

    return TI_OKAY;
}

void ti_gf4_stream_free(ti_stream *stream) {
    delete static_cast<ti_gf4_stream*>(stream);
}

int ti_gf4_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_gf4_stream *ptr = static_cast<ti_gf4_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *gf4 = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;

    const TI_REAL B0 = ptr->constants.B0;
    const TI_REAL A1 = ptr->constants.A1;
    const TI_REAL A2 = ptr->constants.A2;
    const TI_REAL A3 = ptr->constants.A3;
    const TI_REAL A4 = ptr->constants.A4;
    const TI_REAL csum_reciproc = ptr->constants.csum_reciproc;

    auto& f = ptr->state.f;

    int i = 0;
    for (; progress < 5 && i < size; ++i, ++progress, step(f)) {
        f = series[i];
        *gf4++ = f;
    }
    for (; i < size; ++i, ++progress, step(f)) {
        f = (B0*series[i] + A1*f[1] + A2*f[2] + A3*f[3] + A4*f[4]) * csum_reciproc;
        *gf4++ = f;
    }

    ptr->progress = progress;

    return TI_OKAY;
}
