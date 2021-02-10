/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"

/* Source:
 * Kirkpatrick, Dahlquist. Technical Analysis: The Complete Resource for Financial Market Technicians (2 ed.)
 * ISBN 978-0-13-705944-7
 * pp. 419, 421
 * */

int ti_cmf_start(TI_REAL const *options) {
    return (int)options[0]-1;
}

int ti_cmf(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];
    const TI_REAL *volume = inputs[3];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_wma_start(options)) return TI_OKAY;

    #define CHAIKIN_AD(i) (high[i] - low[i] ? volume[i] * ((close[i] - low[i]) - (high[i] - close[i])) / (high[i] - low[i]) : 0.)

    TI_REAL period_volume = 0;
    TI_REAL period_ad_sum = 0;
    int i;
    for (i = 0; i < period-1; ++i) {
        period_ad_sum += CHAIKIN_AD(i);
        period_volume += volume[i];
    }

    for (i = period-1; i < size; ++i) {
        period_ad_sum += CHAIKIN_AD(i);
        period_volume += volume[i];

        *output++ = (period_volume ? period_ad_sum / period_volume : 0);

        period_ad_sum -= CHAIKIN_AD(i-period+1);
        period_volume -= volume[i-period+1];
    }

    #undef CHAIKIN_AD

    assert(output - outputs[0] == size - ti_cmf_start(options));
    return TI_OKAY;
}
