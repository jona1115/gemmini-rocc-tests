// See LICENSE for license details.

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef BAREMETAL
#include <sys/mman.h>
#endif
#include "include/gemmini_testutils.h"

int main() {
#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      exit(1);
    }
#endif

  printf("Flush Gemmini TLB of stale virtual addresses\n");
  gemmini_flush(0);

  printf("Initialize our input and output matrices in main memory\n");
  elem_t In[DIM][DIM];
  elem_t Out[DIM][DIM];

  long int A = 0x5432109876543210LL;
  long int B = 0x0123456789012345LL;

  for (size_t i = 0; i < DIM; i++)
    for (size_t j = 0; j < DIM; j++)
      In[i][j] = i;

  elem_t Identity[DIM][DIM];
  for (size_t i = 0; i < DIM; i++)
    for (size_t j = 0; j < DIM; j++)
      Identity[i][j] = i == j;

  printf("Calculate the scratchpad addresses of all our matrices\n");
  printf("  Note: The scratchpad is \"row-addressed\", where each address contains one matrix row\n");
  size_t In_sp_addr = 0;
  size_t Out_sp_addr = DIM;
  size_t Identity_sp_addr = 2*DIM;

  printf("Move \"In\" matrix from main memory into Gemmini's scratchpad\n");
  gemmini_config_ld(DIM * sizeof(elem_t));
  gemmini_config_st(DIM * sizeof(elem_t));
  gemmini_mvin(In, In_sp_addr);

  printf("Move \"Identity\" matrix from main memory into Gemmini's scratchpad\n");
  gemmini_mvin(Identity, Identity_sp_addr);

  gemmini_preload_lut1(A,B);
  gemmini_preload_lut2(A,B);

  printf("Multiply \"In\" matrix with \"Identity\" matrix with a bias of 0\n");
  gemmini_config_ex(WEIGHT_STATIONARY, 0, 0);
  gemmini_preload(Identity_sp_addr, Out_sp_addr);
  gemmini_compute_preloaded(In_sp_addr, GARBAGE_ADDR);

  printf("Move \"Out\" matrix from Gemmini's scratchpad into main memory\n");
  gemmini_mvout(Out, Out_sp_addr);

  printf("Fence till Gemmini completes all memory operations\n");
  gemmini_fence();

  printf("0x%x\n", Out[0][0]);
  printf("0x%x\n", Out[0][1]);
  printf("0x%x\n", Out[0][2]);
  printf("0x%x\n", Out[0][3]);
  printf("0x%x\n", Out[1][0]);
  printf("0x%x\n", Out[1][1]);
  printf("0x%x\n", Out[1][2]);
  printf("0x%x\n", Out[1][3]);
  
  printf("Check whether \"In\" and \"Out\" matrices are identical\n");
  if (!is_equal(In, Out)) {
    printf("Input and output matrices are different!\n");
    printf("\"In\" matrix:\n");
    printMatrix(In);
    printf("\"Out\" matrix:\n");
    printMatrix(Out);
    printf("\n");

    exit(1);
  }

  printf("Input and output matrices are identical, as expected\n");
  exit(0);
}
