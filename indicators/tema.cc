#include "../indicators.h"


int ti_tema_start(TI_REAL const *options) {
    const int period = (int)options[0];
    return (period-1) * 3;
}


int ti_tema(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_tema_start(options)) return TI_OKAY;

    const TI_REAL per = 2 / ((TI_REAL)period + 1);
    const TI_REAL per1 = 1.0 - per;

    /*Calculate EMA(input)*/
    TI_REAL ema = input[0];

    /*Calculate EMA(EMA(input))*/
    TI_REAL ema2 = 0;

    /*Calculate EMA(EMA(EMA(input)))*/
    TI_REAL ema3 = 0;

    int i;
    for (i = 0; i < size; ++i) {
        ema = ema * per1 + input[i] * per;
        if (i == period-1) {
            ema2 = ema;
        }
        if (i >= period-1) {
            ema2 = ema2 * per1 + ema * per;
            if (i == (period-1) * 2) {
                ema3 = ema2;
            }
            if (i >= (period-1) * 2) {
                ema3 = ema3 * per1 + ema2 * per;

                if (i >= (period-1) * 3) {
                    *output = 3 * ema - 3 * ema2 + ema3;
                    ++output;
                }
            }
        }
    }

    assert(output - outputs[0] == size - ti_tema_start(options));
    return TI_OKAY;
}
