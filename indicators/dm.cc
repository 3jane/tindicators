#include "../indicators.h"
#include "dx.h"


int ti_dm_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_dm(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];

    const int period = (int)options[0];

    TI_REAL *plus_dm = outputs[0];
    TI_REAL *minus_dm = outputs[1];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_dm_start(options)) return TI_OKAY;

    const TI_REAL per = ((TI_REAL)period-1) / ((TI_REAL)period);

    TI_REAL dmup = 0;
    TI_REAL dmdown = 0;

    int i;
    for (i = 1; i < period; ++i) {
        TI_REAL dp, dm;
        CALC_DIRECTION(dp, dm);

        dmup += dp;
        dmdown += dm;
    }


    *plus_dm++ = dmup;
    *minus_dm++ = dmdown;


    for (i = period; i < size; ++i) {
        TI_REAL dp, dm;
        CALC_DIRECTION(dp, dm);


        dmup = dmup * per + dp;
        dmdown = dmdown * per + dm;

        *plus_dm++ = dmup;
        *minus_dm++ = dmdown;
    }


    assert(plus_dm - outputs[0] == size - ti_dm_start(options));
    assert(minus_dm - outputs[1] == size - ti_dm_start(options));
    return TI_OKAY;
}
