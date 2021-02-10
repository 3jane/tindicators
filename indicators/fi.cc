#include <new>

#include "../indicators.h"


int ti_fi_start(TI_REAL const *options) {
    return 1;
}


int ti_fi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *close = inputs[0];
    TI_REAL const *volume = inputs[1];
    TI_REAL const period = options[0];
    TI_REAL *fi = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (size <= ti_fi_start(options)) { return TI_OKAY; }

    const TI_REAL per = 2. / (period + 1.);

    TI_REAL ema = volume[1] * (close[1] - close[0]);

    for (int i = 1; i < size; ++i) {
        ema = (volume[i] * (close[i] - close[i-1]) - ema) * per + ema;
        *fi++ = ema;
    }

    return TI_OKAY;
}

int ti_fi_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *close = inputs[0];
    TI_REAL const *volume = inputs[1];
    TI_REAL const period = options[0];
    TI_REAL *fi = outputs[0];

    if (size <= ti_fi_start(options)) { return TI_OKAY; }

    for (int i = 1; i < size; ++i) {
        fi[i-1] = volume[i] * (close[i] - close[i-1]);
    }

    ti_ema(size-1, &fi, &period, &fi);

    return TI_OKAY;
}

struct ti_fi_stream : ti_stream {

    TI_REAL per;
    TI_REAL ema;
    TI_REAL previous_close;
};

int ti_fi_stream_new(TI_REAL const *options, ti_stream **stream) {
    TI_REAL const period = options[0];
    if (period < 1) { return TI_INVALID_OPTION; }
    ti_fi_stream *ptr = new(std::nothrow) ti_fi_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->per = 2. / (period + 1.);
    ptr->index = TI_INDICATOR_FI_INDEX;
    ptr->progress = -1;

    return TI_OKAY;
}

void ti_fi_stream_free(ti_stream *stream) {
    delete static_cast<ti_fi_stream*>(stream);
}

int ti_fi_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_fi_stream *ptr = static_cast<ti_fi_stream*>(stream);
    int progress = ptr->progress;

    TI_REAL const *close = inputs[0];
    TI_REAL const *volume = inputs[1];
    TI_REAL *fi = outputs[0];

    TI_REAL per = ptr->per;
    TI_REAL ema = ptr->ema;
    TI_REAL previous_close = ptr->previous_close;

    int i = 0;
    for (; i < size && progress == -1; ++i, ++progress) {
        previous_close = close[i];
    }
    for (; i < size && progress == 0; ++i, ++progress) {
        ema = volume[i] * (close[i] - previous_close);
        previous_close = close[i];
        *fi++ = ema;
    }
    for (; i < size; ++i, ++progress) {
        ema = (volume[i] * (close[i] - previous_close) - ema) * per + ema;
        previous_close = close[i];
        *fi++ = ema;
    }

    ptr->progress = progress;
    ptr->ema = ema;
    ptr->previous_close = previous_close;

    return TI_OKAY;
}
