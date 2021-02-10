#include "../indicators.h"


#define INIT() const TI_REAL p = (1.0 / (period)); TI_REAL tsf = 0;

#define FINAL(forecast) do { \
        const TI_REAL a = (y - b * x) * p; \
        if (i >= (period)) {*output++ = input[i] - tsf ? 100 * (input[i] - tsf) / input[i] : 0;} \
        tsf = (a + b * (forecast)); \
} while (0)

#include "trend.h"



int ti_fosc_start(TI_REAL const *options) {
    return (int)options[0];
}


int ti_fosc(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_fosc_start(options)) return TI_OKAY;

    LINEAR_REGRESSION(size, input, period, output, period+1);

    assert(output - outputs[0] == size - ti_fosc_start(options));
    return TI_OKAY;
}
