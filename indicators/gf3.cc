#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

struct {
    TI_REAL B0[20] = {0.836701,0.718670,0.558792,0.422292,0.318295,0.242068,0.186612,0.146016,0.115940,0.093340,0.076111,0.062791,0.052354,0.044075,0.037432,0.032045,0.027635,0.023991,0.020956,0.018409};
    TI_REAL A1[20] = {0.173094,0.312814,0.529009,0.749259,0.951680,1.130321,1.285644,1.420251,1.537154,1.639147,1.728632,1.807607,1.877714,1.940297,1.996460,2.047111,2.093000,2.134754,2.172895,2.207865};
    TI_REAL A2[20] = {-0.009987,-0.032617,-0.093283,-0.187130,-0.301899,-0.425875,-0.550960,-0.672371,-0.787614,-0.895601,-0.996056,-1.089148,-1.175270,-1.254918,-1.328618,-1.396887,-1.460217,-1.519058,-1.573824,-1.624889};
    TI_REAL A3[20] = {0.000192,0.001134,0.005483,0.015579,0.031923,0.053486,0.078704,0.106104,0.134520,0.163114,0.191313,0.218750,0.245202,0.270546,0.294726,0.317731,0.339582,0.360313,0.379973,0.398615};
} static tables;

int ti_gf3_start(TI_REAL const *options) {
    const int period = options[0];

    return 0;
}

int ti_gf3(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int period = options[0];
    TI_REAL *gf3 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    ringbuf<4> f;

    const int idx = period/2-1;

    const TI_REAL B0 = tables.B0[idx];
    const TI_REAL A1 = tables.B0[idx];
    const TI_REAL A2 = tables.B0[idx];
    const TI_REAL A3 = tables.B0[idx];
    const TI_REAL csum_recipr = 1. / (B0 + A1 + A2 + A3);

    int i = 0;
    for (; i < size; ++i, step(f)) {
        f = (B0*series[i] + A1*f[1] + A2*f[2] + A3*f[3]) * csum_recipr;
        *gf3++ = f;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_gf3_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const int period = options[0];
    TI_REAL *gf3 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    ringbuf<4> f;

    const int idx = period/2-1;

    const TI_REAL B0 = tables.B0[idx];
    const TI_REAL A1 = tables.B0[idx];
    const TI_REAL A2 = tables.B0[idx];
    const TI_REAL A3 = tables.B0[idx];
    const TI_REAL csum_recipr = 1. / (B0 + A1 + A2 + A3);

    int i = 0;
    for (; i < size; ++i, step(f)) {
        f = (B0*series[i] + A1*f[1] + A2*f[2] + A3*f[3]) * csum_recipr;
        *gf3++ = f;
    }

    return TI_OKAY;
}

struct ti_gf3_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        ringbuf<4> f;
    } state;

    struct {
        TI_REAL B0;
        TI_REAL A1;
        TI_REAL A2;
        TI_REAL A3;
        TI_REAL csum_recipr;
    } constants;
};

int ti_gf3_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 2 != 0) { return TI_INVALID_OPTION; }
    if (period > 40) { return TI_INVALID_OPTION; }

    ti_gf3_stream *ptr = new(std::nothrow) ti_gf3_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_GF3_INDEX;
    ptr->progress = -ti_gf3_start(options);

    ptr->options.period = period;

    const int idx = period/2-1;
    ptr->constants.B0 = tables.B0[idx];
    ptr->constants.A1 = tables.B0[idx];
    ptr->constants.A2 = tables.B0[idx];
    ptr->constants.A3 = tables.B0[idx];
    ptr->constants.csum_recipr = 1. / (ptr->constants.B0 + ptr->constants.A1 + ptr->constants.A2 + ptr->constants.A3);

    return TI_OKAY;
}

void ti_gf3_stream_free(ti_stream *stream) {
    delete static_cast<ti_gf3_stream*>(stream);
}

int ti_gf3_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_gf3_stream *ptr = static_cast<ti_gf3_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *gf3 = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;

    auto& f = ptr->state.f;

    const TI_REAL B0 = ptr->constants.B0;
    const TI_REAL A1 = ptr->constants.A1;
    const TI_REAL A2 = ptr->constants.A2;
    const TI_REAL A3 = ptr->constants.A3;
    const TI_REAL csum_recipr = ptr->constants.csum_recipr;

    int i = 0;
    for (; i < size; ++i, ++progress, step(f)) {
        f = (B0*series[i] + A1*f[1] + A2*f[2] + A3*f[3]) * csum_recipr;
        *gf3++ = f;
    }

    ptr->progress = progress;

    return TI_OKAY;
}
