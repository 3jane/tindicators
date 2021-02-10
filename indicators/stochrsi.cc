/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#include "../indicators.h"
#include "../utils/buffer.h"


int ti_stochrsi_start(TI_REAL const *options) {
    return ((int)options[0]) * 2 - 1;
}


int ti_stochrsi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    const TI_REAL *input = inputs[0];
    const int period = (int)options[0];
    TI_REAL *output = outputs[0];
    const TI_REAL per = 1.0 / ((TI_REAL)period);

    if (period < 2) return TI_INVALID_OPTION; /* If period = 0 then min-max = 0. */
    if (size <= ti_stochrsi_start(options)) return TI_OKAY;

    ti_buffer *rsi = ti_buffer_new(period);

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
    TI_REAL r = smooth_up ? 100.0 * (smooth_up / (smooth_up + smooth_down)) : 0;
    ti_buffer_push(rsi, r);

    TI_REAL min = r;
    TI_REAL max = r;
    int mini = 0;
    int maxi = 0;

    for (i = period+1; i < size; ++i) {
        const TI_REAL upward = input[i] > input[i-1] ? input[i] - input[i-1] : 0;
        const TI_REAL downward = input[i] < input[i-1] ? input[i-1] - input[i] : 0;

        smooth_up = (upward-smooth_up) * per + smooth_up;
        smooth_down = (downward-smooth_down) * per + smooth_down;

        r = smooth_up ? 100.0 * (smooth_up / (smooth_up + smooth_down)) : 0;

        if (r > max) {
            max = r;
            maxi = rsi->index;
        } else if (maxi == rsi->index) {
            max = r;
            int j;
            for (j = 0; j < rsi->size; ++j) {
                if (j == rsi->index) continue;
                if (rsi->vals[j] > max) {
                    max = rsi->vals[j];
                    maxi = j;
                }
            }
        }

        if (r < min) {
            min = r;
            mini = rsi->index;
        } else if (mini == rsi->index) {
            min = r;
            int j;
            for (j = 0; j < rsi->size; ++j) {
                if (j == rsi->index) continue;
                if (rsi->vals[j] < min) {
                    min = rsi->vals[j];
                    mini = j;
                }
            }
        }

        ti_buffer_qpush(rsi, r);

        if (i > period*2 - 2) {
            const TI_REAL diff = max - min;
            if (diff == 0.0) {
                *output++ = 0.0;
            } else {
                *output++ = (r - min) / (diff);
            }
        }
    }

    ti_buffer_free(rsi);
    assert(output - outputs[0] == size - ti_stochrsi_start(options));
    return TI_OKAY;
}
