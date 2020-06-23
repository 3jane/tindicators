#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

#include <vector>

int ti_idwma_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    const TI_REAL exponent = options[1];

    return period-1;
}

int ti_idwma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    const TI_REAL exponent = options[1];
    TI_REAL *idwma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (exponent < 1) { return TI_INVALID_OPTION; }
    if (exponent > 2) { return TI_INVALID_OPTION; }

    std::vector<TI_REAL> coefficients(period);
    TI_REAL denom = 0;
    for (int i = 0; i < period; ++i) {
        coefficients[i] = 1 / pow(i+1, exponent);
        denom += coefficients[i];
    }

    for (int i = period-1; i < size; ++i) {
        TI_REAL numer = 0;
        for (int j = 0; j < period; ++j) {
            numer += real[i-j] * coefficients[j];
        }
        *idwma++ = numer / denom;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_idwma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const TI_REAL period = options[0];
    const TI_REAL exponent = options[1];
    TI_REAL *idwma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (exponent < 1) { return TI_INVALID_OPTION; }
    if (exponent > 2) { return TI_INVALID_OPTION; }

    TI_REAL denom = 0;
    for (int i = 0; i < period; ++i) {
        denom += 1 / pow(i+1, exponent);
    }

    for (int i = period-1; i < size; ++i) {
        TI_REAL numer = 0;
        for (int j = 0; j < period; ++j) {
            numer += real[i-j] * 1 / pow(j+1, exponent);
        }
        idwma[i-(int)period+1] = numer / denom;
    }

    return TI_OKAY;
}

struct ti_idwma_stream : ti_stream {

    struct {
        int period;
        TI_REAL exponent;
    } options;

    struct {
        ringbuf<0> price;
    } state;

    struct {
        TI_REAL denom;
        std::vector<TI_REAL> coefficients;
    } constants;
};

int ti_idwma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];
    const TI_REAL exponent = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (exponent < 1) { return TI_INVALID_OPTION; }
    if (exponent > 2) { return TI_INVALID_OPTION; }

    ti_idwma_stream *ptr = new(std::nothrow) ti_idwma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_IDWMA_INDEX;
    ptr->progress = -ti_idwma_start(options);

    ptr->options.period = period;
    ptr->options.exponent = exponent;

    ptr->state.price.resize(period);

    ptr->constants.denom = 0;
    ptr->constants.coefficients.resize(period);

    for (int i = 0; i < period; ++i) {
        ptr->constants.coefficients[i] = 1 / pow(i+1, exponent);
        ptr->constants.denom += ptr->constants.coefficients[i];
    }

    return TI_OKAY;
}

void ti_idwma_stream_free(ti_stream *stream) {
    delete static_cast<ti_idwma_stream*>(stream);
}

int ti_idwma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_idwma_stream *ptr = static_cast<ti_idwma_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *idwma = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;
    const TI_REAL exponent = ptr->options.exponent;

    auto &price = ptr->state.price;

    const TI_REAL denom = ptr->constants.denom;
    const std::vector<TI_REAL> &coefficients = ptr->constants.coefficients;

    int i = 0;
    
    for (; progress < 0 && i < size; ++i, ++progress, step(price)) {
        price = real[i];
    }
    for (; i < size; ++i, ++progress, step(price)) {
        price = real[i];
        
        TI_REAL numer = 0;
        for (int j = 0; j < period; ++j) {
            numer += price[j] * coefficients[j];
        }
        *idwma++ = numer / denom;
    }

    ptr->progress = progress;

    return TI_OKAY;
}