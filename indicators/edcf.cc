#include <vector>
#include <deque>
#include <new>

#include "../indicators.h"
#include "../utils/localbuffer.h"
#include "../utils/log.h"


int ti_edcf_start(TI_REAL const *options) {
    TI_REAL length = options[0];

    return 2*length-1;
}

int ti_edcf(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const series = inputs[0];
    TI_REAL length = options[0];
    TI_REAL *edcf = outputs[0];

    if (length < 1) { return TI_INVALID_OPTION; }

    std::deque<TI_REAL> price;
    std::vector<TI_REAL> coef;
    std::vector<TI_REAL> distance2;

    coef.resize(2*length);
    distance2.resize(2*length);

    int progress = -ti_edcf_start(options);
    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress) {
        price.push_front(series[i]);
    }
    for (; i < size; ++i, ++progress) {
        price.push_front(series[i]);
        for (int count = 0; count < length; ++count) {
            distance2[count] = 0;
            for (int lookback = 1; lookback < length; ++lookback) {
                distance2[count] += (price[count] - price[count+lookback]) * (price[count] - price[count+lookback]);
            }
            coef[count] = distance2[count];
        }

        TI_REAL num = 0.;
        TI_REAL sumcoef = 0.;
        for (int count = 0; count < length; ++count) {
            num += coef[count] * price[count];
            sumcoef += coef[count];
        }
        *edcf++ = (sumcoef != 0. ? num / sumcoef : 0.);

        price.pop_back();
    }

    return TI_OKAY;
}

struct ti_edcf_stream : ti_stream {

    struct {
        TI_REAL length;
    } options;

    struct {
        std::vector<TI_REAL> coef;
        std::vector<TI_REAL> distance2;
        std::deque<TI_REAL> price;
    } state;

    struct {

    } constants;
};

int ti_edcf_stream_new(TI_REAL const *options, ti_stream **stream) {
    TI_REAL lookback = options[0];
    TI_REAL length = options[0];

    if (length < 1) { return TI_INVALID_OPTION; }

    ti_edcf_stream *ptr = new(std::nothrow) ti_edcf_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_EDCF_INDEX;
    ptr->progress = -ti_edcf_start(options);

    ptr->options.length = length;

    ptr->state.coef.resize(2*length);
    ptr->state.distance2.resize(2*length);

    return TI_OKAY;
}

void ti_edcf_stream_free(ti_stream *stream) {
    delete static_cast<ti_edcf_stream*>(stream);
}

int ti_edcf_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_edcf_stream *ptr = static_cast<ti_edcf_stream*>(stream);
    TI_REAL const *const series = inputs[0];
    TI_REAL *edcf = outputs[0];
    int progress = ptr->progress;
    TI_REAL length = ptr->options.length;

    auto &price = ptr->state.price;
    auto &coef = ptr->state.coef;
    auto &distance2 = ptr->state.distance2;

    int i = 0;
    for (; i < size && progress < 0; ++i, ++progress) {
        price.push_front(series[i]);
    }
    for (; i < size; ++i, ++progress) {
        price.push_front(series[i]);
        for (int count = 0; count < length; ++count) {
            distance2[count] = 0;
            for (int lookback = 1; lookback < length; ++lookback) {
                distance2[count] += (price[count] - price[count+lookback]) * (price[count] - price[count+lookback]);
            }
            coef[count] = distance2[count];
        }

        TI_REAL num = 0.;
        TI_REAL sumcoef = 0.;
        for (int count = 0; count < length; ++count) {
            num += coef[count] * price[count];
            sumcoef += coef[count];
        }
        *edcf++ = (sumcoef != 0. ? num / sumcoef : 0.);

        price.pop_back();
    }

    ptr->progress = progress;

    return TI_OKAY;
}
