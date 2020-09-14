# Matrix Multiplications on Xilinx Alveo U200
Algortihm to Multiply two Large Matrix Size (>256x256) based on Vitis 2020.1 Examples
and also matrix multiplication examples from the Vitis Accel Library [ https://github.com/Xilinx/Vitis_Accel_Examples ] 

## Execution Times


>FPGA = Alveo U200

  


<table>
   <tr>
      <td colspan="8"><b>Multiple Compute Units Algorithm for Large Size Matrix Calculations</b></td>
  </tr>
    <tr>
    <td>Size</td>
    <td>Host</td>
    <td>Wall-Clock Time</td>
    <td>Kernel</td>
    <td>Wall-Clock Time</td>
    <td>Speedup</td>
    <td>Total on Chip Power</td>

  </tr>
    <tr>
      <td>256&times;256</td>
      <td>CPU</td>
      <td>117.243000 ms</td>
      <td>FPGA</td>
      <td>140.997 ms</td>
      <td>0.831</td>
      <td>14.105W</td>

  </tr>
  <tr>
      <td>1024&times;1024</td>
      <td><i>CPU</i></td>
      <td>7,761ms</td>
      <td>FPGA</td>
      <td>4,485 ms</td>
      <td>1.73</td>
      <td>14.107 W</td>

  </tr>
    <tr>
      <td>4096&times;4096</td>
      <td><i>CPU</i></td>
      <td>1,846,402 ms</td>
      <td>FPGA</td>
      <td> 241,208 ms</td>
      <td>7.65</td>
      <td>14.185 W</td>

  </tr>
</table>
## Power Consumptions
Will be updated later
