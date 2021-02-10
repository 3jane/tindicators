/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2021
 * Author: Lewis Van Winkle <lv@codeplea.com>, 2016-2018
 */



#include "../indicators.h"
#include "buffer.h"


ti_buffer *ti_buffer_new(int size) {
    const int s = (int)sizeof(ti_buffer) + (size-1) * (int)sizeof(TI_REAL);
    ti_buffer *ret = (ti_buffer*)malloc((unsigned int)s);
    ret->size = size;
    ret->pushes = 0;
    ret->index = 0;
    ret->sum = 0;
    return ret;
}


void ti_buffer_free(ti_buffer *buffer) {
    free(buffer);
}

