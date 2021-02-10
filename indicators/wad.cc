#include "../indicators.h"
#include "../utils/minmax.h"


int ti_wad_start(TI_REAL const *options) {
    (void)options;
    return 1;
}


int ti_wad(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *high = inputs[0];
    const TI_REAL *low = inputs[1];
    const TI_REAL *close = inputs[2];

    (void)options;

    if (size <= ti_wad_start(options)) return TI_OKAY;

    TI_REAL *output = outputs[0];
    TI_REAL sum = 0;
    TI_REAL yc = close[0];

    int i;
    for (i = 1; i < size; ++i) {
        const TI_REAL c = close[i];

        if (c > yc) {
            sum += c - MIN(yc, low[i]);
        } else if (c < yc) {
            sum += c - MAX(yc, high[i]);
        } else {
            /* No change */
        }

        *output++ = sum;

        yc = close[i];
    }


    assert(output - outputs[0] == size - ti_wad_start(options));
    return TI_OKAY;
}
