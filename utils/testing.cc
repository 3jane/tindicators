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

#include "testing.h"
#include "log.h"

#include <algorithm>
#include <vector>

/*********** FUNCTIONS **********/

int equal_reals(TI_REAL a, TI_REAL b) {
    return fabs(a - b) < 0.001 || fabs(1 - fabs(a / b)) < 0.001;
}

int equal_arrays(const TI_REAL *a, const TI_REAL *b, int size_a, int size_b) {
    if (size_a != size_b) { return 0; }

    int i;
    for (i = 0; i < size_a; ++i) {
        if (!equal_reals(a[i], b[i])) { return 0; };
    }
    return 1;
}

void print_array(const TI_REAL *a, int size) {
    printf("[%i] = {", size);
    int i;
    for (i = 0; i < size-1; ++i) {
        printf("%.3f,", a[i]);
    }
    if (size) { printf("%.3f", a[size-1]); }
    printf("}");
}

int compare_answers(const ti_indicator_info *info, TI_REAL *answers[], TI_REAL *outputs[], int answer_size, int output_size) {
    int failures = 0;
    for (int i = 0; i < info->outputs; ++i) {
        // if (!equal_arrays(answers[i], outputs[i], answer_size, output_size)) {
        //     ++failures;
        //     printf("output '%s' mismatch\n", info->output_names[i]);
        //     printf("> expected: "); print_array(answers[i], answer_size); printf("\n");
        //     printf("> got:      "); print_array(outputs[i], output_size); printf("\n");
        // }
        const int maxsz = std::max(answer_size, output_size);
        std::vector<int> good(maxsz);
        int any_bad = 0;
        for (int j = 0; j < maxsz; ++j) {
            good[j] = (j < answer_size && j < output_size && equal_reals(answers[i][j], outputs[i][j]));
            any_bad += !good[j];
        }
        if (any_bad) {
            printf("output '%s' mismatch\n", info->output_names[i]);
            printf("> expected: [%i] = {", answer_size);
            for (int j = 0; j < answer_size; ++j) {
                if (good[j]) { printf("\033[22;37m%.3f\033[0m%s", answers[i][j], (j < answer_size-1 ? "," : "")); }
                else { printf("\033[1;31m%.3f\033[0m%s", answers[i][j], (j < answer_size-1 ? "," : "")); }
            }
            printf("}\n");
            printf("> got:      [%i] = {", output_size);
            for (int j = 0; j < output_size; ++j) {
                if (good[j]) { printf("\033[22;37m%.3f\033[0m%s", outputs[i][j], (j < output_size-1 ? "," : "")); }
                else { printf("\033[1;31m%.3f\033[0m%s", outputs[i][j], (j < output_size-1 ? "," : "")); }
            }
            printf("}\n");
        }
        failures += bool(any_bad);
    }
    return failures;
}