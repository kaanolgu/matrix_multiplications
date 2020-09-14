Large Matrix Multiplication (C)
======================

This examples demonstrates techniques that allow user to multiply large matrix multiplications

***KEY CONCEPTS:*** OpenCL API, Synchronize Host and FPGA, Asynchronous Processing, Events, Asynchronous memcpy

***KEYWORDS:*** cl_event, cl::CommandQueue, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, enqueueMigrateMemObjects

##  DESIGN FILES
Application code is located in the src directory.
Examining different sizes rather than 1024x1024 is possible by modifying host.cpp & large_mult.cpp files according to the comments in the code
Accelerator binary files will be compiled to the xclbin directory.
The xclbin directory is required by the Makefile and its contents will be filled during compilation.
A listing of all the files in this example is shown below

```
src/host.cpp
src/large_mult.cpp
```

##  COMMAND LINE ARGUMENTS
Once the environment has been configured, the application can be executed by
```
./execute <large_mult XCLBIN>
```

##  COMMANDS FOR CREATING XCLBIN FOR NIMBIX PLATFORM
Once the environment has been configured, run the following commands : 

Hardware Execution :
```
cd "replace this with project/file/directory"
make all BUILD_DIR=./build/ TARGET=hw DEVICE=xilinx_u200_xdma_201830_2
./execute ./build/large_mult.xclbin
```
Software Emulation :
```
cd "replace this with project/file/directory"
make all BUILD_DIR=./build/ TARGET=sw_emu DEVICE=xilinx_u200_xdma_201830_2 ; export XCL_EMULATION_MODE=sw_emu ; emconfigutil --platform xilinx_u200_xdma_201830_2 --nd 1
./execute ./build/large_mult.xclbin
```
Hardware Emulation :
```
cd "replace this with project/file/directory"
make all BUILD_DIR=./build/ TARGET=hw_emu DEVICE=xilinx_u200_xdma_201830_2 ; export XCL_EMULATION_MODE=hw_emu ; emconfigutil --platform xilinx_u200_xdma_201830_2 --nd 1
./execute ./build/large_mult.xclbin
```
