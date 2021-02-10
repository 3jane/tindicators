#include <new>
#include <vector>
#include <queue>

#include "../indicators.h"
#include "../utils/log.h"
#include "../utils/minmax.h"
#include "../utils/ringbuf.hh"

int ti_vi_start(TI_REAL const *options) {
    const TI_REAL period = options[0];
    return int(period);
}

int ti_vi(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const high = inputs[0];
    TI_REAL const *const low = inputs[1];
    TI_REAL const *const close = inputs[2];
    const int period = options[0];
    TI_REAL *vi_p = outputs[0];
    TI_REAL *vi_m = outputs[1];

    if (period < 1 || period >= size) return TI_INVALID_OPTION;

    TI_REAL last_vm_p_period = 0;
    TI_REAL last_vm_m_period = 0;
    TI_REAL last_TR_period = 0;

    for (int k = 1; k <= int(period); k++) {
        last_vm_p_period += abs(high[k] - low[k - 1]);
        last_vm_m_period += abs(low[k] - high[k - 1]);
        last_TR_period += fmax(high[k] - low[k], fmax(abs(high[k] - close[k - 1]), abs(low[k] - close[k - 1])));
 
        vi_p[0] = last_vm_p_period ? last_vm_p_period / last_TR_period : 0;
        vi_m[0] = last_vm_m_period ? last_vm_m_period / last_TR_period : 0;
    }
    for (int i = period + 1; i < size; i++) {
        last_vm_p_period = last_vm_p_period - abs(high[i - period] - low[i - period - 1]) + abs(high[i] - low[i - 1]); 
        last_vm_m_period = last_vm_m_period - abs(low[i - period] - high[i - period - 1]) + abs(low[i] - high[i - 1]);
        last_TR_period = last_TR_period 
                         - fmax(high[i - period] - low[i - period], fmax(abs(high[i - period] - close[i - period - 1]), abs(low[i - period] - close[i - period - 1])))
                         + fmax(high[i] - low[i], fmax(abs(high[i] - close[i - 1]), abs(low[i] - close[i - 1])));
        vi_p[i - period] = last_vm_p_period ? last_vm_p_period / last_TR_period : 0;
        vi_m[i - period] = last_vm_m_period ? last_vm_m_period / last_TR_period : 0;
    }
    
    return TI_OKAY;
}

DONTOPTIMIZE int ti_vi_ref(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
    TI_REAL const *const high = inputs[0];
    TI_REAL const *const low = inputs[1];
    TI_REAL const *const close = inputs[2];
    const int period = options[0];
    TI_REAL *vi_p = outputs[0];
    TI_REAL *vi_m = outputs[1];

    if (period < 1 || period >= size) return TI_INVALID_OPTION;

    std::vector<TI_REAL> vm_p(size);
    std::vector<TI_REAL> vm_m(size);    
    std::vector<TI_REAL> TR(size);

    for (int i = 1; i < size; i++) {
        vm_p[i] = abs(high[i] - low[i - 1]);
        vm_m[i] = abs(low[i] - high[i - 1]);
        TR[i] = fmax(high[i] - low[i], fmax(abs(high[i] - close[i - 1]), abs(low[i] - close[i - 1])));
    }

    std::vector<TI_REAL> vm_p_period(size);
    std::vector<TI_REAL> vm_m_period(size);
    std::vector<TI_REAL> TR_period(size);

    vm_p_period[period] = 0;
    vm_m_period[period] = 0;
    TR_period[period]   = 0;

    for (int k = 1; k <= int(period); k++) {
        vm_p_period[period] += vm_p[k];
        vm_m_period[period] += vm_m[k];
        TR_period[period] += TR[k];
 
        vi_p[0] = vm_p_period[period] / TR_period[period];
        vi_m[0] = vm_m_period[period] / TR_period[period];
    }
    for (int i = period + 1; i < size; i++) {
        vm_p_period[i] = vm_p_period[i - 1] - vm_p[i - period] + vm_p[i];
        vm_m_period[i] = vm_m_period[i - 1] - vm_m[i - period] + vm_m[i];
        TR_period[i] = TR_period[i - 1] - TR[i - period] + TR[i];

        vi_p[i - period] = vm_p_period[i] / TR_period[i];
        vi_m[i - period] = vm_m_period[i] / TR_period[i];
    }
    
    return TI_OKAY;
}

struct ti_vi_stream : ti_stream {

    struct {
        TI_REAL period;
    } options;

    struct {
        TI_REAL last_vm_p_period = 0;
        TI_REAL last_vm_m_period = 0;
        TI_REAL last_TR_period = 0;
    } state;

    std::queue<TI_REAL> high_story;
    std::queue<TI_REAL> low_story;
    std::queue<TI_REAL> close_story;
    
    struct {

    } constants;
};

int ti_vi_stream_new(TI_REAL const *options, ti_stream **stream) {
    const TI_REAL period = options[0];

    if (period < 1) return TI_INVALID_OPTION;

    ti_vi_stream *ptr = new(std::nothrow) ti_vi_stream();
    if (!ptr) { return TI_OUT_OF_MEMORY; }
    *stream = ptr;

    ptr->index = TI_INDICATOR_VI_INDEX;
    ptr->progress = -ti_vi_start(options);

    ptr->options.period = period;

    return TI_OKAY;
}

void ti_vi_stream_free(ti_stream *stream) {
    delete static_cast<ti_vi_stream*>(stream);
}

int ti_vi_stream_run(ti_stream *stream, int size, TI_REAL const *const *inputs, TI_REAL *const *outputs) {
    ti_vi_stream *ptr = static_cast<ti_vi_stream*>(stream);
    TI_REAL const *const high = inputs[0];
    TI_REAL const *const low = inputs[1];
    TI_REAL const *const close = inputs[2];
    TI_REAL *vi_p = outputs[0];
    TI_REAL *vi_m = outputs[1];
    int progress = ptr->progress;
    const TI_REAL period = ptr->options.period;

    TI_REAL last_vm_p_period = ptr->state.last_vm_p_period;
    TI_REAL last_vm_m_period = ptr->state.last_vm_m_period;
    TI_REAL last_TR_period = ptr->state.last_TR_period;

    std::queue<TI_REAL> &high_story = ptr->high_story;
    std::queue<TI_REAL> &low_story = ptr->low_story;
    std::queue<TI_REAL> &close_story = ptr->close_story;

    int index = 0;

    if (high_story.size() <= period) {
        while(high_story.size() <= period && index < size) {            
            if(high_story.size() > 0) {
                last_vm_p_period += abs(high[index] - low_story.back());
                last_vm_m_period += abs(low[index] - high_story.back());
                last_TR_period += fmax(high[index] - low[index], fmax(abs(high[index] - close_story.back()), abs(low[index] - close_story.back()))); 
            }
            
            high_story.push(high[index]);
            low_story.push(low[index]);
            close_story.push(close[index]);
            
            progress++;
            index++;
        }
        if (high_story.size() > period) {
            *vi_p++ = last_vm_p_period ? last_vm_p_period / last_TR_period : 0;
            *vi_m++ = last_vm_m_period ? last_vm_m_period / last_TR_period : 0;
        }
    }
    
    for (; index < size; index++) {
        TI_REAL high_front = high_story.front();
        TI_REAL low_front = low_story.front();
        TI_REAL close_front = close_story.front();

        high_story.pop();
        low_story.pop();
        close_story.pop();
  
        last_vm_p_period = last_vm_p_period 
                           - abs(high_story.front() - low_front)
                           + abs(high[index] - low_story.back());
        last_vm_m_period = last_vm_m_period
                           - abs(low_story.front() - high_front) 
                           + abs(low[index] - high_story.back());
        last_TR_period = last_TR_period
                         - fmax(high_story.front() - low_story.front(), fmax(abs(high_story.front() - close_front), abs(low_story.front() - close_front)))
                         + fmax(high[index] - low[index], fmax(abs(high[index] - close_story.back()), abs(low[index] - close_story.back())));

        high_story.push(high[index]);
        low_story.push(low[index]);
        close_story.push(close[index]);
 
        *vi_p++ = last_vm_p_period ? last_vm_p_period / last_TR_period : 0;
        *vi_m++ = last_vm_m_period ? last_vm_m_period / last_TR_period : 0;
        progress++;
    }     

    ptr->progress = progress;
    ptr->state.last_vm_p_period = last_vm_p_period;
    ptr->state.last_vm_m_period = last_vm_m_period;
    ptr->state.last_TR_period = last_TR_period;

    return TI_OKAY;
}

