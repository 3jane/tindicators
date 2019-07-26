#include "../indicators.h"
#include <new>
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

#include <algorithm>
#include <limits>

int ti_hurst_start(TI_REAL const *options) {
    const TI_REAL period = options[0];

    return period;
}

int ti_hurst(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) try {
    TI_REAL const *const real = inputs[0];
    const TI_REAL period = options[0];
    TI_REAL *hurst = outputs[0];
    TI_REAL *fractal_dim = outputs[1];

    if (period < 2) { return TI_INVALID_OPTION; }

    ringbuf<0> r(period);
    TI_REAL sum = 0;

    int i = 1;
    for (; i < period && i < size; ++i, step(r)) {
        r = real[i] && real[i-1] ? log(real[i] / real[i-1]) : 0;
        sum += r;
    }
    for (; i < size; ++i, step(r)) {
        r = real[i] && real[i-1] ? log(real[i] / real[i-1]) : 0;
        sum += r;

        TI_REAL mean = sum / period;

        TI_REAL cum_meandev = 0;
        TI_REAL max_cum_meandev = -std::numeric_limits<TI_REAL>::infinity();
        TI_REAL min_cum_meandev = std::numeric_limits<TI_REAL>::infinity();
        TI_REAL stddev = 0;
        for (int j = 0; j < period; ++j) {
            cum_meandev += r[j] - mean;
            max_cum_meandev = std::max(max_cum_meandev, cum_meandev);
            min_cum_meandev = std::min(min_cum_meandev, cum_meandev);
            stddev += pow(r[j] - mean, 2);
        }
        stddev /= period;
        stddev = sqrt(stddev);

        TI_REAL R = max_cum_meandev - min_cum_meandev;
        TI_REAL S = stddev;

        TI_REAL H = R ? log(R/S) / log(period) : 0;
        *hurst++ = H;
        *fractal_dim++ = 2 - H;

        sum -= r[period-1];
    }

    return TI_OKAY;
} catch (std::bad_alloc& e) {
    return TI_OUT_OF_MEMORY;
}

int DONTOPTIMIZE ti_hurst_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const real = inputs[0];
    const TI_REAL period = options[0];
    TI_REAL *hurst = outputs[0];
    TI_REAL *fractal_dim = outputs[1];

    if (period < 2) { return TI_INVALID_OPTION; }

    #define r(i) log(real[(i)]/real[(i)-1])

    for (int i = period; i < size; ++i) {
        TI_REAL mean = 0;
        for (int j = 0; j < period; ++j) {
            mean += r(i-j);
        }
        mean /= period;

        TI_REAL cum_meandev = 0;
        TI_REAL max_cum_meandev = -std::numeric_limits<TI_REAL>::infinity();
        TI_REAL min_cum_meandev = std::numeric_limits<TI_REAL>::infinity();
        for (int j = 0; j < period; ++j) {
            cum_meandev += r(i-j) - mean;
            max_cum_meandev = std::max(max_cum_meandev, cum_meandev);
            min_cum_meandev = std::min(min_cum_meandev, cum_meandev);
        }

        TI_REAL stddev = 0;
        for (int j = 0; j < period; ++j) {
            stddev += pow(r(i-j) - mean, 2);
        }
        stddev /= period;
        stddev = sqrt(stddev);

        TI_REAL R = max_cum_meandev - min_cum_meandev;
        TI_REAL S = stddev;

        TI_REAL H = log(R/S) / log(period);
        *hurst++ = H;
        *fractal_dim++ = 2 - H;
    }

    #undef r

    return TI_OKAY;
}

struct ti_hurst_stream : ti_stream {

    struct {
        TI_REAL period;
    } options;

    struct {
        ringbuf<0> r;
        ringbuf<2> price;
        TI_REAL sum = 0;
    } state;

    struct {

    } constants;
};

int ti_hurst_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];

    if (period < 2) { return TI_INVALID_OPTION; }

    ti_hurst_stream *ptr = new(std::nothrow) ti_hurst_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_HURST_INDEX;
    ptr->progress = -ti_hurst_start(options);

    ptr->options.period = period;

    try {
        ptr->state.r.resize(period);
    } catch (std::bad_alloc& e) {
        delete ptr;
        return TI_OUT_OF_MEMORY;
    }

    return TI_OKAY;
}

void ti_hurst_stream_free(ti_stream *stream) {
    delete static_cast<ti_hurst_stream*>(stream);
}

int ti_hurst_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_hurst_stream *ptr = static_cast<ti_hurst_stream*>(stream);
    TI_REAL const *const real = inputs[0];
    TI_REAL *hurst = outputs[0];
    TI_REAL *fractal_dim = outputs[1];
    int progress = ptr->progress;
    const TI_REAL period = ptr->options.period;

    auto &r = ptr->state.r;
    auto &price = ptr->state.price;
    TI_REAL sum = ptr->state.sum;

    int i = 0;
    for (; progress < -period+1 && i < size; ++i, ++progress, step(price)) {
        price = real[i];
    }
    for (; progress < 0 && i < size; ++i, ++progress, step(price, r)) {
        price = real[i];

        r = price && price[1] ? log(price / price[1]) : 0;
        sum += r;
    }
    for (; i < size; ++i, ++progress, step(price, r)) {
        price = real[i];

        r = price && price[1] ? log(price / price[1]) : 0;
        sum += r;

        TI_REAL mean = sum / period;

        TI_REAL cum_meandev = 0;
        TI_REAL max_cum_meandev = -std::numeric_limits<TI_REAL>::infinity();
        TI_REAL min_cum_meandev = std::numeric_limits<TI_REAL>::infinity();
        TI_REAL stddev = 0;
        for (int j = 0; j < period; ++j) {
            cum_meandev += r[j] - mean;
            max_cum_meandev = std::max(max_cum_meandev, cum_meandev);
            min_cum_meandev = std::min(min_cum_meandev, cum_meandev);
            stddev += pow(r[j] - mean, 2);
        }
        stddev /= period;
        stddev = sqrt(stddev);

        TI_REAL R = max_cum_meandev - min_cum_meandev;
        TI_REAL S = stddev;

        TI_REAL H = R ? log(R/S) / log(period) : 0;
        *hurst++ = H;
        *fractal_dim++ = 2 - H;

        sum -= r[period-1];
    }

    ptr->progress = progress;
    ptr->state.sum = sum;

    return TI_OKAY;
}