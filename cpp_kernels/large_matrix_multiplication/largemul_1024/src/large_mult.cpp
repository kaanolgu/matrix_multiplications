/**********
Copyright (c) 2019, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/

/*
  See host code for additional details about this example

  */

#define BUFFER_SIZE 1*1024

 int sum =0;
 int size = BUFFER_SIZE;

// TRIPCOUNT indentifier
const unsigned int c_size = BUFFER_SIZE;

extern "C" {
void lmult(int *c, int *a, int *b) {

   int arrayA[BUFFER_SIZE];
   int arrayC[BUFFER_SIZE];
   #pragma HLS array_partition variable=arrayA block 
   #pragma HLS array_partition variable=arrayC block 

  readA:
    for (int j = 0; j < size; j++) {
#pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
#pragma HLS PIPELINE II=1
#pragma HLS UNROLL FACTOR = 2
      arrayA[j] = a[j];
    }

    // Read data from global memory and write into local buffer for in2

  // FPGA implementation, local array is mostly implemented as BRAM Memory
  // block.
  // BRAM Memory Block contains two memory ports which allow two parallel access
  // to memory. To utilized both ports of BRAM block, vector addition loop is
  // unroll with factor of 2. It is equivalent to following code:
  //  for (int j = 0 ; j < chunk_size ; j+= 2){
  //    vout_buffer[j]   = v1_buffer[j] + v2_buffer[j];
  //    vout_buffer[j+1] = v1_buffer[j+1] + v2_buffer[j+1];
  //  }
  // Which means two iterations of loop will be executed together and as a
  // result
  // it will double the performance.
  // Auto-pipeline is going to apply pipeline to this loop
    multiply_writeC:
  for (int i = 0; i < size*size; i+=BUFFER_SIZE) {
    for (int j = 0; j < size; j++) {
      
#pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
#pragma HLS PIPELINE II=1
#pragma HLS UNROLL FACTOR = 2
      sum += arrayA[j] * b[i+j];
    }

    arrayC[i/size]=sum;
    sum=0;

  }
  writeC:
    for (int j = 0; j < size; j++) {
#pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
#pragma HLS PIPELINE II=1
#pragma HLS UNROLL FACTOR = 2
      c[j] = arrayC[j];
    }
  }         
  }

