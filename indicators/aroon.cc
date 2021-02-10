/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include "../indicators.h"


int ti_aroon_start(TI_REAL const *options) {
    return (int)options[0];
}


int ti_aroon(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    TI_REAL *adown = outputs[0];
    TI_REAL *aup = outputs[1];
    const int period = (int)options[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_aroon_start(options)) return TI_OKAY;

    const TI_REAL scale = 100.0 / period;

    int trail = 0, maxi = -1, mini = -1;
    TI_REAL max = high[0];
    TI_REAL min = low[0];
    TI_REAL bar;

    int i, j;
    for (i = period; i < size; ++i, ++trail) {
        /* Maintain highest. */
        bar = high[i];
        if (maxi < trail) {
            maxi = trail;
            max = high[maxi];
            j = trail;
            while(++j <= i) {
                bar = high[j];
                if (bar >= max) {
                    max = bar;
                    maxi = j;
                }
            }
        } else if (bar >= max) {
            maxi = i;
            max = bar;
        }


        /* Maintain lowest. */
        bar = low[i];
        if (mini < trail) {
            mini = trail;
            min = low[mini];
            j = trail;
            while(++j <= i) {
                bar = low[j];
                if (bar <= min) {
                    min = bar;
                    mini = j;
                }
            }
        } else if (bar <= min) {
            mini = i;
            min = bar;
        }


        /* Calculate the indicator. */
        *adown++ = ((TI_REAL)period - (i-mini)) * scale;
        *aup++ = ((TI_REAL)period - (i-maxi)) * scale;
    }


    assert(adown - outputs[0] == size - ti_aroon_start(options));
    assert(aup - outputs[1] == size - ti_aroon_start(options));
    return TI_OKAY;
}
