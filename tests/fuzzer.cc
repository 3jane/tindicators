#include "../indicators.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include "../utils/log.h"

double g_options[] = {-20, 0, .7, 5};
#define M (sizeof(g_options) / sizeof(g_options[0]))

#define N 210
TI_REAL g_input[TI_MAXINDPARAMS][N];
TI_REAL g_input_zeros[TI_MAXINDPARAMS][N];
TI_REAL g_input_dups[TI_MAXINDPARAMS][N];

void check_output(const ti_indicator_info& info, int outsize, TI_REAL** outputs) {
    if (!(info.type == TI_TYPE_INDICATOR || info.type  == TI_TYPE_OVERLAY)) { return; /* ignore non-indicators */ }
    for (int out_idx = 0; out_idx < info.outputs; ++out_idx) {
        for (int i = 0; i < outsize; ++i) {
            const TI_REAL op = outputs[out_idx][i];
            // if (!std::isfinite(op)) {
            //     printf("isfinite assertion failed at %s[%i]\n", info.output_names[out_idx], i);
            //     exit(1);
            // }
        }

    }
}

void stress(const ti_indicator_info& info) {
    std::vector<TI_REAL> options(info.options);

    auto test_vector = [&](int size, TI_REAL source[TI_MAXINDPARAMS][N], const char* msg = "") {
        printf("%i%s ", size, msg);
        fflush(stdout);

        TI_REAL** inputs = new TI_REAL*[info.inputs];
        for (int i = 0; i < info.inputs; ++i) {
            inputs[i] = new TI_REAL[size];
            for (int j = 0; j < size; ++j) {
                inputs[i][j] = source[i][j];
            }
        }
        TI_REAL** outputs = new TI_REAL*[info.outputs];
        int outsize = size - info.start(options.data());
        if (outsize > 0) {
            for (int i = 0; i < info.outputs; ++i) {
                outputs[i] = new TI_REAL[outsize];
            }
        }

        int result = info.indicator(size, inputs, options.data(), outputs);
        if (result == TI_OKAY) { check_output(info, outsize, outputs); }

        for (int i = 0; i < info.inputs; ++i) { delete[] inputs[i]; }
        delete[] inputs;
        if (outsize > 0) { for (int i = 0; i < info.outputs; ++i) { delete[] outputs[i]; } }
        delete[] outputs;
    };
    auto test_stream = [&](int size, TI_REAL source[TI_MAXINDPARAMS][N], const char* msg = "") {
        printf("s%i%s ", size, msg);
        fflush(stdout);

        TI_REAL** inputs = new TI_REAL*[info.inputs];
        for (int i = 0; i < info.inputs; ++i) {
            inputs[i] = new TI_REAL[size];
            for (int j = 0; j < size; ++j) {
                inputs[i][j] = source[i][j];
            }
        }
        TI_REAL** outputs = new TI_REAL*[info.outputs];
        int outsize = size - info.start(options.data());
        if (outsize > 0) {
            for (int i = 0; i < info.outputs; ++i) {
                outputs[i] = new TI_REAL[outsize];
            }
        }

        ti_stream *stream;
        int result = info.stream_new(options.data(), &stream);
        if (result == TI_OKAY) {
            info.stream_run(stream, size, inputs, outputs);
            check_output(info, outsize, outputs);
            info.stream_free(stream);
        }

        for (int i = 0; i < info.inputs; ++i) { delete[] inputs[i]; }
        delete[] inputs;
        if (outsize > 0) { for (int i = 0; i < info.outputs; ++i) { delete[] outputs[i]; } }
        delete[] outputs;
    };

    // please note that std::pow doesn't compute in exact even with integers
    for (unsigned option_bitset = 0; option_bitset < std::pow(M, info.options) + 1; ++option_bitset) {
        for (int i = 0; i < info.options; ++i) {
            options[i] = g_options[(option_bitset / (unsigned)std::pow(M, i)) % M];
        }

        printf("%s [", info.name);
        for (int i = 0; i < info.options-1; ++i) { printf("%s = % 5.1f, ", info.option_names[i], options[i]); }
        for (int i = std::max(0, info.options-1); i < info.options; ++i) { printf("%s = % 5.1f", info.option_names[i], options[i]); }
        printf("]: ");
        fflush(stdout);

        test_vector(0, g_input);
        test_vector(1, g_input);
        test_vector(2, g_input);
        test_vector(5, g_input);
        test_vector(N, g_input);
        test_vector(N, g_input_zeros, "x0");
        test_vector(N, g_input_dups, "x1");

        if (info.stream_new) {
            test_stream(0, g_input);
            test_stream(1, g_input);
            test_stream(2, g_input);
            test_stream(5, g_input);
            test_stream(N, g_input);
            test_stream(N, g_input_zeros, "x0");
            test_stream(N, g_input_dups, "x1");
        }

        printf("\r%140s\r", "");
    };
    printf("%s\n", info.name);
}


void flush_stdout(int) {
    printf("\nSegmentation fault\n");
    fflush(stdout);
    exit(1);
}

int main(int argc, char *argv[])
{
    signal(SIGSEGV, flush_stdout);

    for (int i = 0; i < TI_MAXINDPARAMS; ++i) {
        for (int j = 0; j < N; ++j) {
            g_input[i][j] = j;
            g_input_zeros[i][j] = 0;
            g_input_dups[i][j] = j / 10;
        }
    }

    if (argc > 1) {
        const ti_indicator_info *info = ti_find_indicator(argv[1]);
        if (info) {
            stress(*info);
        } else {
            printf("%s not found\n", argv[1]);
        }
    } else {
        for (int i = 0; i < TI_INDICATOR_COUNT; ++i) {
            stress(ti_indicators[i]);
        }
    }

    printf("\r                                                  \rDone\n");
}
