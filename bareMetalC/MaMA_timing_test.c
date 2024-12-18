/*************************************************************************************************
*  
*  Author   : CPRE 581 Project Group: "SADD MaMA Rocket" (https://github.com/jona1115/chipyard)
*  Date     : 12/4/2024
*  
*  
*  MaMA_test.c:
*  Simple test of configuration to check that tagged data types work
* 
*  
*  
*  MODIFICATION HISTORY:
* 
*  Ver          Who       Date	      Changes
*  -----        --------- ---------- ----------------------------------------------
*  1.00         Steven  12/4/2024   Created the file as copy from MaMA_matmul_ws.c
*  
***********************************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef BAREMETAL
#include <sys/mman.h>
#endif
#include "include/gemmini_testutils.h"

// Use the stats stuff
#include "riscv-tests/benchmarks/common/util.h"

int MaMA(int in_rows, uint32_t A_sp_addr, uint32_t D_sp_addr)
{
  gemmini_extended_compute_preloaded(A_sp_addr, D_sp_addr, 4, in_rows, 4, 4)
}

int main_part() {
    //int cid = 0;

#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      exit(1);
    }
#endif

  gemmini_flush(0);

  elem_t A[DIM][DIM] row_align(1); // = {{0,1,2,3}, {4,5,6,7}, {8,9,10,11}, {12,13,14,15}};
  elem_t B[DIM][DIM] row_align(1); // Identity 2's for scaling
  elem_t C[DIM][DIM] row_align(1); // Output
  elem_t D[DIM][DIM] row_align(1); // All 0's or tags (due to order of operations overwrite)

  
  for (size_t i = 0; i < DIM; i++)
  {
    for (size_t j = 0; j < DIM; j++)
    {
      A[i][j] = 0;
      B[i][j] = (1 << 16) | (i << 2) | (j); // All 1's

    }
  }
  
  A[0][0] = (elem_t) ((2 << 16) | (1 << 15) | (2 << 2) | (0)); //last
  A[0][1] = (elem_t) ((2 << 16) | (0 << 2) | (1));
  A[0][3] = (elem_t) ((2 << 16) | (1 << 15) | (0 << 2) | (3)); //last
  A[1][3] = (elem_t) ((2 << 16) | (1 << 15) | (3 << 2) | (3)); //last



  uint32_t A_sp_addr = 0;
  uint32_t B_sp_addr = DIM;
  uint32_t D_sp_addr = GARBAGE_ADDR; //2*DIM; //GARBAGE_ADDR; 

  // uint32_t C_sp_addr = 3*DIM;
  uint32_t C_addr_acc = 1 << (ADDR_LEN-1); //MSB set high for ADDR when addressing ACCUMULATOR

  
  gemmini_config_ld(DIM * sizeof(elem_t));
  gemmini_config_st(DIM * sizeof(elem_t));

  //gemmini_extended_mvin(dram_addr, spad_addr, cols, rows)
  //gemmini_mvin(A, A_sp_addr);

  gemmini_extended_mvin(A, A_sp_addr, 4, 4)

  gemmini_mvin(B, B_sp_addr);

  gemmini_mvin(D, D_sp_addr);

  gemmini_config_ex(WEIGHT_STATIONARY, 0, 0);

  //gemmini_extended_preload(BD, C, BD_cols, BD_rows, C_cols, C_rows)
  gemmini_extended_preload(B_sp_addr, C_addr_acc, 4, 2, 4, 4)

  //gemmini_extended_compute_preloaded(A, BD, A_cols, A_rows, BD_cols, BD_rows)
  gemmini_extended_compute_preloaded(A_sp_addr, D_sp_addr, 4, 2, 4, 4)

  // printf("Stats: 4x4\n");
  // stats(MaMA(4, A_sp_addr, D_sp_addr), 1);
  // printf("Stats: 2x4\n");
  // stats(MaMA(2, A_sp_addr, D_sp_addr), 1);
  // printf("Stats: 4x4\n");
  // stats(MaMA(4, A_sp_addr, D_sp_addr), 1);
  // printf("Stats: 2x4\n");
  // stats(MaMA(2, A_sp_addr, D_sp_addr), 1);
  

  //gemmini_extended_compute_preloaded(A_sp_addr, D_sp_addr, 4, 2, 4, 4)
  //gemmini_extended_compute_preloaded(A_sp_addr, D_sp_addr, 4, a_rows, 4, 4)


  gemmini_config_st(DIM * sizeof(elem_t));
  gemmini_mvout(C, C_addr_acc & ~(1 << (ADDR_LEN-2) &(1 << (ADDR_LEN-3)))); //BIT 30 set low if writing to the accumulator to overwrite address, BIT 29 set high to READ full data

  gemmini_fence();

}



int main() {
  int cid = 0;

  printf("Here we go: input rows of 4\n");

  stats(main_part(), 1);
  
  printf("main_part() done!\n");

  //main_part();

  exit(0);
}