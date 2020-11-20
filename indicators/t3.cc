#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

#include <vector>

int ti_t3_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    const TI_REAL v = options[1];

    return 0;
}

int ti_t3(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const TI_REAL period = options[0];
    const TI_REAL v = options[1];
    TI_REAL *t3 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (v < 0) { return TI_INVALID_OPTION; }

    TI_REAL ema1_1;
    TI_REAL ema2_1;
    TI_REAL gd1;

    TI_REAL ema1_2;
    TI_REAL ema2_2;
    TI_REAL gd2;

    TI_REAL ema1_3;
    TI_REAL ema2_3;
    TI_REAL gd3;

    int i = 0;
    for (; i < 1 && i < size; ++i) {
        ema1_1 = series[i];
        ema2_1 = series[i];
        gd1 = series[i];

        ema1_2 = series[i];
        ema2_2 = series[i];
        gd2 = series[i];

        ema1_3 = series[i];
        ema2_3 = series[i];
        gd3 = series[i];

        *t3++ = gd3;
    }
    for (; i < size; ++i) {
        ema1_1 = (series[i] - ema1_1) * 2. / (period + 1) + ema1_1;
        ema2_1 = (ema1_1 - ema2_1) * 2. / (period + 1) + ema2_1;
        gd1 = ema1_1 * (1+v) - ema2_1 * v;

        ema1_2 = (gd1 - ema1_2) * 2. / (period + 1) + ema1_2;
        ema2_2 = (ema1_2 - ema2_2) * 2. / (period + 1) + ema2_2;
        gd2 = ema1_2 * (1+v) - ema2_2 * v;

        ema1_3 = (gd2 - ema1_3) * 2. / (period + 1) + ema1_3;
        ema2_3 = (ema1_3 - ema2_3) * 2. / (period + 1) + ema2_3;
        gd3 = ema1_3 * (1+v) - ema2_3 * v;

        *t3++ = gd3;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_t3_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const TI_REAL period = options[0];
    const TI_REAL v = options[1];
    TI_REAL *t3 = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (v < 0) { return TI_INVALID_OPTION; }

    auto GD = [period, v](const std::vector<TI_REAL> &input) -> std::vector<TI_REAL> {
        const int size = input.size();

        std::vector<TI_REAL> ema1(size);
        std::vector<TI_REAL> ema2(size);
        std::vector<TI_REAL> output(size);

        TI_REAL const *arr0[1] = {input.data()};
        TI_REAL *arr1[1] = {ema1.data()};
        ti_ema(size, arr0, &period, arr1);

        TI_REAL *arr2[1] = {ema2.data()};
        ti_ema(size, arr1, &period, arr2);

        for (int i = 0; i < size; ++i) {
            output[i] = ema1[i] * (1+v) - ema2[i] * v;
        }

        return output;
    };

    std::vector<TI_REAL> input(size);
    std::memcpy(input.data(), series, sizeof(TI_REAL) * size);
    std::vector<TI_REAL> output = GD(GD(GD(input)));
    std::memcpy(t3, output.data(), sizeof(TI_REAL) * size);

    return TI_OKAY;
}

struct ti_t3_stream : ti_stream {

    struct {
        TI_REAL period;
        TI_REAL v;
    } options;

    struct {
        TI_REAL ema1_1;
        TI_REAL ema2_1;
        TI_REAL gd1;

        TI_REAL ema1_2;
        TI_REAL ema2_2;
        TI_REAL gd2;

        TI_REAL ema1_3;
        TI_REAL ema2_3;
        TI_REAL gd3;
    } state;

    struct {

    } constants;
};

int ti_t3_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];
    const TI_REAL v = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (v < 0) { return TI_INVALID_OPTION; }

    ti_t3_stream *ptr = new(std::nothrow) ti_t3_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_T3_INDEX;
    ptr->progress = -ti_t3_start(options);

    ptr->options.period = period;
    ptr->options.v = v;

    return TI_OKAY;
}

void ti_t3_stream_free(ti_stream *stream) {
    delete static_cast<ti_t3_stream*>(stream);
}

int ti_t3_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_t3_stream *ptr = static_cast<ti_t3_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *t3 = outputs[0];
    int progress = ptr->progress;
    const TI_REAL period = ptr->options.period;
    const TI_REAL v = ptr->options.v;


    TI_REAL ema1_1 = ptr->state.ema1_1;
    TI_REAL ema2_1 = ptr->state.ema2_1;
    TI_REAL gd1 = ptr->state.gd1;
 
    TI_REAL ema1_2 = ptr->state.ema1_2;
    TI_REAL ema2_2 = ptr->state.ema2_2;
    TI_REAL gd2 = ptr->state.gd2;
 
    TI_REAL ema1_3 = ptr->state.ema1_3;
    TI_REAL ema2_3 = ptr->state.ema2_3;
    TI_REAL gd3 = ptr->state.gd3;

    int i = 0;
    for (; progress < 1 && i < size; ++i, ++progress) {
        ema1_1 = series[i];
        ema2_1 = series[i];
        gd1 = series[i];

        ema1_2 = series[i];
        ema2_2 = series[i];
        gd2 = series[i];

        ema1_3 = series[i];
        ema2_3 = series[i];
        gd3 = series[i];

        *t3++ = gd3;
    }
    for (; i < size; ++i, ++progress) {
        ema1_1 = (series[i] - ema1_1) * 2. / (period + 1) + ema1_1;
        ema2_1 = (ema1_1 - ema2_1) * 2. / (period + 1) + ema2_1;
        gd1 = ema1_1 * (1+v) - ema2_1 * v;

        ema1_2 = (gd1 - ema1_2) * 2. / (period + 1) + ema1_2;
        ema2_2 = (ema1_2 - ema2_2) * 2. / (period + 1) + ema2_2;
        gd2 = ema1_2 * (1+v) - ema2_2 * v;

        ema1_3 = (gd2 - ema1_3) * 2. / (period + 1) + ema1_3;
        ema2_3 = (ema1_3 - ema2_3) * 2. / (period + 1) + ema2_3;
        gd3 = ema1_3 * (1+v) - ema2_3 * v;

        *t3++ = gd3;
    }

    ptr->progress = progress;
    ptr->state.ema1_1 = ema1_1;
    ptr->state.ema2_1 = ema2_1;
    ptr->state.gd1 = gd1;
    ptr->state.ema1_2 = ema1_2;
    ptr->state.ema2_2 = ema2_2;
    ptr->state.gd2 = gd2;
    ptr->state.ema1_3 = ema1_3;
    ptr->state.ema2_3 = ema2_3;
    ptr->state.gd3 = gd3;

    return TI_OKAY;
}
