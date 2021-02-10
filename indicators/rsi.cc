#include "../indicators.h"


int ti_rsi_start(TI_REAL const *options) {
    return (int)options[0];
}


int ti_rsi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];
    const TI_REAL per = 1.0 / ((TI_REAL)period);

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_rsi_start(options)) return TI_OKAY;

    TI_REAL smooth_up = 0, smooth_down = 0;

    int i;
    for (i = 1; i <= period; ++i) {
        const TI_REAL upward = input[i] > input[i-1] ? input[i] - input[i-1] : 0;
        const TI_REAL downward = input[i] < input[i-1] ? input[i-1] - input[i] : 0;
        smooth_up += upward;
        smooth_down += downward;
    }

    smooth_up /= period;
    smooth_down /= period;
    *output++ = smooth_up ? 100.0 * (smooth_up / (smooth_up + smooth_down)) : 0;

    for (i = period+1; i < size; ++i) {
        const TI_REAL upward = input[i] > input[i-1] ? input[i] - input[i-1] : 0;
        const TI_REAL downward = input[i] < input[i-1] ? input[i-1] - input[i] : 0;

        smooth_up = (upward-smooth_up) * per + smooth_up;
        smooth_down = (downward-smooth_down) * per + smooth_down;

        *output++ = smooth_up ? 100.0 * (smooth_up / (smooth_up + smooth_down)) : 0;
    }

    assert(output - outputs[0] == size - ti_rsi_start(options));
    return TI_OKAY;
}
