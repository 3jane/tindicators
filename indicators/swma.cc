#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_swma_start(TI_REAL const *options) {
    const int period = options[0];

    return period-1;
}

static TI_REAL PI = acos(-1);

static TI_REAL table[] = {
    sin( 1. * PI / 6),
    sin( 2. * PI / 6),
    sin( 3. * PI / 6),
    sin( 4. * PI / 6),
    sin( 5. * PI / 6),
    sin( 6. * PI / 6),
    sin( 7. * PI / 6),
    sin( 8. * PI / 6),
    sin( 9. * PI / 6),
    sin(10. * PI / 6),
    sin(11. * PI / 6),
    sin(12. * PI / 6),
};

int ti_swma(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *swma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 6 == 0 || (period + 1) % 6 == 0) { return TI_INVALID_OPTION; }

    TI_REAL denom = 0;
    for (int j = 0; j < period; ++j) {
        TI_REAL c = table[j%12];
        denom += c;
    }

    for (int i = period-1; i < size; ++i) {
        TI_REAL numer = 0;
        for (int j = 0; j < period; ++j) {
            TI_REAL c = table[j%12];
            numer += c * real[i-j];
        }
        *swma++ = numer / denom;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_swma_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const int period = options[0];
    TI_REAL *swma = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 6 == 0 || (period + 1) % 6 == 0) { return TI_INVALID_OPTION; }

    for (int i = period-1; i < size; ++i) {
        TI_REAL numer = 0;
        TI_REAL denom = 0;
        for (int j = 0; j < period; ++j) {
            TI_REAL c = sin((j+1) * PI / 6.);
            numer += c * real[i-j];
            denom += c;
        }
        *swma++ = numer / denom;
    }

    return TI_OKAY;
}

struct ti_swma_stream : ti_stream {

    struct {
        int period;
    } options;

    struct {
        ringbuf<0> price;
    } state;

    struct {
        TI_REAL denom;
    } constants;
};

int ti_swma_stream_new(TI_REAL const *options, ti_stream **stream) {
    const int period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (period % 6 == 0 || (period + 1) % 6 == 0) { return TI_INVALID_OPTION; }

    ti_swma_stream *ptr = new(std::nothrow) ti_swma_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_SWMA_INDEX;
    ptr->progress = -ti_swma_start(options);

    ptr->options.period = period;

    try {
        ptr->state.price.resize(period);
    } catch (std::bad_alloc& e) {
        return TI_OUT_OF_MEMORY;
    }

    ptr->constants.denom = 0;
    for (int j = 0; j < period; ++j) {
        TI_REAL c = table[j%12];
        ptr->constants.denom += c;
    }

    return TI_OKAY;
}

void ti_swma_stream_free(ti_stream *stream) {
    delete static_cast<ti_swma_stream*>(stream);
}

int ti_swma_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_swma_stream *ptr = static_cast<ti_swma_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *swma = outputs[0];
    int progress = ptr->progress;
    const int period = ptr->options.period;
    const TI_REAL denom = ptr->constants.denom;
    auto &price = ptr->state.price;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(price)) {
        price = real[i];
    }
    for (; i < size; ++i, ++progress, step(price)) {
        price = real[i];
        TI_REAL numer = 0;
        for (int j = 0; j < period; ++j) {
            TI_REAL c = table[j%12];
            numer += c * price[j];
        }
        *swma++ = numer / denom;
    }

    ptr->progress = progress;

    return TI_OKAY;
}
