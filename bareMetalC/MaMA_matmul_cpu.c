/*************************************************************************************************
*  
*  Author   : CPRE 581 Project Group: "MaMA for BOOM" (https://github.com/jona1115/chipyard)
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
*  1.00         Jonathan  11/17/2024   Created the file
*  
***********************************************************************************************/


#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "include/gemmini_testutils.h"

#ifdef FAST
#define AINIT RELU
#define SINIT 12
#define N 1
#else
#define AINIT NO_ACTIVATION
#define SINIT 0
#define N 2
#endif

void operands(int c, int * a, int * b, int * d) {
  *d = c % N;
  *b = (c / N) % N;
  *a = c / (N*N);
}

int main_part() {
  // srand(28);

  static elem_t ZERO[DIM][DIM] = {0}; // Initialize ZERO matrix with zeros

  for (int activation = AINIT; activation <= RELU; ++activation) {
#ifdef ACC_SCALE_T_IS_FLOAT
    for (acc_scale_t scale = 0; scale <= 1.5; scale += 0.5) {
#else
    for (acc_scale_t scale = SINIT; scale <= 12; scale += 4) {
#endif
      static elem_t A[N][DIM][DIM];
      static elem_t B[N][DIM][DIM];
      static elem_t D[N][DIM][DIM];

      // Initialize matrices A, B, and D with random values
      for (size_t n = 0; n < N; ++n) {
        for (size_t i = 0; i < DIM; ++i) {
          for (size_t j = 0; j < DIM; ++j) {
            A[n][i][j] = (rand() % 64) - 32;
            B[n][i][j] = (rand() % 64) - 32;
            D[n][i][j] = (rand() % 64) - 32;
          }
        }
      }

      // We will try out every combination of A, B, D possible
      static elem_t C[N*N*N][DIM][DIM];
      static full_t gold_full[N*N*N][DIM][DIM];
      static elem_t gold[N*N*N][DIM][DIM];

      // Generate control sequences
      static int preload[N*N*N] = {1};
      for (int i = 1; i < N*N*N; ++i)
        preload[i] = rand() % 2;

      static int add_to_zeros[N*N*N];
      for (int i = 0; i < N*N*N; ++i)
        add_to_zeros[i] = rand() % 2;

      static int accumulate[N*N*N] = {0};
      for (int i = 1; i < N*N*N; ++i)
        accumulate[i] = rand() % 2;

      static int no_output[N*N*N];
      for (int i = 0; i < N*N*N-1; ++i)
        no_output[i] = accumulate[i+1];
      no_output[N*N*N-1] = 0;

      // Compute the expected results (golden results)
      for (size_t g = 0; g < N*N*N; ++g) {
        int a, b, d;
        operands(g, &a, &b, &d);

        if (add_to_zeros[g])
          matmul(A[a], B[b], ZERO, gold_full[g]);
        else
          matmul(A[a], B[b], D[d], gold_full[g]);

        if (accumulate[g])
          matadd(gold_full[g], gold_full[g-1], gold_full[g]);
      }

      for (size_t g = 0; g < N*N*N; ++g) {
        matscale(gold_full[g], gold[g], scale);
        if (activation == RELU)
          matrelu(gold[g], gold[g]);
      }

      // Compute the actual results using CPU computations
      for (size_t g = 0; g < N*N*N; ++g) {
        int a, b, d;
        operands(g, &a, &b, &d);

        elem_t temp[DIM][DIM];
        if (add_to_zeros[g])
          matmul(A[a], B[b], ZERO, temp);
        else
          matmul(A[a], B[b], D[d], temp);

        if (accumulate[g])
          matadd(temp, C[g-1], temp);

        matscale(temp, C[g], scale);

        if (activation == RELU)
          matrelu(C[g], C[g]);

        if (!no_output[g]) {
          // Compare the computed result with the golden result
          if (!is_equal(C[g], gold[g])) {
            printf("Mismatch at step %d\n", g);
            printf("Activation: %d, Scale: %d\n", activation, scale);
            printf("Computed C[%d]:\n", g);
            printMatrix(C[g]);
            printf("Expected Gold[%d]:\n", g);
            printMatrix(gold[g]);
            return -1; // Indicate failure
            // return 0; // Indicate failure
          }
          else {
            printf("Step %d: No mismatch!\n", g);
            // printf("Activation: %d, Scale: %d\n", activation, scale);
            // printf("Computed C[%d]:\n", g);
            // printMatrix(C[g]);
            // printf("Expected Gold[%d]:\n", g);
            // printMatrix(gold[g]);
          }
        }
        else {
          printf("Step %d: !no_output[g] false\n", g);
        }
      }
    }
  }

  printf("All computations match expected results.\n");
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