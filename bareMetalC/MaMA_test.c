/*************************************************************************************************
*  
*  Author   : CPRE 581 Project Group: "MaMA for BOOM" (https://github.com/jona1115/chipyard)
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

void print_binary(elem_t value) {
    // Loop over 16 bits and print each bit
    for (int i = 31; i >= 0; i--) {
        // Print bit 0 or 1 by shifting the bit to the right and using bitwise AND with 1
        if((i+1) % 8 == 0 && i != 31)
        {
          printf(" ");
        }
        printf("%d", (value >> i) & 1);
    }
}

//void print_matrix_binary

int main_part() {
#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      exit(1);
    }
#endif

  printf("Flush Gemmini TLB of stale virtual addresses\n");
  gemmini_flush(0);

  printf("Initialize our input and output matrices in main memory\n");
  elem_t A[DIM][DIM] row_align(1); // = {{0,1,2,3}, {4,5,6,7}, {8,9,10,11}, {12,13,14,15}};
  elem_t B[DIM][DIM] row_align(1); // All 1's
  elem_t C[DIM][DIM] row_align(1); // Output
  elem_t D[DIM][DIM] row_align(1); // All 0's


//   elem_t A[DIM][DIM]; // = {{0,1,2,3}, {4,5,6,7}, {8,9,10,11}, {12,13,14,15}};
//   elem_t B[DIM][DIM] = {{2,0,0,0},{0,2,0,0}, {0,0,2,0}, {0,0,0,2}}; // Output // All 1's
//   elem_t C[DIM][DIM];
//   elem_t D[DIM][DIM]; // All 0's


  for (size_t i = 0; i < DIM; i++)
  {
    for (size_t j = 0; j < DIM; j++)
    {
            // Combine to a 16-bit value
            A[i][j] = ((i*DIM+j) << 16) | (i << 2) | (j);
            if (i == j) 
            {
                B[i][j] = (2 << 16) | (i << 2) | (j);
            }
            else
            {
                B[i][j] = (i << 2) | (j);
            }
            D[i][j] = (i << 2) | (j);
            //D[i][j] = 0;
    }
  }
  
  // printf("\"A\" matrix:\n");
  // printMatrix(A);
  // printf("\"B\" matrix:\n");
  // printMatrix(B);



  printf("Calculate the scratchpad addresses of all our matrices\n");
  printf("  Note: The scratchpad is \"row-addressed\", where each address contains one matrix row\n");
  uint32_t A_sp_addr = 0;
  uint32_t B_sp_addr = DIM;
  uint32_t D_sp_addr = 2*DIM; //GARBAGE_ADDR; //2*DIM;

  // size_t C_sp_addr = 3*DIM;
  uint32_t C_addr_acc = 1 << (ADDR_LEN-1); //MSB set high for ADDR when addressing ACCUMULATOR

  // size_t In_sp_addr = 0;
  // size_t Out_sp_addr = DIM;
  // size_t Identity_sp_addr = 2*DIM;

  printf("Move \"A\" matrix from main memory into Gemmini's scratchpad\n");
  gemmini_config_ld(DIM * sizeof(elem_t));
  gemmini_config_st(DIM * sizeof(elem_t));
  gemmini_mvin(A, A_sp_addr);

  printf("Move \"B\" matrix from main memory into Gemmini's scratchpad\n");
  gemmini_mvin(B, B_sp_addr);

  printf("Move \"D\" matrix from main memory into Gemmini's scratchpad\n");
  gemmini_mvin(D, D_sp_addr);

  printf("Config EX\n");
  gemmini_config_ex(WEIGHT_STATIONARY, 0, 0);
  printf("Preload \"B\" weight matrix and \"C\" output matrix address\n");
  gemmini_preload(B_sp_addr, C_addr_acc); 
  //gemmini_preload_zeros(C_addr_acc);
  printf("Multiply \"A\" matrix with \"B\" matrix with a bias of D Matrix\n");
  gemmini_compute_preloaded(A_sp_addr, D_sp_addr);

  printf("Move \"Out\" matrix from Gemmini's scratchpad into main memory\n");
  gemmini_config_st(DIM * sizeof(elem_t));
  gemmini_mvout(C, C_addr_acc & ~(1 << (ADDR_LEN-2) &(1 << (ADDR_LEN-3)))); //BIT 30 set low if writing to the accumulator to overwrite address, BIT 29 set high to READ full data

  printf("Fence till Gemmini completes all memory operations\n");
  gemmini_fence();

  printf("Check whether \"In\" and \"Out\" matrices are identical\n");

  printf("\"A\" matrix:\n");
  printMatrix(A);
  printf("\"C\" matrix:\n");
  printMatrix(C);

  for (int i = 0; i < DIM; i++) {
    for (int j = 0; j < DIM; j++) {

        elem_t A_value = A[i][j];
        int16_t A_top_half = (A_value >> 16);
        int16_t A_row_bottom = (A_value) & 0xFFFF;

        printf("   A[%d][%d] = 0b", i, j);
        print_binary(A_value);
    }
    printf("\n");
  }

  printf("\n");

  for (int i = 0; i < DIM; i++) {
      for (int j = 0; j < DIM; j++) {
          elem_t C_value = C[i][j];
          // Extract the top 8 bits: data
          int16_t C_top_half = (C_value >> 16);
          // Extract the 2 bits for row index (bits 7-6)
          int16_t C_row_bottom = (C_value) & 0xFFFF;
          //  unsigned short C_value = C[i][j];
          // // Extract the top 8 bits: data
          // unsigned char C_top_half = (C_value >> 8) & 0xFF;
          // // Extract the 2 bits for row index (bits 7-6)
          // unsigned char C_row_index = (C_value >> 6) & 0x03;
          // // Extract the 2 bits for column index (bits 5-4)
          // unsigned char C_col_index = (C_value >> 4) & 0x03;
          // printf("C[%d][%d] = Top 8 bits: %d, Row index (bits 7-6): %d, Column index (bits 5-4): %d\n",
          //        i, j, C_top_half, C_row_index, C_col_index);
          //printf("C[%d][%d] = 0x%x   ", i, j, C_value);
          printf("   C[%d][%d] = 0b", i, j);
          print_binary(C_value);
      }
      printf("\n");
  }

  printf("Input and output matrices are identical, as expected\n");
  //exit(0);
}


// Use the stats stuff
#include "riscv-tests/benchmarks/common/util.h"
int main() {
  int cid = 0;

  printf("Here we go\n");

  stats(main_part(), 1);
  
  printf("main_part() done!\n");

  exit(0);
}