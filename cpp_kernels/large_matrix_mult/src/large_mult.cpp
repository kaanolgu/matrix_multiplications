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
https://github.com/kaanolgu/matrix_multiplications
  See host code for additional details about this example
  In order to examine other sizes than 1024x1024
  Change "BUFFER_SIZE"  variable to modify matrix size
  Note : it must match the set size on the host file
  No need to change any other variables
*/
  

#define BUFFER_SIZE 1*1024



// TRIPCOUNT indentifier
const unsigned int c_size = BUFFER_SIZE;

extern "C" {
void lmult(int *c, int *a, int *b) {

   int arrayA[BUFFER_SIZE];
   int arrayC[BUFFER_SIZE];
   #pragma HLS array_partition variable=arrayA block 
   #pragma HLS array_partition variable=arrayC block 
   int sum =0;
   int size = BUFFER_SIZE;
  readA:
    for (int j = 0; j < size; j++) {
#pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
#pragma HLS PIPELINE II=1
#pragma HLS UNROLL FACTOR = 2
      arrayA[j] = a[j];
    }

    multiply:
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

