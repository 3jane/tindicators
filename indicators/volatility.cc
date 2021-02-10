#include <new>

#include "../indicators.h"

#define CHANGE(i) (input[i-1] ? input[i]/input[i-1]-1.0 : 0)



int ti_volatility_start(TI_REAL const *options) {
    return (int)options[0];
}


int ti_volatility(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    TI_REAL *output = outputs[0];
    const int period = (int)options[0];
    const TI_REAL scale = 1.0 / period;
    const TI_REAL annual = sqrt(252); /* Multiplier, number of trading days in year. */

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_volatility_start(options)) return TI_OKAY;

    TI_REAL sum = 0;
    TI_REAL sum2 = 0;

    int i;
    for (i = 1; i <= period; ++i) {
        const TI_REAL c = CHANGE(i);
        sum += c;
        sum2 += c * c;
    }

    *output++ = sqrt(sum2 * scale - (sum * scale) * (sum * scale)) * annual;

    for (i = period+1; i < size; ++i) {
        const TI_REAL c = CHANGE(i);
        sum += c;
        sum2 += c * c;

        const TI_REAL cp = CHANGE(i-period);
        sum -= cp;
        sum2 -= cp * cp;

        *output++ = sqrt(sum2 * scale - (sum * scale) * (sum * scale)) * annual;
    }

    assert(output - outputs[0] == size - ti_volatility_start(options));
    return TI_OKAY;

}
