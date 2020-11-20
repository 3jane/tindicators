#include "../indicators.h"
#include <new>
#include "../utils/localbuffer.h"
#include "../utils/log.h"

#include <new>
#include <utility>

int ti_ssmooth_start(TI_REAL const *options) {
    return 0;
}

int ti_ssmooth(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *series = inputs[0];
    TI_REAL *ssmooth = outputs[0];

    TI_REAL price1 = 0;
    TI_REAL filt1 = 0;
    TI_REAL filt2 = 0;

    TI_REAL a1 = exp(-1.414*3.14159 / 10);
    TI_REAL b1 = 2*a1*cos(1.414*3.14159 / 10);
    TI_REAL c2 = b1;
    TI_REAL c3 = -a1*a1;
    TI_REAL c1 = 1 - c2 - c3;;

    int i = 0;
    for (; i < size; ++i) {
        TI_REAL price = series[i];
        TI_REAL filt = c1*(price + price1) / 2 + c2*filt1 + c3*filt2;

        price1 = price;
        filt2 = std::exchange(filt1, filt);

        *ssmooth++ = filt;
    }

    return TI_OKAY;
}

struct ti_ssmooth_stream : ti_stream {

    struct {
        
    } options;

    struct {
        TI_REAL price1 = 0;
        TI_REAL filt1 = 0;
        TI_REAL filt2 = 0;
    } state;

    struct {
        TI_REAL a1 = exp(-1.414*3.14159 / 10);
        TI_REAL b1 = 2*a1*cos(1.414*3.14159 / 10);
        TI_REAL c2 = b1;
        TI_REAL c3 = -a1*a1;
        TI_REAL c1 = 1 - c2 - c3;
    } constants;
};

int ti_ssmooth_stream_new(TI_REAL const *options, ti_stream **stream) {
    ti_ssmooth_stream *ptr = new(std::nothrow) ti_ssmooth_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_SSMOOTH_INDEX;
    ptr->progress = -ti_ssmooth_start(options);

    return TI_OKAY;
}

void ti_ssmooth_stream_free(ti_stream *stream) {
    delete static_cast<ti_ssmooth_stream*>(stream);
}

int ti_ssmooth_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_ssmooth_stream *ptr = static_cast<ti_ssmooth_stream*>(stream);
    TI_REAL const *series = inputs[0];
    TI_REAL *ssmooth = outputs[0];
    int progress = ptr->progress;

    TI_REAL price1 = ptr->state.price1;
    TI_REAL filt1 = ptr->state.filt1;
    TI_REAL filt2 = ptr->state.filt2;

    const TI_REAL a1 = ptr->constants.a1;
    const TI_REAL b1 = ptr->constants.b1;
    const TI_REAL c2 = ptr->constants.c2;
    const TI_REAL c3 = ptr->constants.c3;
    const TI_REAL c1 = ptr->constants.c1;

    int i = 0;
    for (; i < size; ++i, ++progress) {
        TI_REAL price = series[i];
        TI_REAL filt = c1*(price + price1) / 2 + c2*filt1 + c3*filt2;

        price1 = price;
        filt2 = std::exchange(filt1, filt);

        *ssmooth++ = filt;
    }

    ptr->progress = progress;
    ptr->state.price1 = price1;
    ptr->state.filt1 = filt1;
    ptr->state.filt2 = filt2;

    return TI_OKAY;
}
