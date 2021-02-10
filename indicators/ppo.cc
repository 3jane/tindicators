#include "../indicators.h"


int ti_ppo_start(TI_REAL const *options) {
    (void)options;
    return 1;
}


int ti_ppo(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];

    TI_REAL *ppo = outputs[0];

    const int short_period = (int)options[0];
    const int long_period = (int)options[1];

    if (short_period < 1) return TI_INVALID_OPTION;
    if (long_period < 2) return TI_INVALID_OPTION;
    if (long_period < short_period) return TI_INVALID_OPTION;

    if (size <= ti_ppo_start(options)) return TI_OKAY;

    TI_REAL short_per = 2 / ((TI_REAL)short_period + 1);
    TI_REAL long_per = 2 / ((TI_REAL)long_period + 1);

    TI_REAL short_ema = input[0];
    TI_REAL long_ema = input[0];

    int i;
    for (i = 1; i < size; ++i) {
        short_ema = (input[i]-short_ema) * short_per + short_ema;
        long_ema = (input[i]-long_ema) * long_per + long_ema;
        const TI_REAL out = short_ema - long_ema ? 100.0 * (short_ema - long_ema) / long_ema : 0;

        *ppo++ = out;
    }

    assert(ppo - outputs[0] == size - ti_ppo_start(options));
    return TI_OKAY;
}
