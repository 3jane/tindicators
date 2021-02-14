/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include "../indicators.h"
#include <new>


int ti_msw_start(TI_REAL const *options) {
    return (int)options[0];
}


int ti_msw(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    TI_REAL *sine = outputs[0];
    TI_REAL *lead = outputs[1];
    const int period = (int)options[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_msw_start(options)) return TI_OKAY;

    const TI_REAL pi = 3.1415926;
    const TI_REAL tpi = 2 * pi;

    TI_REAL weight = 0, phase;
    TI_REAL rp, ip;

    int i, j;
    for (i = period; i < size; ++i) {
        rp = 0;
        ip = 0;
        for (j = 0; j < period; ++j) {
            weight = input[i-j];
            rp = rp + cos(tpi * j / period) * weight;
            ip = ip + sin(tpi * j / period) * weight;
        }
        if (fabs(rp) > .001) {
            phase = atan(ip/rp);
        } else {
            phase = tpi / 2.0 * (ip < 0 ? -1.0 : 1.0);
        }
        if (rp < 0.0) phase += pi;
        phase += pi/2.0;
        if (phase < 0.0) phase += tpi;
        if (phase > tpi) phase -= tpi;
        /* phase = 180 * phase / pi; */

        *sine++ = sin(phase);
        *lead++ = sin(phase + pi/4.0);
    }


    assert(sine - outputs[0] == size - ti_msw_start(options));
    assert(lead - outputs[1] == size - ti_msw_start(options));
    return TI_OKAY;
}
