#include <stdio.h>
#define LOG(what) fprintf(stderr, "%s %s = %f\n", __FUNCTION__, #what, (double)(what));
