/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019-2021
 */


#include <queue>
#include <new>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"


int ti_frama_start(TI_REAL const *options) {
    TI_REAL period = options[0];
    TI_REAL average_period = options[1];

    return 0;
}

// original author: @HelloCreepy
int ti_frama(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL period = options[0];
    TI_REAL average_period = options[1];
    TI_REAL *frama = outputs[0];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (average_period < 2) { return TI_INVALID_OPTION; }
    if ((int)period % 2 != 0) { return TI_INVALID_OPTION; }
    if (size <= ti_frama_start(options)) { return TI_OKAY; }

    const TI_REAL w = log(2.0 / (1.0 + average_period));
    const int half_period = period / 2;

    TI_REAL filt;

    int i, j;
    for (i = 0; i < size && i < period; ++i) {
        /* our first data point just return (High + Low) / 2 */
        filt = (high[i] + low[i]) / 2;
        *frama++ = filt;
    }
    for (i = period; i < size; ++i) {
        /* Index of rolling window center, start and end */
        int window_center = i - half_period + 1,
        window_start = i - period + 1;

        TI_REAL highest = high[window_start], highest_second_half, highest_first_half;
        TI_REAL lowest = low[window_start], lowest_second_half, lowest_first_half;


        for (j = window_start; j <= i; ++j) {
            TI_REAL current_high = high[j];
            TI_REAL current_low = low[j];

            /* Highest and lowest calc of first period half */
            if (j == window_center) {
                highest_first_half = highest;
                lowest_first_half = lowest;
            }

            /* Highest and lowest calc of all period */
            if (current_high > highest) {
                highest = current_high;
            }
            if (current_low < lowest) {
                lowest = current_low;
            }

            /* Highest and lowest calc of second period half */
            if (j >= window_center) {
                /* Set inital values for second half  */
                if (j == window_center) {
                    highest_second_half = current_high;
                    lowest_second_half = current_low;
                } else {
                    if (current_high > highest_second_half) {
                        highest_second_half = current_high;
                    }
                    if (current_low < lowest_second_half) {
                        lowest_second_half = current_low;
                    }
                }
            }
        }


        const TI_REAL n3 = (highest - lowest) / period;
        const TI_REAL n2 = (highest_first_half - lowest_first_half) / half_period;
        const TI_REAL n1 = (highest_second_half - lowest_second_half) / half_period;

        TI_REAL dimension = 0;
        if ((n1 > 0) && (n2 > 0) && (n3 > 0)) {
            dimension = (log(n1 + n2) - log(n3)) / log(2);
        }

        TI_REAL alpha = exp(w * (dimension - 1));
        if (alpha < 0.01) { alpha = 0.01 ; }
        if (alpha > 1) { alpha = 1 ; }

        filt = alpha * (high[i] + low[i]) / 2 + (1 - alpha) * filt;
        *frama++ = filt;
    }

    return TI_OKAY;
}

struct ti_frama_stream : ti_stream {

    struct {
        TI_REAL period;
        TI_REAL average_period;
    } options;

    struct {
        TI_REAL filt;

        TI_REAL max1;
        TI_REAL max2;
        TI_REAL min1;
        TI_REAL min2;
        int max1_idx;
        int max2_idx;
        int min1_idx;
        int min2_idx;
    } state;

    std::deque<TI_REAL> high_store;
    std::deque<TI_REAL> low_store;

    struct {
        TI_REAL w;
    } constants;
};

int ti_frama_stream_new(TI_REAL const *options, ti_stream **stream) {
    TI_REAL period = options[0];
    TI_REAL average_period = options[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    if ((int)period % 2 != 0) { return TI_INVALID_OPTION; }

    ti_frama_stream *ptr = new(std::nothrow) ti_frama_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_FRAMA_INDEX;
    ptr->progress = -ti_frama_start(options);

    ptr->options.period = period;
    ptr->options.average_period = average_period;

    ptr->constants.w = log(2.0 / (1.0 + average_period));

    return TI_OKAY;
}

void ti_frama_stream_free(ti_stream *stream) {
    delete static_cast<ti_frama_stream*>(stream);
}

int ti_frama_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_frama_stream *ptr = static_cast<ti_frama_stream*>(stream);
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL *frama = outputs[0];
    int progress = ptr->progress;
    TI_REAL period = ptr->options.period; // N in the paper
    TI_REAL average_period = ptr->options.average_period;

    std::deque<TI_REAL> &high_store = ptr->high_store;
    std::deque<TI_REAL> &low_store = ptr->low_store;

    TI_REAL w = ptr->constants.w;
    TI_REAL filt = ptr->state.filt;

    TI_REAL max1 = ptr->state.max1;
    TI_REAL max2 = ptr->state.max2;
    TI_REAL min1 = ptr->state.min1;
    TI_REAL min2 = ptr->state.min2;
    int max1_idx = ptr->state.max1_idx;
    int max2_idx = ptr->state.max2_idx;
    int min1_idx = ptr->state.min1_idx;
    int min2_idx = ptr->state.min2_idx;

    int i = 0;
    for (; i < size && progress == 0; ++i, ++progress) {
        high_store.push_front(high[i]);
        low_store.push_front(low[i]);
        filt = (high[i] + low[i]) / 2.;
        *frama++ = filt;

        max2 = high[i];
        min2 = low[i];
        max2_idx = progress;
        min2_idx = progress;
    }
    for (; i < size && progress < period/2; ++i, ++progress) {
        high_store.push_front(high[i]);
        low_store.push_front(low[i]);
        filt = (high[i] + low[i]) / 2.;
        *frama++ = filt;

        if (max2 <= high[i]) {
            max2 = high[i];
            max2_idx = progress;
        }
        if (min2 >= low[i]) {
            min2 = low[i];
            min2_idx = progress;
        }
    }
    for (; i < size && progress == (int)period/2; ++i, ++progress) {
        high_store.push_front(high[i]);
        low_store.push_front(low[i]);
        filt = (high[i] + low[i]) / 2.;
        *frama++ = filt;

        max1 = high[i];
        min1 = low[i];
        max1_idx = progress;
        min1_idx = progress;
    }
    for (; i < size && progress < period; ++i, ++progress) {
        high_store.push_front(high[i]);
        low_store.push_front(low[i]);
        filt = (high[i] + low[i]) / 2.;
        *frama++ = filt;

        if (max1 <= high[i]) {
            max1 = high[i];
            max1_idx = progress;
        }
        if (min1 >= low[i]) {
            min1 = low[i];
            min1_idx = progress;
        }
    }
    for (; i < size; ++i, ++progress) {
        high_store.pop_back();
        low_store.pop_back();
        high_store.push_front(high[i]);
        low_store.push_front(low[i]);

        // the usual min/max maintaining machinery, done four times
        if (max2_idx == progress - (int)period) {
            max2 = high_store[(int)period - 1];
            max2_idx = progress - ((int)period - 1);
            for (int j = 1; j < period / 2; ++j) {
                if (high_store[(int)period - 1 - j] >= max2) {
                    max2 = high_store[(int)period - 1 - j];
                    max2_idx = progress - ((int)period - 1 - j);
                }
            }
        } else if (high_store[(int)(period / 2)] >= max2) {
            max2 = high_store[(int)(period / 2)];
            max2_idx = progress - ((int)(period / 2));
        }
        if (min2_idx == progress - (int)period) {
            min2 = low_store[(int)period - 1];
            min2_idx = progress - ((int)period - 1);
            for (int j = 1; j < period / 2; ++j) {
                if (low_store[(int)period - 1 - j] <= min2) {
                    min2 = low_store[(int)period - 1 - j];
                    min2_idx = progress - ((int)period - 1 - j);
                }
            }
        } else if (low_store[(int)(period / 2)] <= min2) {
            min2 = low_store[(int)(period / 2)];
            min2_idx = progress - ((int)(period / 2));
        }
        if (max1_idx == progress - (int)period / 2) {
            max1 = high_store[(int)period / 2 - 1];
            max1_idx = progress - ((int)period / 2 - 1);
            for (int j = 1; j < period / 2; ++j) {
                if (high_store[(int)period / 2 - 1 - j] >= max1) {
                    max1 = high_store[(int)period / 2 - 1 - j];
                    max1_idx = progress - ((int)period / 2 - 1 - j);
                }
            }
        } else if (high_store[0] >= max1) {
            max1 = high_store[0];
            max1_idx = progress - (0);
        }
        if (min1_idx == progress - (int)period / 2) {
            min1 = low_store[(int)period / 2 - 1];
            min1_idx = progress - ((int)period / 2 - 1);
            for (int j = 1; j < period / 2; ++j) {
                if (low_store[(int)period / 2 - 1 - j] <= min1) {
                    min1 = low_store[(int)period / 2 - 1 - j];
                    min1_idx = progress - ((int)period / 2 - 1 - j);
                }
            }
        } else if (low_store[0] <= min1) {
            min1 = low_store[0];
            min1_idx = progress - (0);
        }

        TI_REAL N3 = (MAX(max1, max2) - MIN(min1, min2)) / period;
        TI_REAL N1 = (max1 - min1) / (period / 2);
        TI_REAL N2 = (max2 - min2) / (period / 2);

        TI_REAL dimen = 0;
        if (N1 > 0 && N2 > 0 && N3 > 0) { dimen = (log(N1 + N2) - log(N3)) / log(2.); }

        TI_REAL alpha = exp(w * (dimen - 1));
        alpha = MAX(alpha, .01);
        alpha = MIN(alpha, 1.);
        filt = alpha * (high[i] + low[i]) / 2. + (1. - alpha) * filt;
        *frama++ = filt;
    }

    ptr->progress = progress;
    ptr->state.filt = filt;

    ptr->state.max1 = max1;
    ptr->state.max2 = max2;
    ptr->state.min1 = min1;
    ptr->state.min2 = min2;
    ptr->state.max1_idx = max1_idx;
    ptr->state.max2_idx = max2_idx;
    ptr->state.min1_idx = min1_idx;
    ptr->state.min2_idx = min2_idx;

    return TI_OKAY;
}
