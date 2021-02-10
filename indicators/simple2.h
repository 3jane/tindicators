#ifndef __SIMPLE2_H__
#define __SIMPLE2_H__

/* This is used for the simple functions that
 * take two input vectors and apply a binary
 * operator for a single output.
 * (e.g. add, multiply)
 */

#define SIMPLE2(START, FUN, OP) \
int START(TI_REAL const *options) { \
    (void)options; \
    return 0; \
} \
 \
 \
int FUN(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) { \
    const TI_REAL *in1 = inputs[0]; \
    const TI_REAL *in2 = inputs[1]; \
 \
    (void)options; \
 \
    TI_REAL *output = outputs[0]; \
 \
    int i; \
    for (i = 0; i < size; ++i) { \
        output[i] = (OP); \
    } \
 \
    return TI_OKAY; \
} \



#endif /*__SIMPLE2_H__*/
