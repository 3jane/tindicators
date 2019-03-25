/*
 * Tulip Indicators
 * https://tulipindicators.org/
 * Copyright (c) 2010-2017 Tulip Charts LLC
 * Lewis Van Winkle (LV@tulipcharts.org)
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

#include "../indicators.h"
#include "../utils/minmax.h"



int ti_frama_start(TI_REAL const *options) {
      return (int)options[0] - 1;
}


int ti_frama(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) {
      const TI_REAL *high = inputs[0];
      const TI_REAL *low = inputs[1];
      const int period = (int)options[0];
      const int average_period = (int)options[1];
      TI_REAL *output = outputs[0];

      if (period < 1) return TI_INVALID_OPTION;
      if (period % 2 != 0) return TI_INVALID_OPTION;
      if (size <= ti_frama_start(options)) return TI_OKAY;

      const TI_REAL w = log(2.0 / (1.0 + average_period));
      const int half_period = period / 2;

      #define HL2(X) ((high[(X)] + low[(X)]) / 2)
      
      /* our first data point just return (High + Low) / 2 */
      TI_REAL frama = HL2(period - 1);
      *output++ = frama;

      int i, j;
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
            if (alpha < 0.01) alpha = 0.01;
            if (alpha > 1) alpha = 1;

            frama = alpha * HL2(i) + (1 - alpha) * frama;
            *output++ = frama;
      }

      #undef HL

      assert(output - outputs[0] == size - ti_frama_start(options));
      return TI_OKAY;
}
