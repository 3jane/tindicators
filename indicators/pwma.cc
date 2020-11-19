#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_pwma_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    const TI_REAL power = options[1];

    return period-1;
}

int ti_pwma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    const TI_REAL power = options[1];
    TI_REAL *pwma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    TI_REAL denom = 0;

    for (int i = 0; i < period && i < size; ++i) {
        denom += pow(i+1, power);
    }

    for (int i = period-1; i < size; ++i) {
        TI_REAL numer = 0;
        for (int j = 0; j < period; ++j) {
            numer += real[i-j] * pow(period-j, power);
        }
        *pwma++ = numer / denom;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_pwma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    const TI_REAL power = options[1];
    TI_REAL *pwma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    TI_REAL denom = 0;

    for (int i = 0; i < period && i < size; ++i) {
        denom += pow(i+1, power);
    }

    for (int i = period-1; i < size; ++i) {
        TI_REAL numer = 0;
        for (int j = 0; j < period; ++j) {
            numer += real[i-j] * pow(period-j, power);
        }
        *pwma++ = numer / denom;
    }

    return TI_OKAY;
}

struct ti_pwma_stream : ti_stream {

    struct {
        int period;
        TI_REAL power;
    } options;

    struct {
        ringbuf<0> price;
    } state;

    struct {
        TI_REAL denom_recipr = 0;
    } constants;
};

int ti_pwma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];
    const TI_REAL power = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }

    ti_pwma_stream *ptr = new(std::nothrow) ti_pwma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_PWMA_INDEX;
    ptr->progress = -ti_pwma_start(options);

    ptr->options.period = period;
    ptr->options.power = power;

    try {
        ptr->state.price.resize(period+1);
    } catch (std::bad_alloc& e) {
        return TI_OUT_OF_MEMORY;
    }

    for (int i = 0; i < period; ++i) {
        ptr->constants.denom_recipr += pow(i+1, power);
    }
    ptr->constants.denom_recipr = 1. / ptr->constants.denom_recipr;

    return TI_OKAY;
}

void ti_pwma_stream_free(ti_stream *stream) {
    delete static_cast<ti_pwma_stream*>(stream);
}

int ti_pwma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_pwma_stream *ptr = static_cast<ti_pwma_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *pwma = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;
    const TI_REAL power = ptr->options.power;
    const TI_REAL denom_recipr = ptr->constants.denom_recipr;
    auto &price = ptr->state.price;

    int i = 0;

    for (; progress < 0 && i < size; ++i, ++progress, step(price)) {
        price = real[i];
    }
    for (; i < size; ++i, ++progress, step(price)) {
        price = real[i];

        TI_REAL numer = 0;
        for (int j = 0; j < period; ++j) {
            numer += price[j] * pow(period-j, power);
        }
        *pwma++ = numer * denom_recipr;
    }

    ptr->progress = progress;

    return TI_OKAY;
}
