#include "../indicators.h"
#include <new>
#include "../utils/localbuffer.h"
#include "../utils/log.h"

#include <new>
#include <utility>

int ti_bf3_start(TI_REAL const *options) {
    TI_REAL period = options[0];

    return ceil(3*period/(3.1415*3.1415));
}

struct {
    TI_REAL B0[20] = {0.170149,0.100733,0.050373,0.027610,0.016541,0.010629,0.007213,0.005111,0.003750,0.002831,0.002188,0.001726,0.001385,0.001128,0.000931,0.000778,0.000656,0.000558,0.000479,0.000414};
    TI_REAL B1[20] = {0.510448,0.302200,0.151118,0.082830,0.049622,0.031887,0.021640,0.015334,0.011250,0.008492,0.006565,0.005179,0.004156,0.003385,0.002794,0.002333,0.001967,0.001674,0.001437,0.001242};
    TI_REAL B2[20] = {0.510448,0.302200,0.151118,0.082830,0.049622,0.031887,0.021640,0.015334,0.011250,0.008492,0.006565,0.005179,0.004156,0.003385,0.002794,0.002333,0.001967,0.001674,0.001437,0.001242};
    TI_REAL B3[20] = {0.170149,0.100733,0.050373,0.027610,0.016541,0.010629,0.007213,0.005111,0.003750,0.002831,0.002188,0.001726,0.001385,0.001128,0.000931,0.000778,0.000656,0.000558,0.000479,0.000414};
    TI_REAL A1[20] = {-0.336246,0.398405,1.080990,1.505892,1.783327,1.976163,2.117205,2.224560,2.308883,2.376806,2.432658,2.479376,2.519020,2.553078,2.582648,2.608560,2.631451,2.651819,2.670059,2.686486};
    TI_REAL A2[20] = {-0.026816,-0.247486,-0.607116,-0.934652,-1.200263,-1.412114,-1.582459,-1.721388,-1.836396,-1.932941,-2.015013,-2.085571,-2.146834,-2.200500,-2.247883,-2.290012,-2.327708,-2.361631,-2.392315,-2.420202};
    TI_REAL A3[20] = {0.001867,0.043214,0.123145,0.207880,0.284610,0.350920,0.407548,0.455938,0.497514,0.533488,0.564848,0.592385,0.616731,0.638395,0.657784,0.675232,0.691011,0.705347,0.718425,0.730403};
} static tables;

int ti_bf3(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    TI_REAL period = options[0];
    TI_REAL *bf3 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if ((int)period % 2 != 0) { return TI_INVALID_OPTION; }
    if ((int)period > 40) { return TI_INVALID_OPTION; }

    TI_REAL B0 = tables.B0[(int)period / 2 - 1];
    TI_REAL B1 = tables.B1[(int)period / 2 - 1];
    TI_REAL B2 = tables.B2[(int)period / 2 - 1];
    TI_REAL B3 = tables.B3[(int)period / 2 - 1];
    TI_REAL A1 = tables.A1[(int)period / 2 - 1];
    TI_REAL A2 = tables.A2[(int)period / 2 - 1];
    TI_REAL A3 = tables.A3[(int)period / 2 - 1];

    TI_REAL f1 = 0.;
    TI_REAL f2 = 0.;
    TI_REAL f3 = 0.;
    TI_REAL g1 = 0.;
    TI_REAL g2 = 0.;
    TI_REAL g3 = 0.;

    int progress = -ti_bf3_start(options);

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress) {
        TI_REAL f = A1*f1 + A2*f2 + A3*f3 + B0*real[i] + B1*g1 + B2*g2 + B3*g3;

        g3 = std::exchange(g2, std::exchange(g1, real[i]));
        f3 = std::exchange(f2, std::exchange(f1, f));
    }
    for (; i < size; ++i, ++progress) {
        TI_REAL f = A1*f1 + A2*f2 + A3*f3 + B0*real[i] + B1*g1 + B2*g2 + B3*g3;
        *bf3++ = f;

        g3 = std::exchange(g2, std::exchange(g1, real[i]));
        f3 = std::exchange(f2, std::exchange(f1, f));
    }

    return TI_OKAY;
}

struct ti_bf3_stream : ti_stream {

    struct {
        TI_REAL period;
    } options;

    struct {
        TI_REAL f1;
        TI_REAL f2;
        TI_REAL f3;
        TI_REAL g1;
        TI_REAL g2;
        TI_REAL g3;
    } state;

    struct {
        TI_REAL B0;
        TI_REAL B1;
        TI_REAL B2;
        TI_REAL B3;
        TI_REAL A1;
        TI_REAL A2;
        TI_REAL A3;
    } constants;
};

int ti_bf3_stream_new(TI_REAL const *options, ti_stream **stream) {
    TI_REAL period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if ((int)period % 2 != 0) { return TI_INVALID_OPTION; }
    if ((int)period > 40) { return TI_INVALID_OPTION; }

    ti_bf3_stream *ptr = new(std::nothrow) ti_bf3_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_BF3_INDEX;
    ptr->progress = -ti_bf3_start(options);

    ptr->options.period = period;

    ptr->constants.B0 = tables.B0[(int)period / 2 - 1];
    ptr->constants.B1 = tables.B1[(int)period / 2 - 1];
    ptr->constants.B2 = tables.B2[(int)period / 2 - 1];
    ptr->constants.B3 = tables.B3[(int)period / 2 - 1];
    ptr->constants.A1 = tables.A1[(int)period / 2 - 1];
    ptr->constants.A2 = tables.A2[(int)period / 2 - 1];
    ptr->constants.A3 = tables.A3[(int)period / 2 - 1];

    return TI_OKAY;
}

void ti_bf3_stream_free(ti_stream *stream) {
    delete static_cast<ti_bf3_stream*>(stream);
}

int ti_bf3_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_bf3_stream *ptr = static_cast<ti_bf3_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *bf3 = outputs[0];
    int progress = ptr->progress;
    TI_REAL period = ptr->options.period;

    TI_REAL B0 = ptr->constants.B0;
    TI_REAL B1 = ptr->constants.B1;
    TI_REAL B2 = ptr->constants.B2;
    TI_REAL B3 = ptr->constants.B3;
    TI_REAL A1 = ptr->constants.A1;
    TI_REAL A2 = ptr->constants.A2;
    TI_REAL A3 = ptr->constants.A3;

    TI_REAL f1 = ptr->state.f1;
    TI_REAL f2 = ptr->state.f2;
    TI_REAL f3 = ptr->state.f3;
    TI_REAL g1 = ptr->state.g1;
    TI_REAL g2 = ptr->state.g2;
    TI_REAL g3 = ptr->state.g3;

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress) {
        TI_REAL f = A1*f1 + A2*f2 + A3*f3 + B0*real[i] + B1*g1 + B2*g2 + B3*g3;

        g3 = std::exchange(g2, std::exchange(g1, real[i]));
        f3 = std::exchange(f2, std::exchange(f1, f));
    }
    for (; i < size; ++i, ++progress) {
        TI_REAL f = A1*f1 + A2*f2 + A3*f3 + B0*real[i] + B1*g1 + B2*g2 + B3*g3;
        *bf3++ = f;

        g3 = std::exchange(g2, std::exchange(g1, real[i]));
        f3 = std::exchange(f2, std::exchange(f1, f));
    }

    ptr->progress = progress;
    ptr->state.f1 = f1;
    ptr->state.f2 = f2;
    ptr->state.f3 = f3;
    ptr->state.g1 = g1;
    ptr->state.g2 = g2;
    ptr->state.g3 = g3;

    return TI_OKAY;
}