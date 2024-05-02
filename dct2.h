#pragma once

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "dct1.h"

#define PI 3.141592653589793
#define SQH 0.707106781186547  /* square root of 2 */

void dct2(float** x, int n);
void idct2(float** x, int n);


