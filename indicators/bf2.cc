#include "../indicators.h"
#include <new>
#include "../utils/localbuffer.h"
#include "../utils/log.h"

#include <new>
#include <utility>

int ti_bf2_start(TI_REAL const *options) {
    TI_REAL period = options[0];

    return ceil(2*period/(3.1415*3.1415));
}

struct {
    TI_REAL B0[20] = {0.285784,0.203973,0.130825,0.088501,0.063284,0.047322,0.036654,0.029198,0.023793,0.019754,0.016658,0.014235,0.012303,0.010739,0.009454,0.008386,0.007490,0.006729,0.006079,0.005518};
    TI_REAL B1[20] = {0.571568,0.407946,0.261650,0.177002,0.126567,0.094643,0.073308,0.058397,0.047586,0.039507,0.033317,0.028470,0.024607,0.021477,0.018908,0.016773,0.014980,0.013459,0.012158,0.011037};
    TI_REAL B2[20] = {0.285784,0.203973,0.130825,0.088501,0.063284,0.047322,0.036654,0.029198,0.023793,0.019754,0.016658,0.014235,0.012303,0.010739,0.009454,0.008386,0.007490,0.006729,0.006079,0.005518};
    TI_REAL A1[20] = {-0.131366,0.292597,0.704171,0.975372,1.158161,1.287652,1.383531,1.457120,1.515266,1.562309,1.601119,1.633667,1.661342,1.685157,1.705862,1.724025,1.740086,1.754388,1.767204,1.778753};
    TI_REAL A2[20] = {-0.011770,-0.108489,-0.227470,-0.329377,-0.411296,-0.476938,-0.530147,-0.573914,-0.610438,-0.641324,-0.667753,-0.690607,-0.710555,-0.728112,-0.743678,-0.757571,-0.770045,-0.781305,-0.791520,-0.800827};
} static tables;

int ti_bf2(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    TI_REAL period = options[0];
    TI_REAL *bf2 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if ((int)period % 2 != 0) { return TI_INVALID_OPTION; }
    if ((int)period > 40) { return TI_INVALID_OPTION; }

    TI_REAL B0 = tables.B0[(int)period / 2 - 1];
    TI_REAL B1 = tables.B1[(int)period / 2 - 1];
    TI_REAL B2 = tables.B2[(int)period / 2 - 1];
    TI_REAL A1 = tables.A1[(int)period / 2 - 1];
    TI_REAL A2 = tables.A2[(int)period / 2 - 1];

    TI_REAL f1 = 0.;
    TI_REAL f2 = 0.;
    TI_REAL g1 = 0.;
    TI_REAL g2 = 0.;

    int progress = -ti_bf2_start(options);

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress) {
        TI_REAL f = A1*f1 + A2*f2 + B0*real[i] + B1*g1 + B2*g2;

        g2 = std::exchange(g1, real[i]);
        f2 = std::exchange(f1, f);
    }
    for (; i < size; ++i, ++progress) {
        TI_REAL f = A1*f1 + A2*f2 + B0*real[i] + B1*g1 + B2*g2;
        *bf2++ = f;

        g2 = std::exchange(g1, real[i]);
        f2 = std::exchange(f1, f);
    }

    return TI_OKAY;
}

struct ti_bf2_stream : ti_stream {

    struct {
        TI_REAL period;
    } options;

    struct {
        TI_REAL f1;
        TI_REAL f2;
        TI_REAL g1;
        TI_REAL g2;
    } state;

    struct {
        TI_REAL B0;
        TI_REAL B1;
        TI_REAL B2;
        TI_REAL A1;
        TI_REAL A2;
    } constants;
};

int ti_bf2_stream_new(TI_REAL const *options, ti_stream **stream) {
    TI_REAL period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if ((int)period % 2 != 0) { return TI_INVALID_OPTION; }
    if ((int)period > 40) { return TI_INVALID_OPTION; }

    ti_bf2_stream *ptr = new(std::nothrow) ti_bf2_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_BF2_INDEX;
    ptr->progress = -ti_bf2_start(options);

    ptr->options.period = period;

    ptr->constants.B0 = tables.B0[(int)period / 2 - 1];
    ptr->constants.B1 = tables.B1[(int)period / 2 - 1];
    ptr->constants.B2 = tables.B2[(int)period / 2 - 1];
    ptr->constants.A1 = tables.A1[(int)period / 2 - 1];
    ptr->constants.A2 = tables.A2[(int)period / 2 - 1];

    return TI_OKAY;
}

void ti_bf2_stream_free(ti_stream *stream) {
    delete static_cast<ti_bf2_stream*>(stream);
}

int ti_bf2_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_bf2_stream *ptr = static_cast<ti_bf2_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *bf2 = outputs[0];
    int progress = ptr->progress;
    TI_REAL period = ptr->options.period;

    TI_REAL B0 = ptr->constants.B0;
    TI_REAL B1 = ptr->constants.B1;
    TI_REAL B2 = ptr->constants.B2;
    TI_REAL A1 = ptr->constants.A1;
    TI_REAL A2 = ptr->constants.A2;

    TI_REAL f1 = ptr->state.f1;
    TI_REAL f2 = ptr->state.f2;
    TI_REAL g1 = ptr->state.g1;
    TI_REAL g2 = ptr->state.g2;

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress) {
        TI_REAL f = A1*f1 + A2*f2 + B0*real[i] + B1*g1 + B2*g2;

        g2 = std::exchange(g1, real[i]);
        f2 = std::exchange(f1, f);
    }
    for (; i < size; ++i, ++progress) {
        TI_REAL f = A1*f1 + A2*f2 + B0*real[i] + B1*g1 + B2*g2;
        *bf2++ = f;

        g2 = std::exchange(g1, real[i]);
        f2 = std::exchange(f1, f);
    }

    ptr->progress = progress;
    ptr->state.f1 = f1;
    ptr->state.f2 = f2;
    ptr->state.g1 = g1;
    ptr->state.g2 = g2;

    return TI_OKAY;
}
