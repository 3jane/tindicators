#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

#include <algorithm>

int ti_hd_start(TI_REAL const *options) {
    return 5;
}

static const TI_REAL PI = acos(-1);

int ti_hd(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    
    TI_REAL *hd = outputs[0];

    ringbuf<7> Smooth;
    ringbuf<7> Detrender;
    ringbuf<7> I1;
    ringbuf<7> Q1;
    ringbuf<2> I2;
    ringbuf<2> Q2;
    ringbuf<2> Re;
    ringbuf<2> Im;
    ringbuf<2> Period;
    ringbuf<2> SmoothPeriod;
    ringbuf<4> Price;

    int i = 0;
    for (; i < 5 && i < size; ++i, step(Price)) {
        Price = real[i];
    }
    for (; i < size; ++i, step(Smooth,Detrender,I1,Q1,I2,Q2,Re,Im,Period,SmoothPeriod,Price)) {
        Price = real[i];

        Smooth = (4*Price + 3*Price[1] + 2*Price[2] + Price[3]) / 10.;
        Detrender = (.0962*Smooth + .5769*Smooth[2] - .5769*Smooth[4] - .0962*Smooth[6]) * (.075*Period[1] + .54);

        Q1 = (.0962*Detrender + .5769*Detrender[2] - .5769*Detrender[4] - .0962*Detrender[6]) * (.075*Period[1] + .54);
        I1 = Detrender[3];

        TI_REAL jI = (.0962*I1 + .5769*I1[2] - .5769*I1[4] - .0962*I1[6]) * (.075*Period[1] + .54);
        TI_REAL jQ = (.0962*Q1 + .5769*Q1[2] - .5769*Q1[4] - .0962*Q1[6]) * (.075*Period[1] + .54);

        I2 = I1 - jQ;
        Q2 = Q1 + jI;

        I2 = .2*I2 + .8*I2[1];
        Q2 = .2*Q2 + .8*Q2[1];

        Re = I2*I2[1] + Q2*Q2[1];
        Im = I2*Q2[1] - Q2*I2[1];
        Re = .2*Re + .8*Re[1];
        Im = .2*Im + .8*Im[1];
        if (Im != 0 && Re != 0) { Period = 2*PI / atan(Im/Re); }
        Period = std::min<TI_REAL>(Period, 1.5*Period[1]);
        Period = std::max<TI_REAL>(Period, .67*Period[1]);
        Period = std::max<TI_REAL>(Period, 6);
        Period = std::min<TI_REAL>(Period, 50);
        Period = .2*Period + .8*Period[1];
        SmoothPeriod = .33*Period + .67*SmoothPeriod[1];

        *hd++ = SmoothPeriod;
    }


    return TI_OKAY;
}

DONTOPTIMIZE int ti_hd_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    
    TI_REAL *hd = outputs[0];

    ringbuf<7> Smooth;
    ringbuf<7> Detrender;
    ringbuf<7> I1;
    ringbuf<7> Q1;
    ringbuf<1> jI;
    ringbuf<1> jQ;
    ringbuf<2> I2;
    ringbuf<2> Q2;
    ringbuf<2> Re;
    ringbuf<2> Im;
    ringbuf<2> Period;
    ringbuf<2> SmoothPeriod;
    ringbuf<4> Price;

    int i = 0;
    for (; i < 5 && i < size; ++i, step(Price)) {
        Price = real[i];
    }
    for (; i < size; ++i, step(Smooth,Detrender,I1,Q1,jI,jQ,I2,Q2,Re,Im,Period,SmoothPeriod,Price)) {
        Price = real[i];

        Smooth = (4*Price + 3*Price[1] + 2*Price[2] + Price[3]) / 10.;
        Detrender = (.0962*Smooth + .5769*Smooth[2] - .5769*Smooth[4] - .0962*Smooth[6]) * (.075*Period[1] + .54);

        Q1 = (.0962*Detrender + .5769*Detrender[2] - .5769*Detrender[4] - .0962*Detrender[6]) * (.075*Period[1] + .54);
        I1 = Detrender[3];

        jI = (.0962*I1 + .5769*I1[2] - .5769*I1[4] - .0962*I1[6]) * (.075*Period[1] + .54);
        jQ = (.0962*Q1 + .5769*Q1[2] - .5769*Q1[4] - .0962*Q1[6]) * (.075*Period[1] + .54);

        I2 = I1 - jQ;
        Q2 = Q1 + jI;

        I2 = .2*I2 + .8*I2[1];
        Q2 = .2*Q2 + .8*Q2[1];

        Re = I2*I2[1] + Q2*Q2[1];
        Im = I2*Q2[1] - Q2*I2[1];
        Re = .2*Re + .8*Re[1];
        Im = .2*Im + .8*Im[1];
        if (Im != 0 && Re != 0) { Period = 2*PI / atan(Im/Re); }
        Period = std::min<TI_REAL>(Period, 1.5*Period[1]);
        Period = std::max<TI_REAL>(Period, .67*Period[1]);
        Period = std::max<TI_REAL>(Period, 6);
        Period = std::min<TI_REAL>(Period, 50);
        Period = .2*Period + .8*Period[1];
        SmoothPeriod = .33*Period + .67*SmoothPeriod[1];

        *hd++ = SmoothPeriod;
    }

    return TI_OKAY;
}

struct ti_hd_stream : ti_stream {

    struct {
        
    } options;

    struct {
        ringbuf<7> Smooth;
        ringbuf<7> Detrender;
        ringbuf<7> I1;
        ringbuf<7> Q1;
        ringbuf<2> I2;
        ringbuf<2> Q2;
        ringbuf<2> Re;
        ringbuf<2> Im;
        ringbuf<2> Period;
        ringbuf<2> SmoothPeriod;
        ringbuf<4> Price;
    } state;

    struct {

    } constants;
};

int ti_hd_stream_new(TI_REAL const *options, ti_stream **stream) {
    ti_hd_stream *ptr = new(std::nothrow) ti_hd_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_HD_INDEX;
    ptr->progress = -ti_hd_start(options);

    return TI_OKAY;
}

void ti_hd_stream_free(ti_stream *stream) {
    delete static_cast<ti_hd_stream*>(stream);
}

int ti_hd_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
ti_hd_stream *ptr = static_cast<ti_hd_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *hd = outputs[0];
    int progress = ptr->progress;
    auto &Smooth = ptr->state.Smooth;
    auto &Detrender = ptr->state.Detrender;
    auto &I1 = ptr->state.I1;
    auto &Q1 = ptr->state.Q1;
    auto &I2 = ptr->state.I2;
    auto &Q2 = ptr->state.Q2;
    auto &Re = ptr->state.Re;
    auto &Im = ptr->state.Im;
    auto &Period = ptr->state.Period;
    auto &SmoothPeriod = ptr->state.SmoothPeriod;
    auto &Price = ptr->state.Price;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(Price)) {
        Price = real[i];
    }
    for (; i < size; ++i, ++progress, step(Smooth,Detrender,I1,Q1,I2,Q2,Re,Im,Period,SmoothPeriod,Price)) {
        Price = real[i];

        Smooth = (4*Price + 3*Price[1] + 2*Price[2] + Price[3]) / 10.;
        Detrender = (.0962*Smooth + .5769*Smooth[2] - .5769*Smooth[4] - .0962*Smooth[6]) * (.075*Period[1] + .54);

        Q1 = (.0962*Detrender + .5769*Detrender[2] - .5769*Detrender[4] - .0962*Detrender[6]) * (.075*Period[1] + .54);
        I1 = Detrender[3];

        TI_REAL jI = (.0962*I1 + .5769*I1[2] - .5769*I1[4] - .0962*I1[6]) * (.075*Period[1] + .54);
        TI_REAL jQ = (.0962*Q1 + .5769*Q1[2] - .5769*Q1[4] - .0962*Q1[6]) * (.075*Period[1] + .54);

        I2 = I1 - jQ;
        Q2 = Q1 + jI;

        I2 = .2*I2 + .8*I2[1];
        Q2 = .2*Q2 + .8*Q2[1];

        Re = I2*I2[1] + Q2*Q2[1];
        Im = I2*Q2[1] - Q2*I2[1];
        Re = .2*Re + .8*Re[1];
        Im = .2*Im + .8*Im[1];
        if (Im != 0 && Re != 0) { Period = 2*PI / atan(Im/Re); }
        Period = std::min<TI_REAL>(Period, 1.5*Period[1]);
        Period = std::max<TI_REAL>(Period, .67*Period[1]);
        Period = std::max<TI_REAL>(Period, 6);
        Period = std::min<TI_REAL>(Period, 50);
        Period = .2*Period + .8*Period[1];
        SmoothPeriod = .33*Period + .67*SmoothPeriod[1];

        *hd++ = SmoothPeriod;
    }

    ptr->progress = progress;

    return TI_OKAY;
}
