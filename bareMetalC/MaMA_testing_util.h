/*************************************************************************************************
*  
*  Author   : CPRE 581 Project Group: "SADD MaMA Rocket" (https://github.com/jona1115/chipyard)
*  Date     : 12/16/2024
*  
*  
*  MaMA_testing_util.h:
*  We use some of the functions/macros from the gemmini framework when doing testing/benchmarking,
*  unfortunately the way the program it makes it hard to change the dimension of matrixes easily (
*  they use the "DIM" macro globally). This file has copies of the functions we use to easily change
*  the dimension of matrixes (DIM2). Note, in many of these functions, the change is renaming DIM to
*  DIM2.
*  
*  
*  Note: This file is written by, or written with the aid of, ChatGPT.
*  
*  
*  MODIFICATION HISTORY:
* 
*  Ver          Who       Date	      Changes
*  -----        --------- ---------- ----------------------------------------------
*  1.00         Jonathan  12/16/2024  Created the file
*  
***********************************************************************************************/


#ifndef MaMA_TESTING_UTIL_H
#define MaMA_TESTING_UTIL_H

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "include/gemmini_testutils.h"
#include "include/gemmini_params.h" // maybe dont need

#define MaMA_TEST_SPARSITY 0.75

// from gemmini_params.h
#define DIM2 10

// from gemmini_testutils.h
static void printMatrix2(elem_t m[DIM2][DIM2]) {
  for (size_t i = 0; i < DIM2; ++i) {
    for (size_t j = 0; j < DIM2; ++j)
#ifndef ELEM_T_IS_FLOAT
      printf("%d ", m[i][j]);
#else
      printf("%x ", elem_t_to_elem_t_bits(m[i][j]));
#endif
    printf("\n");
  }
}

// from gemmini_testutils.h
static void matmul2(elem_t A[DIM2][DIM2], elem_t B[DIM2][DIM2], elem_t D[DIM2][DIM2], full_t C_full[DIM2][DIM2]) {
  for (size_t r = 0; r < DIM2; r++)
    for (size_t c = 0; c < DIM2; c++) {
      C_full[r][c] = D[r][c];
      for (size_t k = 0; k < DIM2; k++)
        C_full[r][c] += A[r][k]*B[k][c];
    }
}

#endif