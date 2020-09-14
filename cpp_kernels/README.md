Vitis Acceleration Examples & Large Matrix Multiplication Example
==================================
This section contains examples stored in there respective folders, inside these folders the size of square matrix are mentioned with *examplename_size*

 __Examples Table__ 

Example        | Description           | Key Concepts / Keywords 
---------------|-----------------------|-------------------------
[array_partition/][]|This is a simple example of matrix multiplication (Row x Col) to demonstrate how to achieve better performance by array partitioning, using HLS kernel in Vitis Environment.|__Key__ __Concepts__<br> - Kernel Optimization<br> - HLS C Kernel<br> - Array Partition<br>__Keywords__<br> - #pragma HLS ARRAY_PARTITION<br> - complete
[loop_reorder/][]|This is a simple example of matrix multiplication (Row x Col) to demonstrate how to achieve better pipeline II factor by loop reordering.|__Key__ __Concepts__<br> - Kernel Optimization<br> - Loop reorder to improve II<br>__Keywords__<br> - #pragma HLS ARRAY_PARTITION
[plram_access/][]|This example shows the usage of PLRAM and how to use it with simple matrix multiplication (Row x Col).|__Key__ __Concepts__<br> - Vitis Memory Hierarchy<br> - PLRAMs<br>__Keywords__<br> - PLRAM
[systolic_array/][]|This is a simple example of matrix multiplication (Row x Col) to help developers learn systolic array based algorithm design. Note : Systolic array based algorithm design is well suited for FPGA.|
[large_mult/][]|This is simple example of large size matrix multuplication to demonstrate multiplications between square matrices beyond the size of 128x128. Generated with the guidance of the Vitis Acceleration Examples|__Key__ __Concepts__<br> - Kernel to DDR<br> - Xilinx Alveo U200<br> - Nimbix<><br> - Large Matrix Multiplication

[.]:.
[array_partition/]:array_partition/
[loop_reorder/]:loop_reorder/
[plram_access/]:plram_access/
[systolic_array/]:systolic_array/
[large_mult/]:large_matrix_mult/
