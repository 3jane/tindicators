#include <new>
#include <algorithm>
#include <limits>

#include "../indicators.h"
#include "../utils/ringbuf.hh"
#include "../utils/log.h"

int ti_pc_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    return period-1;
}

int ti_pc (int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    const int period = options[0];
    TI_REAL *pc_low = outputs[0];
    TI_REAL *pc_high = outputs[1];

    if (period < 1) { return TI_INVALID_OPTION; }

    TI_REAL max = -std::numeric_limits<TI_REAL>::infinity();
    TI_REAL min = std::numeric_limits<TI_REAL>::infinity();
    int max_idx;
    int min_idx;


    int i = 0;
    for (; i < period - 1 && i < size; ++i) {
        if (max <= high[i]) {
            max = high[i];
            max_idx = i;
        }
        if (min >= low[i]) {
            min = low[i];
            min_idx = i;
        }
    }
    for (; i < size; ++i) {
        if (max_idx == i - period) {
            auto it = std::max_element(std::make_reverse_iterator(high + i + 1), std::make_reverse_iterator(high + i - period + 1)).base() - 1;
            max = *it;
            max_idx = it - high;
        } else if (max <= high[i]) {
            max = high[i];
            max_idx = i;
        }
        if (min_idx == i - period) {
            auto it = std::min_element(std::make_reverse_iterator(low + i + 1), std::make_reverse_iterator(low + i - period + 1)).base() - 1;
            min = *it;
            min_idx = it - low;
        } else if (min >= low[i]) {
            min = low[i];
            min_idx = i;
        }

        *pc_low++ = min;
        *pc_high++ = max;
    }

    return TI_OKAY;
}

int ti_pc_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    const TI_REAL period = options[0];
    TI_REAL *pc_low = outputs[0];
    TI_REAL *pc_high = outputs[1];

    if (period < 1) { return TI_INVALID_OPTION; }
    if (size <= ti_pc_start(options)) { return TI_OKAY; }

    ti_min(size, &low, &period, &pc_low);
    ti_max(size, &high, &period, &pc_high);

    return TI_OKAY;
}

struct ti_pc_stream : ti_stream {
    struct {
        int period;
    } options;

    struct {
        TI_REAL max = -std::numeric_limits<TI_REAL>::infinity();
        TI_REAL min = std::numeric_limits<TI_REAL>::infinity();
        int max_idx = 0;
        int min_idx = 0;

        ringbuf<0> price_low;
        ringbuf<0> price_high;
    } state;

    struct {

    } constants;
};

int ti_pc_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];

    if (period < 1) { return TI_INVALID_OPTION; }

    ti_pc_stream *ptr = new(std::nothrow) ti_pc_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_PC_INDEX;
    ptr->progress = -ti_pc_start(options);
    ptr->options.period = period;

    try {
        ptr->state.price_high.resize(period);
        ptr->state.price_low.resize(period);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_pc_stream_free(ti_stream *stream) {
    delete static_cast<ti_pc_stream*>(stream);
}

int ti_pc_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_pc_stream *ptr = static_cast<ti_pc_stream*>(stream);
    int progress = ptr->progress;

    TI_REAL const *high = inputs[0];
    TI_REAL const *low = inputs[1];
    TI_REAL *pc_low = outputs[0];
    TI_REAL *pc_high = outputs[1];

    const int period = ptr->options.period;

    TI_REAL max = ptr->state.max;
    TI_REAL min = ptr->state.min;
    int max_idx = ptr->state.max_idx;
    int min_idx = ptr->state.min_idx;

    auto &price_high = ptr->state.price_high;
    auto &price_low = ptr->state.price_low;

    int i = 0;
    for (; progress < 0 && i < size; ++i, ++progress, step(price_high, price_low)) {
        price_low = low[i];
        price_high = high[i];

        if (max <= high[i]) {
            max = high[i];
            max_idx = progress;
        }
        if (min >= low[i]) {
            min = low[i];
            min_idx = progress;
        }
    }
    for (; i < size; ++i, ++progress, step(price_high, price_low)) {
        price_low = low[i];
        price_high = high[i];

        if (max_idx == progress - period) {
            auto it = price_high.find_max(period);
            max = *it;
            max_idx = progress - price_high.iterator_to_age(it);
        } else if (max <= high[i]) {
            max = high[i];
            max_idx = progress;
        }
        if (min_idx == progress - period) {
            auto it = price_low.find_min(period);
            min = *it;
            min_idx = progress - price_low.iterator_to_age(it);
        } else if (min >= low[i]) {
            min = low[i];
            min_idx = progress;
        }

        *pc_low++ = min;
        *pc_high++ = max;
    }

    ptr->progress = progress;
    ptr->state.max = max;
    ptr->state.max_idx = max_idx;
    ptr->state.min = min;
    ptr->state.min_idx = min_idx;

    return TI_OKAY;
}
