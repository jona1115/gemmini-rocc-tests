/*************************************************************************************************
*  
*  Author   : CPRE 581 Project Group: "SADD MaMA Rocket" (https://github.com/jona1115/chipyard)
*  Date     : 11/17/2024
*  
*  
*  MaMA_matmul_cpu.c:
*  A matmul code that runs on CPU only. This is inspired by riscv-tests/benchmarks/mt-matmul.c, 
*  but using gemmini_testutils.h's functions for fair benchmarking.
*  
*  
*  Note: This file is written by, or written with the aid of, ChatGPT.
*  
*  
*  MODIFICATION HISTORY:
* 
*  Ver          Who       Date	      Changes
*  -----        --------- ---------- ----------------------------------------------
*  1.00         Jonathan  11/17/2024  Created the file
*  1.01         Jonathan  12/16/2024  Removed all the original crap that I can't figure out
*                                     the purpose.
*  
***********************************************************************************************/


#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "include/gemmini_testutils.h"
#include "MaMA_testing_util.h"

// Comment this out to remove all the verbose prints
#define VERBOSE_DEBUG_PRINTS_ON

#ifdef FAST
#define AINIT RELU
#define SINIT 12
// #define N 1
#else
#define AINIT NO_ACTIVATION
#define SINIT 0
// #define N 2
#endif

// How many mm to create, and operate on
#define N 1

void operands(int c, int * a, int * b, int * d) {
  *d = c % N;
  *b = (c / N) % N;
  *a = c / (N*N);
}

int main_part() {
  // srand(28); // Doesnt work with our compiler for some reason

  static elem_t ZERO[DIM2][DIM2] = {0}; // Initialize ZERO matrix with zeros

  static elem_t A[N][DIM2][DIM2];
  static elem_t B[N][DIM2][DIM2];
  static elem_t D[N][DIM2][DIM2];

  // Initialize matrices A, B, and D with random values
  for (size_t n = 0; n < N; ++n) {
    for (size_t i = 0; i < DIM2; ++i) {
      for (size_t j = 0; j < DIM2; ++j) {
        A[n][i][j] = (rand() % 64) - 32;
        B[n][i][j] = (rand() % 64) - 32;
        D[n][i][j] = (rand() % 64) - 32;
      }
    }

    #ifdef VERBOSE_DEBUG_PRINTS_ON
    printf("N:%d matrix is:\nA:\n", n);
    printMatrix2(A[n]);
    printf("\nB:\n");
    printMatrix2(B[n]);
    printf("\nD:\n");
    printMatrix2(D[n]);
    printf("\n===================================\n");
    #endif
  }

  // C is an array of N results matrix
  #ifdef VERBOSE_DEBUG_PRINTS_ON
  printf("+++++++++++++++++++++++++++++++++++\n");
  #endif
  full_t C[N][DIM2][DIM2] = {0};
  for (int n = 0; n < N; ++n) {
    #ifdef VERBOSE_DEBUG_PRINTS_ON
    printf("n == %d\n", n);
    printf("C pre matmul:\n");
    printMatrix2(C[n]);
    printf("\n");
    #endif

    matmul2(A[n], B[n], ZERO, C[n]);
    
    #ifdef VERBOSE_DEBUG_PRINTS_ON
    printf("C post matmul:\n");
    printMatrix2(C[n]);
    printf("\n");

    printf("+++++++++++++++++++++++++++++++++++\n");
    #endif
  }

  #ifdef VERBOSE_DEBUG_PRINTS_ON
  for (int n = 0; n < N; ++n) {
    printf("C[%d]:\n", n);
    printMatrix2(C[n]);
    printf("\n");

    printf("***********************************\n"); 
  }
  #endif

  return 0;
}

// Use the stats stuff
#include "riscv-tests/benchmarks/common/util.h"
int main() {
  int cid = 0;

  printf("Here we go\n");

  stats(main_part(), 1);
  // main_part();
  
  printf("main_part() done!\n");

  exit(0);
}