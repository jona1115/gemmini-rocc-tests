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

#define ELEM_T_IS_FLOAT 1

int main() {
#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      exit(1);
    }
#endif

  //printf("Flush Gemmini TLB of stale virtual addresses\n");
  gemmini_flush(0);

  //printf("Initialize our input and output matrices in main memory\n");
  elem_t In[DIM][DIM];
  elem_t Out[DIM][DIM];
  elem_t Identity[DIM][DIM];

  for (size_t i = 0; i < DIM; i++) {
    for (size_t j = 0; j < DIM; j++) {
      In[i][j] = i == j ? NN_floatToHalf(1) : 0; //10 + i * DIM + j;
      Identity[i][j] = i == j ? NN_floatToHalf(1) : 0;
    }
  }

  //printFPMatrix(Identity);
  printf("Calculate the scratchpad addresses of all our matrices\n");
  //printf("  Note: The scratchpad is \"row-addressed\", where each address contains one matrix row\n");
  size_t In_sp_addr = 0;
  size_t Out_sp_addr = DIM;
  size_t Identity_sp_addr = 2*DIM;
  const uint32_t acc_addr = 1 << (ADDR_LEN-1);

  printf("Move \"In\" matrix from main memory into Gemmini's scratchpad\n");
  gemmini_config_ld(DIM * sizeof(elem_t));
  gemmini_config_st(DIM * sizeof(elem_t));
  gemmini_mvin(In, In_sp_addr);

  printf("Move \"Identity\" matrix from main memory into Gemmini's scratchpad\n");
  gemmini_mvin(Identity, Identity_sp_addr);

  printf("Multiply \"In\" matrix with \"Identity\" matrix with a bias of 0\n");
  gemmini_config_ex(WEIGHT_STATIONARY, NO_ACTIVATION, 0);
  gemmini_preload(Identity_sp_addr, Out_sp_addr);
  gemmini_compute_preloaded(In_sp_addr, GARBAGE_ADDR);

  printf("Move \"Out\" matrix from Gemmini's scratchpad into main memory\n");
  gemmini_mvout(Out, acc_addr + n*DIM);

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
    printFPMatrix(In);
    printf("\"Out\" matrix:\n");
    printFPMatrix(Out);
    printf("\n");

    printf("FAIL\n");
    exit(1);
  }

  printf("Input and output matrices are identical, as expected\nPASS\n");
  exit(0);
}
