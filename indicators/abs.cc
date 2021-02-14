/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#include <new>

#include "../indicators.h"
#include "simple1.h"


SIMPLE1(ti_abs_start, ti_abs, fabs(in1[i]))
