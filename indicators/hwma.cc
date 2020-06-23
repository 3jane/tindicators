#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_hwma_start(TI_REAL const *options) {
    const int period = options[0];

    return period-1;
}

struct {
    TI_REAL coefficients[7][13] = {
        {0,0,0,0,0,0,-0.092,-0.058,0.012,0.12,0.244,0.353,0.421},
        {0,0,0,0,0,-0.043,-0.038,0.002,0.08,0.174,0.254,0.292,0.279},
        {0,0,0,0,-0.016,-0.025,0.003,0.068,0.149,0.216,0.241,0.216,0.148},
        {0,0,0,-0.009,-0.022,0.004,0.066,0.145,0.208,0.23,0.201,0.131,0.046},
        {0,0,-0.011,-0.022,0.003,0.067,0.145,0.21,0.235,0.205,0.136,0.05,-0.018},
        {0,-0.017,-0.025,0.001,0.066,0.147,0.213,0.238,0.212,0.144,0.061,-0.006,-0.034},
        {-0.019,-0.028,0,0.066,0.147,0.214,0.24,0.214,0.147,0.066,0,-0.028,-0.019},
    };
} static tables;

int ti_hwma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *hwma = outputs[0];

    auto &c = tables.coefficients[period-7];

    if (period < 7 || period > 13) { return TI_INVALID_OPTION; }

    for (int i = period-1; i < size; ++i) {
        TI_REAL filt = 0;
        for (int j = 0; j < period; ++j) {
            filt += c[12-j] * real[i-j];
        }
        *hwma++ = filt;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_hwma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *hwma = outputs[0];

    if (period < 7 || period > 13) { return TI_INVALID_OPTION; }

    auto &c = tables.coefficients[period-7];

    for (int i = period-1; i < size; ++i) {
        TI_REAL filt = 0;
        for (int j = 0; j < period; ++j) {
            filt += c[12-j] * real[i-j];
        }
        *hwma++ = filt;
    }

    return TI_OKAY;
}

struct ti_hwma_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        ringbuf<13> price;
    } state;

    struct {

    } constants;
};

int ti_hwma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];

    if (period < 7 || period > 13) { return TI_INVALID_OPTION; }


    ti_hwma_stream *ptr = new(std::nothrow) ti_hwma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_HWMA_INDEX;
    ptr->progress = -ti_hwma_start(options);

    ptr->options.period = period;

    return TI_OKAY;
}

void ti_hwma_stream_free(ti_stream *stream) {
    delete static_cast<ti_hwma_stream*>(stream);
}

int ti_hwma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_hwma_stream *ptr = static_cast<ti_hwma_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *hwma = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;
    auto &price = ptr->state.price;

    auto &c = tables.coefficients[period-7];

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(price)) {
        price = real[i];
    }
    for (; i < size; ++i, ++progress, step(price)) {
        price = real[i];
        *hwma++ =
              c[0] *price[12]
            + c[1] *price[11]
            + c[2] *price[10]
            + c[3] *price[9]
            + c[4] *price[8]
            + c[5] *price[7]
            + c[6] *price[6]
            + c[7] *price[5]
            + c[8] *price[4]
            + c[9] *price[3]
            + c[10]*price[2]
            + c[11]*price[1]
            + c[12]*price;
    }

    ptr->progress = progress;

    return TI_OKAY;
}
