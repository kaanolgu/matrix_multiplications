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
  Overlap Host Code

  There are many applications where all of the data cannot reside in an FPGA.
  For example, the data is too big to fit in an FPGA or the data is being
  streamed from a sensor or the network. In these situations data must be
  transferred to the host memory to the FPGA before the computation can be
  performed.

  Because PCIe is an full-duplex interconnect, you can transfer data to and from
  the FPGA simultaneously. Xilinx FPGAs can also perform computations during
  these data transfers. Performing all three of these operations at the same
  time allows you to keep the FPGA busy and take full advantage of all of the
  hardware on your system.

  In this example, we will demonstrate how to perform this using an out of order
  command queue.

  +---------+---------+---------+----------+---------+---------+---------
  | WriteA1 | WriteB1 | WriteA2 | Write B2 | WriteA1 | WriteB1 |   Wri...
  +---------+---------+---------+----------+---------+---------+---------
                      |       Compute1     |     Compute2      |  Compu...
                      +--------------------+-------------------+--------+
                                           | ReadC1 |          | ReadC2 |
                                           +--------+          +--------+

  Many OpenCL commands are asynchronous. This means that whenever you call an
  OpenCL function, the function will return before the operation has completed.
  Asynchronous nature of OpenCL allows you to simultaneously perform tasks on
  the host CPU as well as the FPGA.

  Memory transfer operations are asynchronous when the blocking_read,
  blocking_write parameters are set to CL_FALSE. These operations are behaving
  on host memory so it is important to make sure that the command has completed
  before that memory is used.

  You can make sure an operation has completed by querying events returned by
  these commands. Events are OpenCL objects that track the status of operations.
  Event objects are created by kernel execution commands, read, write, copy
  commands on memory objects or user events created using clCreateUserEvent.

  Events can be used to synchronize operations between the host thread and the
  device or between two operations in the same context. You can also use events
  to time a particular operation if the command queue was created using the
  CL_QUEUE_PROFILING_ENABLE flag.

  Most enqueuing commands return events by accepting a cl_event pointer as their
  last argument of the call. These events can be queried using the
  clGetEventInfo function to get the status of a particular operation.

  Many functions also accept event lists that can be used to enforce ordering in
  an OpenCL context. These events lists are especially important in the context
  of out of order command queues as they are the only way specify dependency.
  Normal in-order command queues do not need this because dependency is enforced
  in the order the operation was enqueued. See the concurrent execution example
  for additional details on how create an use these types of command queues.
 */
#include "xcl2.hpp"

#include <algorithm>
#include <cstdio>
#include <random>
#include <vector>

using std::default_random_engine;
using std::generate;
using std::uniform_int_distribution;
using std::vector;
const int columns = 256;
const int rows = 256;
const int ARRAY_SIZE = rows*columns;

void matmul(int *C, int *A, int *B, int M) {
  for (int k = 0; k < M; k++) {
    for (int j = 0; j < M; j++) {
      for (int i = 0; i < M; i++) {
        C[k * M + j] += A[k * M + i] * B[i * M + j];
      }
    }
  }
}
uint64_t get_duration_ns(const cl::Event &event) {
  uint64_t nstimestart, nstimeend;
  cl_int err;
  OCL_CHECK(err, err = event.getProfilingInfo<uint64_t>(
                     CL_PROFILING_COMMAND_START, &nstimestart));
  OCL_CHECK(err, err = event.getProfilingInfo<uint64_t>(
                     CL_PROFILING_COMMAND_END, &nstimeend));
  return (nstimeend - nstimestart);
}


void print(int *data, int columns, int rows) {
  vector<int> out(columns * rows);
  for (int r = 0; r < 16; r++) {
    for (int c = 0; c < 16; c++) {
      printf("%4d ", data[r * columns + c]);
    }
    printf("…\n");
  }
  for (int r = 0; r < 16; r++) {
    printf("   %s ", "…");
  }
  printf("⋱\n\n");
}

void verify(vector<int, aligned_allocator<int>> &gold,
            vector<int, aligned_allocator<int>> &output) {
  for (int i = 0; i < (int)output.size(); i++) {
    if (output[i] != gold[i]) {
      printf("Mismatch %d: gold: %d device: %d\n", i, gold[i], output[i]);
      print(output.data(), 16, 16);
      exit(EXIT_FAILURE);
    }
  }
}

void transpose(int *transpose,int *original){
  for (int r = 0; r < columns; r++) {
    for (int c = 0; c < columns; c++) {
       transpose[c * columns + r] = original[r * columns + c];
    }
  }
}

int gen_random() {
  static default_random_engine e;
  static uniform_int_distribution<int> dist(0, 10);

  return dist(e);
}

// An event callback function that prints the operations performed by the OpenCL
// runtime.
void event_cb(cl_event event1, cl_int cmd_status, void *data) {
  cl_int err;
  cl_command_type command;
  cl::Event event(event1, true);

  OCL_CHECK(err, err = event.getInfo(CL_EVENT_COMMAND_TYPE, &command));
  cl_int status;
  OCL_CHECK(err,
            err = event.getInfo(CL_EVENT_COMMAND_EXECUTION_STATUS, &status));
  const char *command_str;
  const char *status_str;
  switch (command) {
  case CL_COMMAND_READ_BUFFER:
    command_str = "buffer read";
    break;
  case CL_COMMAND_WRITE_BUFFER:
    command_str = "buffer write";
    break;
  case CL_COMMAND_NDRANGE_KERNEL:
    command_str = "kernel";
    break;
  case CL_COMMAND_MAP_BUFFER:
    command_str = "kernel";
    break;
  case CL_COMMAND_COPY_BUFFER:
    command_str = "kernel";
    break;
  case CL_COMMAND_MIGRATE_MEM_OBJECTS:
    command_str = "buffer migrate";
    break;
  default:
    command_str = "unknown";
  }
  switch (status) {
  case CL_QUEUED:
    status_str = "Queued";
    break;
  case CL_SUBMITTED:
    status_str = "Submitted";
    break;
  case CL_RUNNING:
    status_str = "Executing";
    break;
  case CL_COMPLETE:
    status_str = "Completed";
    break;
  }
  printf("[%s]: %s %s\n", reinterpret_cast<char *>(data), status_str,
         command_str);
  fflush(stdout);
}

// Sets the callback for a particular event
void set_callback(cl::Event event, const char *queue_name) {
  cl_int err;
  OCL_CHECK(err,
            err = event.setCallback(CL_COMPLETE, event_cb, (void *)queue_name));
}

int main(int argc, char **argv) {

  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
    return EXIT_FAILURE;
  }

  auto binaryFile = argv[1];
  cl_int err;
  cl::CommandQueue q;
  cl::Context context;
  cl::Kernel krnl_vadd;

  // OPENCL HOST CODE AREA START
  // get_xil_devices() is a utility API which will find the xilinx
  // platforms and will return list of devices connected to Xilinx platform
  std::cout << "Creating Context..." << std::endl;
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
    // This example will use an out of order command queue. The default command
    // queue created by cl::CommandQueue is an inorder command queue.
    OCL_CHECK(err, q = cl::CommandQueue(context, device,
                                        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
                                        &err));

    std::cout << "Trying to program device[" << i
              << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    cl::Program program(context, {device}, bins, NULL, &err);
    if (err != CL_SUCCESS) {
      std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
    } else {
      std::cout << "Device[" << i << "]: program successful!\n";
      OCL_CHECK(err, krnl_vadd = cl::Kernel(program, "vadd", &err));
      valid_device++;
      break; // we break because we found a valid device
    }
  }
  if (valid_device == 0) {
    std::cout << "Failed to program any device found, exit!\n";
    exit(EXIT_FAILURE);
  }

  // We will break down our problem into multiple iterations. Each iteration
  // will perform computation on a subset of the entire data-set.
  size_t elements_per_iteration = 1*columns;
  size_t bytes_per_iteration = elements_per_iteration * sizeof(int);
  size_t num_iterations = ARRAY_SIZE / elements_per_iteration;

  // Allocate memory on the host and fill with random data.
  vector<int, aligned_allocator<int>> A(ARRAY_SIZE);
  vector<int, aligned_allocator<int>> B(ARRAY_SIZE);
  vector<int, aligned_allocator<int>> tB(ARRAY_SIZE);
  vector<int, aligned_allocator<int>> gold(ARRAY_SIZE);
  vector<int, aligned_allocator<int>> device_result(ARRAY_SIZE);

  generate(begin(A), end(A), gen_random);
  generate(begin(B), end(B), gen_random);

  printf("A:\n");
  print(A.data(), columns, rows);
  printf("B:\n");
  print(B.data(), columns, rows);
  clock_t t;
  t = clock(); 
  matmul(gold.data(), A.data(), B.data(), columns);
  t = clock() - t;
  double time_taken_s = ((double)t)/CLOCKS_PER_SEC; // in seconds
  double time_taken_ms = time_taken_s*1000; // in seconds

  printf("Gold:\n");
  print(gold.data(), columns, rows);
  transpose(tB.data(),B.data());


  // THIS PAIR OF EVENTS WILL BE USED TO TRACK WHEN A KERNEL IS FINISHED WITH
  // THE INPUT BUFFERS. ONCE THE KERNEL IS FINISHED PROCESSING THE DATA, A NEW
  // SET OF ELEMENTS WILL BE WRITTEN INTO THE BUFFER.
  vector<cl::Event> kernel_events(2);
  vector<cl::Event> read_events(2);
  cl::Buffer buffer_a[2], buffer_b[2], buffer_c[2];
  
  // Buffer B has the whole matrix so no need to iterate it again and again in the for loop below 
  OCL_CHECK(err, buffer_b[0] = cl::Buffer(
                       context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       bytes_per_iteration*elements_per_iteration,
                       &tB[0], &err));

  buffer_b[1]=buffer_b[0];
  int flag = 0; // make flag initialisation outside of the for loop to decrease execution time

  for (size_t iteration_idx = 0; iteration_idx < num_iterations; iteration_idx++) {
    flag = iteration_idx % 2;

    if (iteration_idx >= 2) {
      OCL_CHECK(err, err = read_events[flag].wait());
    }

    // Allocate Buffer in Global Memory
    // Buffers are allocated using CL_MEM_USE_HOST_PTR for efficient memory and
    // Device-to-host communication
    std::cout << "Creating Buffers..." << std::endl;
    OCL_CHECK(err, buffer_a[flag] = cl::Buffer(
                       context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       bytes_per_iteration,
                       &A[iteration_idx * elements_per_iteration], &err));

    OCL_CHECK(err, buffer_c[flag] = cl::Buffer(
                       context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                       bytes_per_iteration,
                       &device_result[iteration_idx*elements_per_iteration],
                       &err));

    vector<cl::Event> write_event(1);

    OCL_CHECK(err, err = krnl_vadd.setArg(0, buffer_c[flag]));
    OCL_CHECK(err, err = krnl_vadd.setArg(1, buffer_a[flag]));
    OCL_CHECK(err, err = krnl_vadd.setArg(2, buffer_b[flag]));


    // Copy input data to device global memory
    std::cout << "Copying data (Host to Device)..." << std::endl;
    // Because we are passing the write_event, it returns an event object
    // that identifies this particular command and can be used to query
    // or queue a wait for this particular command to complete.
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects(
                       {buffer_a[flag], buffer_b[flag]},
                       0 /*0 means from host*/, NULL, &write_event[0]));
    set_callback(write_event[0], "ooo_queue");

    printf("Enqueueing NDRange kernel.\n");
    // This event needs to wait for the write buffer operations to complete
    // before executing. We are sending the write_events into its wait list to
    // ensure that the order of operations is correct.
    // Launch the Kernel
    std::vector<cl::Event> waitList;
    waitList.push_back(write_event[0]);
    OCL_CHECK(err, err = q.enqueueNDRangeKernel(krnl_vadd, 0, 1, 1, &waitList,
                                                &kernel_events[flag]));
    set_callback(kernel_events[flag], "ooo_queue");
    
    // Copy Result from Device Global Memory to Host Local Memory
    std::cout << "Getting Results (Device to Host)..." << std::endl;
    std::vector<cl::Event> eventList;
    eventList.push_back(kernel_events[flag]);
    // This operation only needs to wait for the kernel call. This call will
    // potentially overlap the next kernel call as well as the next read
    // operations
 
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects(
                       {buffer_c[flag]}, CL_MIGRATE_MEM_OBJECT_HOST, &eventList,
                       &read_events[flag]));
    set_callback(read_events[flag], "ooo_queue");

    OCL_CHECK(err, err = read_events[flag].wait());
  }
 
  
  
  // Wait for all of the OpenCL operations to complete
  printf("Waiting...\n");
  OCL_CHECK(err, err = q.flush());
  OCL_CHECK(err, err = q.finish());
  // OPENCL HOST CODE AREA ENDS
  bool match = true;
  // Verify the results

  verify(gold,device_result);


  // Launch the kernel and get profile data (stop-start)
  uint64_t nstimestart, nstimeend;
  cl::Event event;
  double fpga_exec_time_s=0;
  double avg_fpga_exec_time=0;
  auto duration_nanosec=0;
  for (int i = 0; i < 5; i++)//averaging execution time results for 5 runs
  { 
  OCL_CHECK(err, err = q.enqueueTask(krnl_vadd, NULL, &event));
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


  printf("TEST %s\n", (match ? "PASSED" : "FAILED"));
  return (match ? EXIT_SUCCESS : EXIT_FAILURE);
}
