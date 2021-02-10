#include <new>

#include "../indicators.h"
#include "../utils/ringbuf.hh"

int ti_pbands_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    return period-1;
}

int ti_pbands(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    const int period = options[0];
    TI_REAL *pbands_lower = outputs[0];
    TI_REAL *pbands_upper = outputs[1];

    if (period < 1) { return TI_INVALID_OPTION; }

    TI_REAL y_sum = 0.;
    TI_REAL xy_sum = 0.;

    const TI_REAL x_sum = period * (period + 1) / 2.;
    const TI_REAL xsq_sum = period * (period + 1) * (2*period + 1) / 6.;

    TI_REAL max;
    TI_REAL min;
    int max_idx;
    int min_idx;

    int i = 0;
    for (; i < period-1 && i < size; ++i) {
        xy_sum += close[i] * (i + 1);
        y_sum += close[i];
    }
    for (; i < size; ++i) {
        xy_sum += close[i] * period;
        y_sum += close[i];

        // y = a + bx
        TI_REAL b = (xy_sum / period - x_sum / period * y_sum / period) / (xsq_sum / period - (x_sum / period)*(x_sum / period));

        TI_REAL the_max = high[i];
        for (int j = 1; j < period; ++j) {
            if (the_max < high[i-j] + j * b) {
                the_max = high[i-j] + j * b;
            }
        }
        TI_REAL the_min = low[i];
        for (int j = 1; j < period; ++j) {
            if (the_min > low[i-j] + j * b) {
                the_min = low[i-j] + j * b;
            }
        }
        *pbands_upper++ = the_max;
        *pbands_lower++ = the_min;

        xy_sum -= y_sum;
        y_sum -= close[i-period+1];
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int ti_pbands_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    const TI_REAL period = options[0];
    TI_REAL *pbands_lower = outputs[0];
    TI_REAL *pbands_upper = outputs[1];

    int start = ti_linregslope_start(options);
    TI_REAL *b = (TI_REAL*)malloc(sizeof(TI_REAL) * (size - start));

    ti_linregslope(size, &close, &period, &b);

    for (int i = start; i < size; ++i) {
        TI_REAL the_max = high[i];
        for (int j = 1; j < period; ++j) {
            if (the_max < high[i-j] + j * b[i-start]) {
                the_max = high[i-j] + j * b[i-start];
            }
        }
        TI_REAL the_min = low[i];
        for (int j = 1; j < period; ++j) {
            if (the_min > low[i-j] + j * b[i-start]) {
                the_min = low[i-j] + j * b[i-start];
            }
        }

        *pbands_upper++ = the_max;
        *pbands_lower++ = the_min;
    }

    free(b);

    return TI_OKAY;
}


struct ti_pbands_stream : ti_stream {
    struct {
        TI_REAL period;
    } options;

    struct {
        TI_REAL y_sum;
        TI_REAL xy_sum;

        ringbuf<0> price_high;
        ringbuf<0> price_low;
        ringbuf<0> price_close;
    } state;

    struct {
        TI_REAL x_sum;
        TI_REAL xsq_sum;
    } constants;
};

int ti_pbands_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];
    if (period < 1) { return TI_INVALID_OPTION; }

    ti_pbands_stream *ptr = new ti_pbands_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_PBANDS_INDEX;
    ptr->progress = -ti_pbands_start(options);

    ptr->options.period = period;

    ptr->state.y_sum = 0.;
    ptr->state.xy_sum = 0.;

    ptr->constants.x_sum = period * (period + 1) / 2.;
    ptr->constants.xsq_sum = period * (period + 1) * (2*period + 1) / 6.;

    try {
        ptr->state.price_high.resize(period+1);
        ptr->state.price_low.resize(period+1);
        ptr->state.price_close.resize(period+1);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_pbands_stream_free(ti_stream *stream) {
    delete static_cast<ti_pbands_stream*>(stream);
}

int ti_pbands_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_pbands_stream *ptr = static_cast<ti_pbands_stream*>(stream);

    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL const *close = inputs[2];
    TI_REAL *pbands_lower = outputs[0];
    TI_REAL *pbands_upper = outputs[1];

    int progress = ptr->progress;

    const TI_REAL period = ptr->options.period;

    TI_REAL y_sum = ptr->state.y_sum;
    TI_REAL xy_sum = ptr->state.xy_sum;

    const TI_REAL xsq_sum = ptr->constants.xsq_sum;
    const TI_REAL x_sum = ptr->constants.x_sum;

    auto &price_high = ptr->state.price_high;
    auto &price_low = ptr->state.price_low;
    auto &price_close = ptr->state.price_close;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(price_high, price_low, price_close)) {
        price_high = high[i];
        price_low = low[i];
        price_close = close[i];

        xy_sum += close[i] * (progress - (-period+1) + 1);
        y_sum += close[i];
    }

    for (; i < size; ++i, ++progress, step(price_high, price_low, price_close)) {
        price_high = high[i];
        price_low = low[i];
        price_close = close[i];

        xy_sum += close[i] * period;
        y_sum += close[i];

        // y = a + bx
        TI_REAL b = (xy_sum / period - x_sum / period * y_sum / period) / (xsq_sum / period - (x_sum / period)*(x_sum / period));

        TI_REAL the_max = high[i];
        for (int j = 1; j < period; ++j) {
            if (the_max < price_high[j] + j * b) {
                the_max = price_high[j] + j * b;
            }
        }
        TI_REAL the_min = low[i];
        for (int j = 1; j < period; ++j) {
            if (the_min > price_low[j] + j * b) {
                the_min = price_low[j] + j * b;
            }
        }
        *pbands_upper++ = the_max;
        *pbands_lower++ = the_min;

        xy_sum -= y_sum;
        y_sum -= price_close[period-1];
    }

    ptr->progress = progress;

    ptr->state.y_sum = y_sum;
    ptr->state.xy_sum = xy_sum;

    return TI_OKAY;
}
