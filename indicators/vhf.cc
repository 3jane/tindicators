#include <new>

#include "../indicators.h"


int ti_vhf_start(TI_REAL const *options) {
    return (int)options[0];
}


int ti_vhf(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *in = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_vhf_start(options)) return TI_OKAY;

    int trail = 1, maxi = -1, mini = -1;
    TI_REAL max = in[0], min = in[0];
    TI_REAL bar;

    TI_REAL sum = 0;

    int i, j;

    TI_REAL yc = in[0];
    TI_REAL c;

    for (i = 1; i < period; ++i) {
        c = in[i];
        sum += fabs(c - yc);
        yc = c;
    }

    for (i = period; i < size; ++i, ++trail) {
        c = in[i];
        sum += fabs(c - yc);
        yc = c;

        if (i > period) {
            sum -= fabs(in[i-period] - in[i-period-1]);
        }

        /* Maintain highest. */
        bar = c;
        if (maxi < trail) {
            maxi = trail;
            max = in[maxi];
            j = trail;
            while(++j <= i) {
                bar = in[j];
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
        bar = c;
        if (mini < trail) {
            mini = trail;
            min = in[mini];
            j = trail;
            while(++j <= i) {
                bar = in[j];
                if (bar <= min) {
                    min = bar;
                    mini = j;
                }
            }
        } else if (bar <= min) {
            mini = i;
            min = bar;
        }

        /* Calculate it. */
        *output++ = fabs(max - min) ? fabs(max - min) / sum : 0;
    }

    assert(output - outputs[0] == size - ti_vhf_start(options));
    return TI_OKAY;
}
