#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

#include <vector>

int ti_ehma_start(TI_REAL const *options) {
    const int period = options[0];

    return 0;
}

int ti_ehma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *ehma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    TI_REAL ema_n2;
    TI_REAL ema_n;
    TI_REAL ema_n05;

    // Either these must be integral or we can't use ti_ema
    const TI_REAL n2 = period / 2.;
    const TI_REAL n = period;
    const TI_REAL n05 = sqrt(period);

    int i = 0;
    for (; i < 1 && i < size; ++i) {
        ema_n2 = real[i];
        ema_n = real[i];
        ema_n05 = 2*ema_n2 - ema_n;
        
        *ehma++ = ema_n05;
    }
    for (; i < size; ++i) {
        ema_n2 = (real[i] - ema_n2) * 2. / (n2 + 1.) + ema_n2;
        ema_n = (real[i] - ema_n) * 2. / (n + 1.) + ema_n;
        ema_n05 = ((2*ema_n2 - ema_n) - ema_n05) * 2. / (n05 + 1.) + ema_n05;

        *ehma++ = ema_n05;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_ehma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *ehma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    std::vector<TI_REAL> ema_n2(size);
    std::vector<TI_REAL> ema_n(size);
    std::vector<TI_REAL> subtr(size);

    TI_REAL n2 = period / 2.;
    TI_REAL n = period;
    TI_REAL n05 = sqrt(period);

    TI_REAL* arr_[1];

    arr_[0] = ema_n2.data();
    ti_ema(size, &real, &n2, arr_);
    arr_[0] = ema_n.data();
    ti_ema(size, &real, &n, arr_);

    for (int i = 0; i < size; ++i) {
        subtr[i] = 2. * ema_n2[i] - ema_n[i];
    }

    arr_[0] = subtr.data();
    ti_ema(size, arr_, &n05, outputs);

    return TI_OKAY;
}

struct ti_ehma_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        TI_REAL ema_n2;
        TI_REAL ema_n;
        TI_REAL ema_n05;
    } state;

    struct {
        TI_REAL n2_plus1_recipr;
        TI_REAL n_plus1_recipr;
        TI_REAL n05_plus1_recipr;
    } constants;
};

int ti_ehma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    ti_ehma_stream *ptr = new(std::nothrow) ti_ehma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_EHMA_INDEX;
    ptr->progress = -ti_ehma_start(options);

    ptr->options.period = period;

    ptr->constants.n2_plus1_recipr = 1. / (period / 2. + 1);
    ptr->constants.n_plus1_recipr = 1. / (period + 1);
    ptr->constants.n05_plus1_recipr = 1. / (sqrt(period) + 1);

    return TI_OKAY;
}

void ti_ehma_stream_free(ti_stream *stream) {
    delete static_cast<ti_ehma_stream*>(stream);
}

int ti_ehma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_ehma_stream *ptr = static_cast<ti_ehma_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *ehma = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;

    TI_REAL ema_n2 = ptr->state.ema_n2;
    TI_REAL ema_n = ptr->state.ema_n;
    TI_REAL ema_n05 = ptr->state.ema_n05;

    TI_REAL n2_plus1_recipr = ptr->constants.n2_plus1_recipr;
    TI_REAL n_plus1_recipr = ptr->constants.n_plus1_recipr;
    TI_REAL n05_plus1_recipr = ptr->constants.n05_plus1_recipr;

    int i = 0;
    for (; progress < 1 && i < size; ++i, ++progress) {
        ema_n2 = real[i];
        ema_n = real[i];
        ema_n05 = 2*ema_n2 - ema_n;

        *ehma++ = ema_n05;
    }
    for (; i < size; ++i, ++progress) {
        ema_n2 = (real[i] - ema_n2) * 2. * n2_plus1_recipr + ema_n2;
        ema_n = (real[i] - ema_n) * 2. * n_plus1_recipr + ema_n;
        ema_n05 = ((2*ema_n2 - ema_n) - ema_n05) * 2. * n05_plus1_recipr + ema_n05;

        *ehma++ = ema_n05;
    }

    ptr->progress = progress;
    ptr->state.ema_n2 = ema_n2;
    ptr->state.ema_n = ema_n;
    ptr->state.ema_n05 = ema_n05;

    return TI_OKAY;
}
