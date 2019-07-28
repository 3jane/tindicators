/*
 * Tulip Indicators
 * https://tulipindicators.org/
 * Copyright (c) 2018 Tulip Charts LLC
 * Ilya Pikulin (ilya.pikulin@gmail.com)
 *
 * This file is part of Tulip Indicators.
 *
 * Tulip Indicators is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Tulip Indicators is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Tulip Indicators.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

#include "indicators.h"
#include "utils/testing.h"
#include "utils/minmax.h"

#define INSIZE 400
#define MIN_PERIOD 4
#define MAX_PERIOD 150
#define LOOPS 10

TI_REAL g_inputs[TI_MAXINDPARAMS][INSIZE];

TI_REAL *const open = g_inputs[0];
TI_REAL *const high = g_inputs[1];
TI_REAL *const low = g_inputs[2];
TI_REAL *const close = g_inputs[3];
TI_REAL *const volume = g_inputs[4];

void generate_inputs() {
    open[0] = 100;

    srand(22);

    int i;
    for (i = 0; i < INSIZE; ++i) {
        const TI_REAL diff1 = (((TI_REAL)(rand()) / RAND_MAX - .5) + .01) * 2.5;
        const TI_REAL diff2 = (((TI_REAL)(rand()) / RAND_MAX - .5) + .01) * 2.5;
        const TI_REAL diff3 = (((TI_REAL)(rand()) / RAND_MAX)) * 0.5;
        const TI_REAL diff4 = (((TI_REAL)(rand()) / RAND_MAX)) * 0.5;
        const TI_REAL vol = (((TI_REAL)(rand()) / RAND_MAX)) * 10000 + 500;

        if (i) {
            open[i] = open[i-1] + diff1;
        }

        close[i] = open[i] + diff2;
        high[i] = open[i] > close[i] ? open[i] + diff3 : close[i] + diff3;
        low[i] = open[i] < close[i] ? open[i] - diff4 : close[i] - diff4;
        volume[i] = vol;
        if (rand() % 100 == 0) {
            close[i] = low[i] = high[i] = open[i];
        }

        assert(open[i] <= high[i]);
        assert(close[i] <= high[i]);

        assert(open[i] >= low[i]);
        assert(close[i] >= low[i]);

        assert(high[i] >= low[i]);
    }
}

/************ SPECIAL OPTION SETTERS *************/

void simple_option_setter(double period, double *options) {
    options[0] = period;
}

void macd_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = period + 10;

    options[2] = period + 1;
}

void ppo_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = period + 10;
}

void psar_option_setter(double period, double *options) {
    options[0] = 1/period;
    options[1] = options[0] * 10;
}

void fast_slow_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = period + 10;
}

void stoch_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = 3;
    options[2] = 4;
}

void stochrsi_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = period;
    options[2] = period;
    options[3] = 1;
}

void ultosc_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = period * 2;
    options[2] = period * 4;
}

void vidya_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = period + 10;
    options[2] = .2;
}

void ce_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = 3;
}

void rmta_option_setter(double period,  double *options) {
    options[0] = period;
    options[1] = 1 - (2. / (period + 1));
}

void kst_option_setter(double period, double *options) {
    for (int i = 0; i < 4; ++i) {
        options[i+4] = options[i] = period + period / 4. * i;
    }
}

void pfe_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = 5;
}

void mama_option_setter(double period, double *options) {
    options[0] = 0.5;
    options[1] = 0.05;
}

void kc_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = 0.77;
}

void copp_option_setter(double period, double *options) {
    options[0] = 11;
    options[1] = 14;
    options[2] = period;
}


void posc_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = 3;
}

void rmi_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = 3;
}

void rvi_option_setter(double period, double *options) {
    options[0] = period;
    options[1] = 10;
}

void frama_option_setter(double period, double* options) {
	options[0] = (int)period / 2 * 2.;
	options[1] = 198;
}

void smi_option_setter(double period, double* options) {
    options[0] = period;
    options[1] = 25;
    options[2] = 3;
}

void tsi_option_setter(double period, double* options) {
    options[0] = period;
    options[1] = 3;
}

void hfema_option_setter(double period, double* options) {
    options[0] = period;
    options[1] = (int)period/2;
    options[2] = 1;
}

void rema_option_setter(double period, double* options) {
    options[0] = period;
    options[1] = 0.5;
}

void lf_option_setter(double period, double* options) {
    options[0] = 0.8;
}

void hfsma_option_setter(double period, double* options) {
    options[0] = period;
    options[1] = (int)period/2;
    options[2] = 1;
}

void edcf_option_setter(double period, double* options) {
    options[0] = (1 + (int)period % 5); // known to be slow for its algorithmic complexity
}

void mesastoch_option_setter(double period, double* options) {
    options[0] = period;
    options[1] = 48;
}

void bf23_option_setter(double period, double* options) {
    options[0] = ((int)period / 4 / 2 + 1) * 2;
}

void gf1_option_setter(double period, double* options) {
    options[0] = MIN(period*2., 40.);
}

void gf2_option_setter(double period, double* options) {
    options[0] = MIN(period*2, 40);
}

void gf3_option_setter(double period, double* options) {
    options[0] = MIN(period*2, 40);
}

void gf4_option_setter(double period, double* options) {
    options[0] = MIN(period*2, 40);
}

void ehma_option_setter(double period, double* options) {
    while ((int)pow((int)sqrt(period), 2) != (int)period || (int)period % 2 != 0) {
        period += 1;
    }
    options[0] = period;
}

void evwma_option_setter(double period, double* options) {
    options[0] = period;
    options[1] = 1;
}

void pwma_option_setter(double period, double* options) {
    options[0] = fmin(period, 10);
    options[1] = 2;
}

void swma_option_setter(double period, double* options) {
    period = MIN(period, 25);
    if ((int)period % 6 == 0 || ((int)period + 1) % 6 == 0) { period += 2; }
    options[0] = period;
}

void hurst_option_setter(double period, double* options) {
    options[0] = MIN(period, 10);
}

void arsi_option_setter(double period, double* options) {
    options[0] = MIN(period, 10);
}

void mhlma_option_setter(double period, double* options) {
    options[0] = period;
    options[1] = 10 + period / 4;
}

void bench(const ti_indicator_info *info) {
    printf("Running: %s ", info->name);
    fflush(stdout);

    void (*options_setter)(double period, double *options) = simple_option_setter;
    if (strcmp(info->name, "apo") == 0) { options_setter = ppo_option_setter; }
    if (strcmp(info->name, "ce") == 0) { options_setter = ce_option_setter; }
    if (strcmp(info->name, "frama") == 0) { options_setter = frama_option_setter; }
    if (strcmp(info->name, "macd") == 0) { options_setter = macd_option_setter; }
    if (strcmp(info->name, "ppo") == 0) { options_setter = ppo_option_setter; }
    if (strcmp(info->name, "psar") == 0) { options_setter = psar_option_setter; }
    if (strcmp(info->name, "adosc") == 0) { options_setter = fast_slow_option_setter; }
    if (strcmp(info->name, "kst") == 0) { options_setter = kst_option_setter; }
    if (strcmp(info->name, "kvo") == 0) { options_setter = fast_slow_option_setter; }
    if (strcmp(info->name, "rmta") == 0) { options_setter = rmta_option_setter; }
    if (strcmp(info->name, "rmi") == 0) { options_setter = rmi_option_setter; }
    if (strcmp(info->name, "rvi") == 0) { options_setter = rvi_option_setter; }
    if (strcmp(info->name, "tsi") == 0) { options_setter = tsi_option_setter; }
    if (strcmp(info->name, "smi") == 0) { options_setter = smi_option_setter; }
    if (strcmp(info->name, "stoch") == 0) { options_setter = stoch_option_setter; }
    if (strcmp(info->name, "stochrsi") == 0) { options_setter = stochrsi_option_setter; }
    if (strcmp(info->name, "ultosc") == 0) { options_setter = ultosc_option_setter; }
    if (strcmp(info->name, "vosc") == 0) { options_setter = fast_slow_option_setter; }
    if (strcmp(info->name, "vidya") == 0) { options_setter = vidya_option_setter; }
    if (strcmp(info->name, "pfe") == 0) { options_setter = pfe_option_setter; }
    if (strcmp(info->name, "mama") == 0) { options_setter = mama_option_setter; }
    if (strcmp(info->name, "kc") == 0) { options_setter = kc_option_setter; }
    if (strcmp(info->name, "copp") == 0) { options_setter = copp_option_setter; }
    if (strcmp(info->name, "posc") == 0) { options_setter = posc_option_setter; }
    if (strcmp(info->name, "hfsma") == 0) { options_setter = hfsma_option_setter; }
    if (strcmp(info->name, "hfema") == 0) { options_setter = hfema_option_setter; }
    if (strcmp(info->name, "rema") == 0) { options_setter = rema_option_setter; }
    if (strcmp(info->name, "lf") == 0) { options_setter = lf_option_setter; }
    if (strcmp(info->name, "edcf") == 0) { options_setter = edcf_option_setter; }
    if (strcmp(info->name, "mesastoch") == 0) { options_setter = mesastoch_option_setter; }
    if (strcmp(info->name, "bf2") == 0) { options_setter = bf23_option_setter; }
    if (strcmp(info->name, "bf3") == 0) { options_setter = bf23_option_setter; }
    if (strcmp(info->name, "gf1") == 0) { options_setter = gf1_option_setter; }
    if (strcmp(info->name, "gf2") == 0) { options_setter = gf2_option_setter; }
    if (strcmp(info->name, "gf3") == 0) { options_setter = gf3_option_setter; }
    if (strcmp(info->name, "gf4") == 0) { options_setter = gf4_option_setter; }
    if (strcmp(info->name, "ehma") == 0) { options_setter = ehma_option_setter; }
    if (strcmp(info->name, "evwma") == 0) { options_setter = evwma_option_setter; }
    if (strcmp(info->name, "pwma") == 0) { options_setter = pwma_option_setter; }
    if (strcmp(info->name, "swma") == 0) { options_setter = swma_option_setter; }
    if (strcmp(info->name, "hurst") == 0) { options_setter = hurst_option_setter; }
    if (strcmp(info->name, "arsi") == 0) { options_setter = arsi_option_setter; }
    if (strcmp(info->name, "mhlma") == 0) { options_setter = mhlma_option_setter; }

    static TI_REAL *inputs[TI_MAXINDPARAMS];
    for (int i = 0; i < info->inputs; ++i) {
        if (strcmp(info->input_names[i], "open") == 0) { inputs[i] = open; }
        else if (strcmp(info->input_names[i], "high") == 0) { inputs[i] = high; }
        else if (strcmp(info->input_names[i], "low") == 0) { inputs[i] = low; }
        else if (strcmp(info->input_names[i], "close") == 0) { inputs[i] = close; }
        else if (strcmp(info->input_names[i], "real") == 0) { inputs[i] = close; }
        else if (strcmp(info->input_names[i], "volume") == 0) { inputs[i] = volume; }
        else { assert(0 && "unsupported input"); };
    }

    int elapsed_plain = 0;
    int elapsed_ref = 0;
    int elapsed_stream_1 = 0;
    int elapsed_stream_all = 0;


    for (int i = 0; i < LOOPS; ++i) {
        for (int period = MIN_PERIOD; period <= MAX_PERIOD; ++period) {
            static TI_REAL outputs_mem[4][TI_MAXINDPARAMS][INSIZE];
            static TI_REAL options[TI_MAXINDPARAMS];

            static TI_REAL *outputs[TI_MAXINDPARAMS];
            static TI_REAL *outputs_ref[TI_MAXINDPARAMS];
            static TI_REAL *outputs_stream_all[TI_MAXINDPARAMS];
            static TI_REAL *outputs_stream_1[TI_MAXINDPARAMS];
            for (int j = 0; j < info->outputs; ++j) {
                outputs[j] = outputs_mem[0][j];
                outputs_ref[j] = outputs_mem[1][j];
                outputs_stream_all[j] = outputs_mem[2][j];
                outputs_stream_1[j] = outputs_mem[3][j];
            }
            options_setter(period, options);

            const int OUTSIZE = INSIZE - info->start(options);

            clock_t start_ts, end_ts;

            {
                start_ts = clock();
                const int ret = info->indicator(INSIZE, (const TI_REAL * const*)inputs, options, outputs);
                end_ts = clock();
                elapsed_plain += end_ts - start_ts;
                if (ret != TI_OKAY) {
                    printf("%s returned %i, exiting\n", info->name, ret);
                    exit(2);
                }
            }

            if (info->indicator_ref) {
                start_ts = clock();
                const int ret = info->indicator_ref(INSIZE, (const TI_REAL * const*)inputs, options, outputs_ref);
                end_ts = clock();
                elapsed_ref += end_ts - start_ts;
                if (ret != TI_OKAY) {
                    printf("%s_ref returned %i, exiting\n", info->name, ret);
                    exit(2);
                }
                int ok = !compare_answers(info, outputs, outputs_ref, OUTSIZE, OUTSIZE);
                if (!ok) {
                    printf("%s_ref mismatched, exiting\n", info->name);
                    printf("options were: ");
                    for (int k = 0; k < info->options; ++k) {
                        printf("%s=%f ", info->option_names[k], options[k]);
                    }
                    printf("\n");
                    exit(1);
                }
            }

            if (info->stream_new) {
                ti_stream *stream;
                const int ret = info->stream_new(options, &stream);
                start_ts = clock();
                ti_stream_run(stream, INSIZE, (const TI_REAL * const*)inputs, outputs_stream_all);
                end_ts = clock();
                ti_stream_free(stream);
                elapsed_stream_all += end_ts - start_ts;
                if (ret != TI_OKAY) {
                    printf("%s_stream_new returned %i, exiting\n", info->name, ret);
                    exit(2);
                }
                int ok = !compare_answers(info, outputs, outputs_stream_all, OUTSIZE, OUTSIZE);
                if (!ok) {
                    printf("%s_stream_all mismatched, exiting\n", info->name);
                    printf("options were: ");
                    for (int k = 0; k < info->options; ++k) {
                        printf("%s=%f ", info->option_names[k], options[k]);
                    }
                    printf("\n");
                    exit(1);
                }
            }

            if (info->stream_new) {
                ti_stream *stream;
                int ret = info->stream_new(options, &stream);
                TI_REAL *inputs_[TI_MAXINDPARAMS] = {0};
                TI_REAL *outputs_[TI_MAXINDPARAMS] = {0};
                for (int j = 0; j < info->inputs; ++j) {
                    inputs_[j] = malloc(sizeof(TI_REAL));
                }
                start_ts = clock();
                for (int bar = 0; bar < INSIZE; ++bar) {
                    for (int j = 0; j < info->inputs; ++j) {
                        *inputs_[j] = inputs[j][bar];
                    }
                    for (int j = 0; j < info->outputs; ++j) {
                        outputs_[j] = outputs_stream_1[j] + ti_stream_get_progress(stream);
                    }
                    ret = ti_stream_run(stream, 1, (const TI_REAL * const*)inputs_, outputs_);
                    if (ret != TI_OKAY) {
                        printf("%s_stream_new returned %i, exiting\n", info->name, ret);
                        exit(2);
                    }
                }
                end_ts = clock();
                for (int j = 0; j < info->inputs; ++j) { free(inputs_[j]); }
                elapsed_stream_1 += end_ts - start_ts;
                int ok = !compare_answers(info, outputs, outputs_stream_1, OUTSIZE, OUTSIZE);
                if (!ok) {
                    printf("%s_stream_1 mismatched, exiting\n", info->name);
                    printf("options were: ");
                    for (int k = 0; k < info->options; ++k) {
                        printf("%s=%f ", info->option_names[k], options[k]);
                    }
                    printf("\n");
                    exit(1);
                }
                ti_stream_free(stream);
            }
        }
    }

    #define MS(elapsed) (int)(elapsed * 1000. / CLOCKS_PER_SEC)
    const int iterations = LOOPS * (MAX_PERIOD - MIN_PERIOD + 1);
    #define PERFORMANCE(elapsed) (int)(MS(elapsed) ? (iterations * INSIZE) / MS(elapsed) / 1000. : 0)

    // mfps = million floats per second
    printf("\r");
    printf("Benchmark %15s%s\t%5dms\t%5dmfps\n", info->name, "           ", MS(elapsed_plain), PERFORMANCE(elapsed_plain));
    if (info->indicator_ref) { printf("Benchmark %15s%s\t%5dms\t%5dmfps\n", info->name, "_ref       ", MS(elapsed_ref), PERFORMANCE(elapsed_ref)); }
    if (info->stream_new) { printf("Benchmark %15s%s\t%5dms\t%5dmfps\n", info->name,    "_stream_all", MS(elapsed_stream_all), PERFORMANCE(elapsed_stream_all)); }
    if (info->stream_new) { printf("Benchmark %15s%s\t%5dms\t%5dmfps\n", info->name,    "_stream_1  ", MS(elapsed_stream_1), PERFORMANCE(elapsed_stream_1)); }

	fflush(stdout);
}

int main(int argc, char** argv) {
    generate_inputs();

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            const ti_indicator_info *info = ti_find_indicator(argv[i]);
            if (!info) {
                printf("indicator %s not found\n", argv[i]);
                exit(3);
            }
            bench(info);
        }
    } else {
        ti_indicator_info *info = ti_indicators;
        for (int i = 0; i < TI_INDICATOR_COUNT; ++i) {
            bench(info + i);
        }
    }
}
