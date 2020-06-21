#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_pvt_start(TI_REAL const *options) {
    return 1;
}

int ti_pvt(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const close = inputs[0];
    TI_REAL const *const volume = inputs[1];
    
    TI_REAL *pvt = outputs[0];

    int i = 1;
    for (; i < 2 && i < size; ++i) {
        *pvt++ = volume[i] && volume[i-1] ? volume[i] * (close[i] - close[i-1]) / close[i-1] + volume[i-1] : 0;
    }
    for (; i < size; i++) {
        *pvt = volume[i] && volume[i-1] ? *(pvt - 1) + volume[i] * (close[i] - close[i-1]) / close[i-1] : 0;
        pvt++;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_pvt_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const close = inputs[0];
    TI_REAL const *const volume = inputs[1];
    
    TI_REAL *pvt = outputs[0];

    *pvt++ = volume[1] * (close[1] - close[0]) / close[0] + volume[0];
    for (int i = 2; i < size; i++) {
        *pvt = *(pvt - 1) + volume[i] * (close[i] - close[i - 1]) / close[i - 1];
        pvt++;
    }

    return TI_OKAY;
}

struct ti_pvt_stream : ti_stream {

    struct {
        TI_REAL last_close;
        TI_REAL starting;
        TI_REAL last_pvt;
    } state;
};

int ti_pvt_stream_new(TI_REAL const *options, ti_stream **stream) {

    ti_pvt_stream *ptr = new(std::nothrow) ti_pvt_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_PVT_INDEX;
    ptr->progress = -ti_pvt_start(options);

    return TI_OKAY;
}

void ti_pvt_stream_free(ti_stream *stream) {
    delete static_cast<ti_pvt_stream*>(stream);
}

int ti_pvt_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_pvt_stream *ptr = static_cast<ti_pvt_stream*>(stream);
    TI_REAL const *const close = inputs[0];
    TI_REAL const *const volume = inputs[1];
    TI_REAL *pvt = outputs[0];
    int progress = ptr->progress;
    TI_REAL last_close = ptr->state.last_close;
    TI_REAL last_pvt = ptr->state.last_pvt;
    TI_REAL starting = ptr->state.starting;

    int index = 0;
    
    if (progress < 0 && index < size) {
        last_close = close[index];
        starting = volume[index];
        index++;
        progress++;
    } 
 
    if (progress == 0 && index < size) {
        last_pvt = volume[index] && starting ? volume[index] * (close[index] - last_close) / last_close + starting : 0;
        last_close = close[index];

        *pvt++ = last_pvt;

        index++;
        progress++;
    }

    for (; index < size; index++) {
        last_pvt += volume[index] && last_pvt ? volume[index] * (close[index] - last_close) / last_close : 0;
        last_close = close[index];

        *pvt++ = last_pvt;

        progress++;   
    }

    ptr->progress = progress;
    ptr->state.last_close = last_close;
    ptr->state.last_pvt = last_pvt;
    ptr->state.starting = starting;
  
    return TI_OKAY;
}

