#include "../indicators.h"


int ti_adosc_start(TI_REAL const *options) {
    return (int)(options[1])-1;
}


int ti_adosc(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];
    const TI_REAL *volume = inputs[3];

    const int short_period = (int)options[0];
    const int long_period = (int)options[1];

    const int start = long_period - 1;

    if (short_period < 1) return TI_INVALID_OPTION;
    if (long_period < short_period) return TI_INVALID_OPTION;
    if (size <= ti_adosc_start(options)) return TI_OKAY;

    const TI_REAL short_per = 2 / ((TI_REAL)short_period + 1);
    const TI_REAL long_per = 2 / ((TI_REAL)long_period + 1);

    TI_REAL *output = outputs[0];
    TI_REAL sum = 0, short_ema = 0, long_ema = 0;
    int i;
    for (i = 0; i < size; ++i) {
        const TI_REAL hl = (high[i] - low[i]);
        if (hl != 0.0) {
            sum += (close[i] - low[i] - high[i] + close[i]) / hl * volume[i];
        }

        if (i == 0) {
            short_ema = sum;
            long_ema = sum;
        } else {
            short_ema = (sum-short_ema) * short_per + short_ema;
            long_ema = (sum-long_ema) * long_per + long_ema;
        }

        if (i >= start) {
            *output++ = short_ema - long_ema;
        }
    }

    assert(output - outputs[0] == size - ti_adosc_start(options));

    return TI_OKAY;
}

