#include "../indicators.h"
#include <new>
#include "../utils/log.h"

#include <new>
#include <utility>

int ti_roof_start(TI_REAL const *options) {
    return 0;
}

int ti_roof(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *real = inputs[0];
    
    TI_REAL *roof = outputs[0];

    TI_REAL price1 = 0.;
    TI_REAL price2 = 0.;
    TI_REAL HP1 = 0.;
    TI_REAL HP2 = 0.;
    TI_REAL filt1 = 0.;
    TI_REAL filt2 = 0.;

    TI_REAL alpha1 = (cos(.707*2*3.14159 / 48) + sin(.707*2*3.14159 / 48) - 1) / cos(.707*2*3.14159 / 48);
    TI_REAL a1 = exp(-1.414*3.14159 / 10.);
    TI_REAL b1 = 2. * a1 * cos(1.414*3.14159 / 10);
    TI_REAL c2 = b1;
    TI_REAL c3 = -a1*a1;
    TI_REAL c1 = 1. - c2 - c3;

    for (int i = 0; i < size; ++i) {
        TI_REAL price = real[i];
        TI_REAL HP = (1. - alpha1 / 2.)*(1. - alpha1 / 2.)*(price - 2*price1 + price2) + 2*(1 - alpha1)*HP1 - (1 - alpha1)*(1 - alpha1)*HP2;
        TI_REAL filt = c1*(HP + HP1) / 2. + c2*filt1 + c3*filt2;

        HP2 = std::exchange(HP1, HP);
        price2 = std::exchange(price1, price);
        filt2 = std::exchange(filt1, filt);

        *roof++ = filt;
    }

    return TI_OKAY;
}

struct ti_roof_stream : ti_stream {

    struct {
        
    } options;

    struct {
        TI_REAL price1 = 0;
        TI_REAL price2 = 0;
        TI_REAL HP1 = 0;
        TI_REAL HP2 = 0;
        TI_REAL filt1 = 0;
        TI_REAL filt2 = 0;
    } state;

    struct {
        TI_REAL alpha1 = (cos(.707*2*3.14159 / 48) + sin(.707*2*3.14159 / 48) - 1) / cos(.707*2*3.14159 / 48);
        TI_REAL a1 = exp(-1.414*3.14159 / 10.);
        TI_REAL b1 = 2. * a1 * cos(1.414*3.14159 / 10);
        TI_REAL c2 = b1;
        TI_REAL c3 = -a1*a1;
        TI_REAL c1 = 1. - c2 - c3;
    } constants;
};

int ti_roof_stream_new(TI_REAL const *options, ti_stream **stream) {
    ti_roof_stream *ptr = new(std::nothrow) ti_roof_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_ROOF_INDEX;
    ptr->progress = -ti_roof_start(options);

    return TI_OKAY;
}

void ti_roof_stream_free(ti_stream *stream) {
    delete static_cast<ti_roof_stream*>(stream);
}

int ti_roof_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_roof_stream *ptr = static_cast<ti_roof_stream*>(stream);
    TI_REAL const *real = inputs[0];
    TI_REAL *roof = outputs[0];
    int progress = ptr->progress;

    TI_REAL price1 = ptr->state.price1;
    TI_REAL price2 = ptr->state.price2;
    TI_REAL HP1 = ptr->state.HP1;
    TI_REAL HP2 = ptr->state.HP2;
    TI_REAL filt1 = ptr->state.filt1;
    TI_REAL filt2 = ptr->state.filt2;

    TI_REAL alpha1 = ptr->constants.alpha1;
    TI_REAL a1 = ptr->constants.a1;
    TI_REAL b1 = ptr->constants.b1;
    TI_REAL c2 = ptr->constants.c2;
    TI_REAL c3 = ptr->constants.c3;
    TI_REAL c1 = ptr->constants.c1;

    int i = 0;
    for (; i < size; ++i, ++progress) {
        TI_REAL price = real[i];
        TI_REAL HP = (1. - alpha1 / 2.)*(1. - alpha1 / 2.)*(price - 2*price1 + price2) + 2*(1 - alpha1)*HP1 - (1 - alpha1)*(1 - alpha1)*HP2;
        TI_REAL filt = c1*(HP + HP1) / 2. + c2*filt1 + c3*filt2;

        HP2 = std::exchange(HP1, HP);
        price2 = std::exchange(price1, price);
        filt2 = std::exchange(filt1, filt);

        *roof++ = filt;
    }

    ptr->progress = progress;
    ptr->state.price1 = price1;
    ptr->state.price2 = price2;
    ptr->state.HP1 = HP1;
    ptr->state.HP2 = HP2;
    ptr->state.filt1 = filt1;
    ptr->state.filt2 = filt2;

    return TI_OKAY;
}