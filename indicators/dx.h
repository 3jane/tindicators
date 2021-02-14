/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2017
 */


#ifndef __DX_H__
#define __DX_H__



/* This is used with the DX family of indicators.
 * e.g. DX ADX ADXR
 */


#include "truerange.h"



#define CALC_DIRECTION(up, down) do {\
        up = high[i] - high[i-1];\
        down = low[i-1] - low[i];\
\
        if (up < 0)\
            up = 0;\
        else if (up > down)\
            down = 0;\
\
        if (down < 0)\
            down = 0;\
        else if (down > up)\
            up = 0;\
} while (0)



#endif /*__DX_H__*/
