#ifndef __SIMPLE1_H__
#define __SIMPLE1_H__

/* This is used for the simple functions that
 * take one input vectors and apply a unary
 * operator for a single output.
 * (e.g. sqrt, sin)
 */

/* Fallback */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#define SIMPLE1(START, FUN, OP) \
int START(TI_REAL const *options) { \
    (void)options; \
    return 0; \
} \
 \
 \
int FUN(int size, TI_REAL const *const *inputs, TI_REAL const *options, TI_REAL *const *outputs) { \
    const TI_REAL *in1 = inputs[0]; \
    TI_REAL *output = outputs[0]; \
 \
    (void)options; \
 \
    for (int i = 0; i < size; ++i) { \
        output[i] = (OP); \
    } \
 \
    return TI_OKAY; \
} \

#endif /*__SIMPLE1_H__*/
