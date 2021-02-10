#include <new>

#include "../indicators.h"
#include "dx.h"
#include "../utils/buffer.h"


int ti_adxr_start(TI_REAL const *options) {
    return ((int)options[0]-1) * 3;
}


int ti_adxr(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];

    const int period = (int)options[0];

    TI_REAL *output = outputs[0];

    if (period < 2) return TI_INVALID_OPTION;
    if (size <= ti_adxr_start(options)) return TI_OKAY;

    const TI_REAL per = ((TI_REAL)period-1) / ((TI_REAL)period);
    const TI_REAL invper = 1.0 / ((TI_REAL)period);


    TI_REAL dmup = 0;
    TI_REAL dmdown = 0;

    int i;
    for (i = 1; i < period; ++i) {
        TI_REAL dp, dm;
        CALC_DIRECTION(dp, dm);

        dmup += dp;
        dmdown += dm;
    }

    TI_REAL adx = 0.0;

    {
        TI_REAL di_up = dmup;
        TI_REAL di_down = dmdown;
        TI_REAL dm_diff = fabs(di_up - di_down);
        TI_REAL dm_sum = di_up + di_down;
        TI_REAL dx = dm_diff / dm_sum * 100;
        if (dm_sum == 0) { dx = 0; }

        adx += dx;
    }

    ti_buffer *adxr = ti_buffer_new(period-1);
    const int first_adxr = ti_adxr_start(options);

    for (i = period; i < size; ++i) {

        TI_REAL dp, dm;
        CALC_DIRECTION(dp, dm);


        dmup = dmup * per + dp;
        dmdown = dmdown * per + dm;


        TI_REAL di_up = dmup;
        TI_REAL di_down = dmdown;
        TI_REAL dm_diff = fabs(di_up - di_down);
        TI_REAL dm_sum = di_up + di_down;
        TI_REAL dx = dm_diff / dm_sum * 100;
        if (dm_sum == 0) { dx = 0; }


        if (i-period < period-2) {
            adx += dx;
        } else if (i-period == period-2) {
            adx += dx;
            ti_buffer_qpush(adxr, adx * invper);
        } else {
            adx = adx * per + dx;

            if (i >= first_adxr) {
                *output++ = 0.5 * (adx * invper + ti_buffer_get(adxr, 1));
            }

            ti_buffer_qpush(adxr, adx * invper);
        }

    }

    ti_buffer_free(adxr);

    assert(output - outputs[0] == size - ti_adxr_start(options));
    return TI_OKAY;
}
