/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */


#ifndef __TESTING_H__
#define __TESTING_H__

#include "../indicators.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int equal_reals(TI_REAL a, TI_REAL b);

extern int equal_arrays(const TI_REAL *a, const TI_REAL *b, int size_a, int size_b);

extern void print_array(const TI_REAL *a, int size);

extern int compare_answers(const ti_indicator_info *info, TI_REAL *answers[], TI_REAL *outputs[], int answer_size, int output_size);

#ifdef __cplusplus
}
#endif

#endif