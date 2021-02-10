#include "../indicators.h"

#define UPWARD(I) (input[(I)] > input[(I)-1] ? input[(I)] - input[(I)-1] : 0)
#define DOWNWARD(I) (input[(I)] < input[(I)-1] ? input[(I)-1] - input[(I)] : 0)


int ti_cmo_start(TI_REAL const *options) {
    return (int)options[0];
}


int ti_cmo(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    TI_REAL *output = outputs[0];

    const int period = (int)options[0];

    if (period < 1) return TI_INVALID_OPTION;
    if (size <= ti_cmo_start(options)) return TI_OKAY;

    TI_REAL up_sum = 0, down_sum = 0;

    int i = 1;
    for (; i < period && i < size; ++i) {
        up_sum += UPWARD(i);
        down_sum += DOWNWARD(i);
    }
    for (; i < size; ++i) {
        up_sum += UPWARD(i);
        down_sum += DOWNWARD(i);

        *output++ = up_sum + down_sum ? 100 * (up_sum - down_sum) / (up_sum + down_sum) : 0;

        up_sum -= UPWARD(i-period+1);
        down_sum -= DOWNWARD(i-period+1);
    }

    assert(output - outputs[0] == size - ti_cmo_start(options));
    return TI_OKAY;

}
