#include "../indicators.h"


int ti_qstick_start(TI_REAL const *options) {
    return (int)options[0]-1;
}


int ti_qstick(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *open = inputs[0];
    const TI_REAL *close = inputs[1];
    TI_REAL *output = outputs[0];
    const int period = (int)options[0];
    const TI_REAL scale = 1.0 / period;

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_qstick_start(options)) return TI_OKAY;

    TI_REAL sum = 0;

    int i;
    for (i = 0; i < period; ++i) {
        sum += close[i] - open[i];
    }

    *output++ = sum * scale;

    for (i = period; i < size; ++i) {
        sum += close[i] - open[i];
        sum -= close[i-period] - open[i-period];
        *output++ = sum * scale;
    }

    assert(output - outputs[0] == size - ti_qstick_start(options));
    return TI_OKAY;
}
