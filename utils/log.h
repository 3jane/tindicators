/*
 * This file is part of tindicators, licensed under GNU LGPL v3.
 * Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019
 */


#pragma once

#include <stdio.h>

/* Prints value together with its name, useful for debugging purposes */
#define LOG(what) fprintf(stderr, "%s %s = %f\n", __FUNCTION__, #what, (double)(what));
