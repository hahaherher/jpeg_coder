#pragma once

/* 1-D DCT (type II) program  */
/* This file contains three subprograms. The first one is "dct1(x,n)",
which performs the forward 1-D DCT. The second one is "idct1(x,n)",
which performs the inverse 1-D DCT.  The program, dct1 (or idct1),
will replace the input x (1-D array [0..n-1]) by its discrete cosine
transform (or inverse discrete cosine transform).  The array size "n"
must be an integer power of 2.
The third program is "fft1(data,nn,isign)", which performs the 1-D FFT.
Please note that the programs "dct1" and "idct1" are computed
via "fft1.c".      */

#include <stdio.h>
#include <cstdlib >
#include <math.h>
#include <malloc.h>

#define PI 3.141592653589793
#define SQH 0.707106781186547  /* square root of 2 */
#define SWAP(a,b)  tempr=(a); (a) = (b); (b) = tempr

static void fft1();

void dct1(float* x, int n);
void idct1(float* x, int n);


/* 1-D fft program  */
/* Replace data by its discrete Fourier ransform if isign is input as 1,
   or replace data by its inverse discrete Fourier transform if
   isign is input as -1.  "data" is a complex array of length nn, input
   as a real array data [1..2*nn], nn must be an integer power of 2.     */

   /* If your data array is zero-offset, that is the range of data is
      [0..2*nn-1], you have to decrease the pointer to data by one when
      fft1 is invoked, for example fft1(data-1,256,1).
      The real part of the first output will now be return in data[0],
      the imaginary part in data[1] and so on.                              */

static void fft1(float* data, int nn, int isign);