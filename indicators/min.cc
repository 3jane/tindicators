#include "../indicators.h"


int ti_min_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_min(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_min_start(options)) return TI_OKAY;

    int trail = 0, mini = -1;
    TI_REAL min = input[0];
    int i, j;
    for (i = period-1; i < size; ++i, ++trail) {
        TI_REAL bar = input[i];

        if (mini < trail) {
            mini = trail;
            min = input[mini];
            j = trail;
            while(++j <= i) {
                bar = input[j];
                if (bar <= min) {
                    min = bar;
                    mini = j;
                }
            }
        } else if (bar <= min) {
            mini = i;
            min = bar;
        }

        *output++ = min;
    }

    assert(output - outputs[0] == size - ti_min_start(options));
    return TI_OKAY;
}

int ti_min_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_min_start(options)) return TI_OKAY;

    #define MIN(a, b) ((a) < (b) ? (a) : (b))

    for (int i = period-1; i < size; ++i) {
        TI_REAL min = input[i-period+1];
        for (int j = i-period+2; j <= i; ++j) {
            min = MIN(min, input[j]);
        }
        *output++ = min;
    }

    assert(output - outputs[0] == size - ti_min_start(options));
    return TI_OKAY;
}
