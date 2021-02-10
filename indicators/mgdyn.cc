#include <new>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_mgdyn_start(TI_REAL const *options) {
    return 0;
}

int ti_mgdyn(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const TI_REAL N = options[0];
    
    TI_REAL *mgdyn = outputs[0];

    TI_REAL filt;

    int i = 0;
    for (; i < 1 && i < size; ++i) {
        filt = series[i];
        *mgdyn++ = filt;
    }
    for (; i < size; ++i) {
        filt = filt + (series[i] - filt)/(N*pow(series[i]/filt, 4));
        *mgdyn++ = filt;        
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_mgdyn_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const TI_REAL N = options[0];
    
    TI_REAL *mgdyn = outputs[0];

    TI_REAL filt;

    int i = 0;
    for (; i < 1 && i < size; ++i) {
        filt = series[i];
        *mgdyn++ = filt;
    }
    for (; i < size; ++i) {
        filt = filt + (series[i] - filt)/(N*pow(series[i]/filt, 4));
        *mgdyn++ = filt;        
    }

    return TI_OKAY;
}

struct ti_mgdyn_stream : ti_stream {

    struct {
        TI_REAL N;
    } options;

    struct {
        TI_REAL filt;
    } state;

    struct {

    } constants;
};

int ti_mgdyn_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL N = options[0];

    ti_mgdyn_stream *ptr = new(std::nothrow) ti_mgdyn_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_MGDYN_INDEX;
    ptr->progress = -ti_mgdyn_start(options);

    ptr->options.N = N;

    ptr->state.filt;

    return TI_OKAY;
}

void ti_mgdyn_stream_free(ti_stream *stream) {
    delete static_cast<ti_mgdyn_stream*>(stream);
}

int ti_mgdyn_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_mgdyn_stream *ptr = static_cast<ti_mgdyn_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *mgdyn = outputs[0];
    const TI_REAL N = ptr->options.N;
    int progress = ptr->progress;
    TI_REAL filt = ptr->state.filt;

    int i = 0;
    for (; progress < 1 && i < size; ++i, ++progress) {
        filt = series[i];
        *mgdyn++ = filt;
    }
    for (; i < size; ++i, ++progress) {
        filt = filt + (series[i] - filt)/(N*pow(series[i]/filt, 4));
        *mgdyn++ = filt;        
    }

    ptr->progress = progress;
    ptr->state.filt = filt;

    return TI_OKAY;
}
