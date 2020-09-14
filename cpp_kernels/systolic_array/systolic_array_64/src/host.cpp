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

/*******************************************************************************

Description:

    This is a matrix multiplication which showcases the "Systolic Array" based
    algorithm design. Systolic array type of implementation is well suited for
    FPGAs. It is a good coding practice to convert base algorithm into Systolic
    Array implementation if it is feasible to do so.

*******************************************************************************/
#include "xcl2.hpp"
#include <vector>

// Array Size to access
#define DATA_SIZE 64

// Maximum Array Size
#define MAX_SIZE 64

// Software implementation of Matrix Multiplication
// The inputs are of the size (DATA_SIZE x DATA_SIZE)
void m_softwareGold(
    std::vector<int, aligned_allocator<int>> &in1, // Input Matrix 1
    std::vector<int, aligned_allocator<int>> &in2, // Input Matrix 2
    std::vector<int, aligned_allocator<int>> &out  // Output Matrix
    ) {
  // Perform Matrix multiply Out = In1 x In2
  for (int i = 0; i < DATA_SIZE; i++) {
    for (int j = 0; j < DATA_SIZE; j++) {
      for (int k = 0; k < DATA_SIZE; k++) {
        out[i * DATA_SIZE + j] +=
            in1[i * DATA_SIZE + k] * in2[k * DATA_SIZE + j];
      }
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
    return EXIT_FAILURE;
  }

  std::string binaryFile = argv[1];

  // Allocate Memory in Host Memory
  if (DATA_SIZE > MAX_SIZE) {
    std::cout << "Size is bigger than internal buffer size, please use a "
                 "size smaller than "
              << MAX_SIZE << "!" << std::endl;
    return EXIT_FAILURE;
  }

  size_t matrix_size = DATA_SIZE * DATA_SIZE;
  size_t matrix_size_bytes = sizeof(int) * matrix_size;
  cl_int err;
  cl::CommandQueue q;
  cl::Context context;
  cl::Kernel krnl_systolic_array;

  std::vector<int, aligned_allocator<int>> source_in1(matrix_size);
  std::vector<int, aligned_allocator<int>> source_in2(matrix_size);
  std::vector<int, aligned_allocator<int>> source_hw_results(matrix_size);
  std::vector<int, aligned_allocator<int>> source_sw_results(matrix_size);

  // Create the test data and Software Result
  for (size_t i = 0; i < matrix_size; i++) {
    source_in1[i] = i % 10;
    source_in2[i] = i % 10;
    source_sw_results[i] = 0;
    source_hw_results[i] = 0;
  }

  // OPENCL HOST CODE AREA START
  auto devices = xcl::get_xil_devices();

  // read_binary_file() is a utility API which will load the binaryFile
  // and will return the pointer to file buffer.
  auto fileBuf = xcl::read_binary_file(binaryFile);
  cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
  int valid_device = 0;
  for (unsigned int i = 0; i < devices.size(); i++) {
    auto device = devices[i];
    // Creating Context and Command Queue for selected Device
    OCL_CHECK(err, context = cl::Context(device, NULL, NULL, NULL, &err));
    OCL_CHECK(err, q = cl::CommandQueue(context, device,
                                        CL_QUEUE_PROFILING_ENABLE, &err));

    std::cout << "Trying to program device[" << i
              << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    cl::Program program(context, {device}, bins, NULL, &err);
    if (err != CL_SUCCESS) {
      std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
    } else {
      std::cout << "Device[" << i << "]: program successful!\n";
      OCL_CHECK(err, krnl_systolic_array = cl::Kernel(program, "mmult", &err));
      valid_device++;
      break; // we break because we found a valid device
    }
  }
  if (valid_device == 0) {
    std::cout << "Failed to program any device found, exit!\n";
    exit(EXIT_FAILURE);
  }

  // Allocate Buffer in Global Memory
  OCL_CHECK(err, cl::Buffer buffer_in1(
                     context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                     matrix_size_bytes, source_in1.data(), &err));
  OCL_CHECK(err, cl::Buffer buffer_in2(
                     context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                     matrix_size_bytes, source_in2.data(), &err));
  OCL_CHECK(err, cl::Buffer buffer_output(
                     context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                     matrix_size_bytes, source_hw_results.data(), &err));

  int a_row = DATA_SIZE;
  int a_col = DATA_SIZE;
  int b_col = DATA_SIZE;

  OCL_CHECK(err, err = krnl_systolic_array.setArg(0, buffer_in1));
  OCL_CHECK(err, err = krnl_systolic_array.setArg(1, buffer_in2));
  OCL_CHECK(err, err = krnl_systolic_array.setArg(2, buffer_output));
  OCL_CHECK(err, err = krnl_systolic_array.setArg(3, a_row));
  OCL_CHECK(err, err = krnl_systolic_array.setArg(4, a_col));
  OCL_CHECK(err, err = krnl_systolic_array.setArg(5, b_col));

  // Copy input data to device global memory
  OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in1, buffer_in2},
                                                  0 /* 0 means from host*/));

  // Launch the Kernel
  OCL_CHECK(err, err = q.enqueueTask(krnl_systolic_array));
  q.finish();

  // Copy Result from Device Global Memory to Host Local Memory
  OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output},
                                                  CL_MIGRATE_MEM_OBJECT_HOST));
  q.finish();
  // OPENCL HOST CODE AREA END

  // Compute Software Results

  // Compute Software Results
  clock_t t;
  t = clock(); 
  m_softwareGold(source_in1, source_in2, source_sw_results);
  t = clock() - t;
  double time_taken_s = ((double)t)/CLOCKS_PER_SEC; // in seconds
  double time_taken_ms = time_taken_s*1000; // in seconds
  // Compare the results of the Device to the simulation
  int match = 0;
  for (int i = 0; i < DATA_SIZE * DATA_SIZE; i++) {
    if (source_hw_results[i] != source_sw_results[i]) {
      std::cout << "Error: Result mismatch" << std::endl;
      std::cout << "i = " << i << " CPU result = " << source_sw_results[i]
                << " Device result = " << source_hw_results[i] << std::endl;
      match = 1;
      break;
    }
  }
// Launch the kernel and get profile data (stop-start)
  cl::Event event;
  uint64_t nstimestart, nstimeend;
  double fpga_exec_time_s=0;
  double avg_fpga_exec_time=0;
  auto duration_nanosec=0;
  for (int i = 0; i < 5; i++)//averaging execution time results for 5 runs
  { 
  OCL_CHECK(err, err = q.enqueueTask(krnl_systolic_array, NULL, &event));
  OCL_CHECK(err, err = q.finish());
  OCL_CHECK(err, err = event.getProfilingInfo<uint64_t>(
                     CL_PROFILING_COMMAND_START, &nstimestart));
  OCL_CHECK(err, err = event.getProfilingInfo<uint64_t>(
                     CL_PROFILING_COMMAND_END, &nstimeend));
  duration_nanosec = nstimeend - nstimestart;
  fpga_exec_time_s += double((duration_nanosec * (1.0e-6))); // conversion to miliseconds
  }
  avg_fpga_exec_time = fpga_exec_time_s/5;
  ////////////
  

  printf("|-------------------------+-------------------------|\n"
         "| Kernel                  |    Wall-Clock Time (ns) |\n"
         "|-------------------------+-------------------------|\n");
  printf("| %-23s | %21f ms|\n", "FPGA", avg_fpga_exec_time);
  printf("| %-23s | %21f ms|\n","CPU",time_taken_ms);
  printf("|-------------------------+-------------------------|\n");
  printf("| Speedup:  %23f                                    | \n", time_taken_ms/avg_fpga_exec_time);
  printf("|-------------------------+-------------------------|\n");
  printf("Note: Wall Clock Time is meaningful for real hardware execution "
         "only, not for emulation.\n");
  printf("Please refer to profile summary for kernel execution time for "
         "hardware emulation.\n");
  std::cout << "TEST " << (match ? "FAILED" : "PASSED") << std::endl;
  return (match ? EXIT_FAILURE : EXIT_SUCCESS);
}
