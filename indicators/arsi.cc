#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

#include <algorithm>

int ti_arsi_start(TI_REAL const *options) {
    const TI_REAL cycpart = options[0];

    return 5;
}

const static TI_REAL PI = acos(-1);

int ti_arsi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const TI_REAL cycpart = options[0];
    TI_REAL *arsi = outputs[0];

    if (cycpart <= 0) { return TI_INVALID_OPTION; }

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

    for (int i = 5; i < size; ++i, step(Smooth,Detrender,I1,Q1,I2,Q2,Re,Im,Period,SmoothPeriod)) {
        Smooth = (4*series[i] + 3*series[i-1] + 2*series[i-2] + series[i-3]) / 10.;
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
        if (Im != 0 && Re != 0) { Period = 2*PI / atan(Im/Re); } // WARNING
        if (Period > 1.5*Period[1]) { Period = 1.5*Period[1]; }
        if (Period < .67*Period[1]) { Period = .67*Period[1]; }
        if (Period < 6) { Period = 6; }
        if (Period > 50) { Period = 50; }
        Period = .2*Period + .8*Period[1];
        SmoothPeriod = .33*Period + .67*SmoothPeriod[1];

        TI_REAL CU = 0;
        TI_REAL CD = 0;
        for (int count = 0; count < (int)(cycpart * SmoothPeriod) && count < i; ++count) {
            CU += std::max<TI_REAL>(0, series[i-count] - series[i-count-1]);
            CD += -std::min<TI_REAL>(0, series[i-count] - series[i-count-1]);
        }
        *arsi++ = CU + CD != 0 ? 100*CU / (CU + CD) : 0;
    }

    return TI_OKAY;
}

DONTOPTIMIZE int ti_arsi_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    const TI_REAL cycpart = options[0];
    TI_REAL *arsi = outputs[0];

    if (cycpart <= 0) { return TI_INVALID_OPTION; }

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

    for (int i = 5; i < size; ++i, step(Smooth,Detrender,I1,Q1,I2,Q2,Re,Im,Period,SmoothPeriod)) {
        Smooth = (4*series[i] + 3*series[i-1] + 2*series[i-2] + series[i-3]) / 10.;
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
        if (Im != 0 && Re != 0) { Period = 2*PI / atan(Im/Re); } // WARNING
        if (Period > 1.5*Period[1]) { Period = 1.5*Period[1]; }
        if (Period < .67*Period[1]) { Period = .67*Period[1]; }
        if (Period < 6) { Period = 6; }
        if (Period > 50) { Period = 50; }
        Period = .2*Period + .8*Period[1];
        SmoothPeriod = .33*Period + .67*SmoothPeriod[1];

        TI_REAL CU = 0;
        TI_REAL CD = 0;
        for (int count = 0; count < (int)(cycpart * SmoothPeriod) && count < i; ++count) {
            CU += std::max<TI_REAL>(0, series[i-count] - series[i-count-1]);
            CD += -std::min<TI_REAL>(0, series[i-count] - series[i-count-1]);
        }
        *arsi++ = CU + CD != 0 ? 100*CU / (CU + CD) : 0;
    }

    return TI_OKAY;
}

struct ti_arsi_stream : ti_stream {

    struct {
        TI_REAL cycpart;
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
        ringbuf<0> Price;
    } state;

    struct {

    } constants;
};

int ti_arsi_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL cycpart = options[0];

    if (cycpart <= 0) { return TI_INVALID_OPTION; }

    ti_arsi_stream *ptr = new(std::nothrow) ti_arsi_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_ARSI_INDEX;
    ptr->progress = -ti_arsi_start(options);

    ptr->options.cycpart = cycpart;

    ptr->state.Price.resize(cycpart * 50 + 1);

    return TI_OKAY;
}

void ti_arsi_stream_free(ti_stream *stream) {
    delete static_cast<ti_arsi_stream*>(stream);
}

int ti_arsi_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_arsi_stream *ptr = static_cast<ti_arsi_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *arsi = outputs[0];
    int progress = ptr->progress;
    const TI_REAL cycpart = ptr->options.cycpart;
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
        Price = series[i];
    }
    for (; i < size; ++i, ++progress, step(Smooth,Detrender,I1,Q1,I2,Q2,Re,Im,Period,SmoothPeriod,Price)) {
        Price = series[i];

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
        if (Im != 0 && Re != 0) { Period = 2*PI / atan(Im/Re); } // WARNING
        if (Period > 1.5*Period[1]) { Period = 1.5*Period[1]; }
        if (Period < .67*Period[1]) { Period = .67*Period[1]; }
        if (Period < 6) { Period = 6; }
        if (Period > 50) { Period = 50; }
        Period = .2*Period + .8*Period[1];
        SmoothPeriod = .33*Period + .67*SmoothPeriod[1];

        TI_REAL CU = 0;
        TI_REAL CD = 0;
        for (int count = 0; count < (int)(cycpart * SmoothPeriod) && count < progress + 5; ++count) {
            CU += std::max<TI_REAL>(0, Price[count] - Price[count+1]);
            CD += -std::min<TI_REAL>(0, Price[count] - Price[count+1]);
        }
        *arsi++ = CU + CD != 0 ? 100*CU / (CU + CD) : 0;
    }

    ptr->progress = progress;

    return TI_OKAY;
}
