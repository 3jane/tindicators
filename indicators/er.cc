#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_er_start(TI_REAL const *options) {
    const int period = options[0];
    const int n = period;

    return n;
}

int ti_er(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    const int n = period;
    TI_REAL *er = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    TI_REAL net_change = 0;
    TI_REAL trajectory = 0;

    int i = 0;
    for (i = 1; i < n && i < size; ++i) {
        trajectory += fabs(real[i] - real[i-1]);
    }
    for (; i < size; ++i) {
        trajectory += fabs(real[i] - real[i-1]);
        TI_REAL net_change = fabs(real[i] - real[i-n]);

        *er++ = net_change ? net_change / trajectory : 0;

        trajectory -= fabs(real[i-n+1] - real[i-n]);
    }

    return TI_OKAY;
}

int DONTOPTIMIZE ti_er_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    const int n = period;
    TI_REAL *er = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    for (int i = n + 1; i <= size; ++i) {
        TI_REAL net_change = fabs(real[i-1] - real[i-n-1]);
        TI_REAL trajectory = 0;
        for (int j = i - n; j < i; ++j) {
            trajectory += fabs(real[j] - real[j-1]);
        }
        *er++ = net_change ? net_change / trajectory : 0;
    }

    return TI_OKAY;
}

struct ti_er_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        TI_REAL trajectory;
        ringbuf<0> price;
    } state;

    struct {

    } constants;
};

int ti_er_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];
    const int n = period;

    if (period < 1) { return TI_INVALID_OPTION; }

    ti_er_stream *ptr = new(std::nothrow) ti_er_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_ER_INDEX;
    ptr->progress = -ti_er_start(options);

    ptr->options.period = period;

    ptr->state.price.resize(period+1);

    return TI_OKAY;
}

void ti_er_stream_free(ti_stream *stream) {
    delete static_cast<ti_er_stream*>(stream);
}

int ti_er_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_er_stream *ptr = static_cast<ti_er_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *er = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;
    const int n = period;
    TI_REAL trajectory = ptr->state.trajectory;
    auto &price = ptr->state.price;

    int i = 0;
    for (; i < size && progress < -n+1; ++i, ++progress, step(price)) {
        price = real[i];
    }
    for (; i < size && progress < 0; ++i, ++progress, step(price)) {
        price = real[i];
        trajectory += fabs(price - price[1]);
    }
    for (; i < size; ++i, ++progress, step(price)) {
        price = real[i];
        trajectory += fabs(price - price[1]);

        TI_REAL net_change = fabs(price - price[n]);

        *er++ = net_change ? net_change / trajectory : 0;

        trajectory -= fabs(price[n-1] - price[n]);
    }

    ptr->progress = progress;
    ptr->state.trajectory = trajectory;

    return TI_OKAY;
}