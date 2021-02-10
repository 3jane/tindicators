#include <new>

#include "../indicators.h"
#include "truerange.h"


int ti_natr_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_natr(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];

    const int period = (int)options[0];

    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_natr_start(options)) return TI_OKAY;

    const TI_REAL per = 1.0 / ((TI_REAL)period);

    TI_REAL sum = 0;
    TI_REAL truerange;

    sum += high[0] - low[0];
    int i;
    for (i = 1; i < period; ++i) {
        CALC_TRUERANGE();
        sum += truerange;
    }


    TI_REAL val = sum / period;
    *output++ = val ? 100 * (val) / close[period-1] : 0;

    for (i = period; i < size; ++i) {
        CALC_TRUERANGE();
        val = (truerange-val) * per + val;
        *output++ = val ? 100 * (val) / close[i] : 0;
    }


    assert(output - outputs[0] == size - ti_natr_start(options));
    return TI_OKAY;
}
