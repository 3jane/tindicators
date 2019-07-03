#pragma once

#include <stdio.h>

/* Prints value together with its name, useful for debugging purposes */
#define LOG(what) fprintf(stderr, "%s %s = %f\n", __FUNCTION__, #what, (double)(what));
