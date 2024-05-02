

/* 2-D DCT (type II) program */
/* This file contains two subprograms. The first one is "dct2(x,n)",
which performs the forward 2-D DCT, and the second one is "idct2(x,n)",
which performs the inverse 2-D DCT.  The program, dct2 (or idct2),
will replace the input x (2-D square array [0..n-1][0..n-1]) by
its discrete cosine transform (or inverse discrete cosine transform).
The array size is n*n where n must be an integer power of 2. */

#include "dct2.h"

void dct2(float** x, int n)
{
    int i, j;
    float* y;

    y = (float*)calloc(n, sizeof(float));
    if (y == NULL) {
        printf("allocation failure\n");
        exit(1);
    }
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            y[j] = x[j][i];
        dct1(y, n);
        for (j = 0; j < n; j++)
            x[j][i] = y[j];
    }   /* end of loop i */

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            y[j] = x[i][j];
        dct1(y, n);
        for (j = 0; j < n; j++)
            x[i][j] = y[j];
    }   /* end of loop i */

    free(y);
}

/* ----------------------------------------------- */

void idct2(float** x, int n)
{
    int i, j;
    float* y;

    y = (float*)calloc(n, sizeof(float));
    if (y == NULL) {
        printf("allocation failure\n");
        exit(1);
    }
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            y[j] = x[j][i];
        idct1(y, n);
        for (j = 0; j < n; j++)
            x[j][i] = y[j];
    }   /* end of loop i */

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            y[j] = x[i][j];
        idct1(y, n);
        for (j = 0; j < n; j++)
            x[i][j] = y[j];
    }   /* end of loop i */

    free(y);
}

/* ----------------------------------------------- */

