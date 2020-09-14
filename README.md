# MatMul_MultipleCU
Algortihm to Multiply two Large Matrix Size (>256x256) based on Vitis 2020.1 Examples


## Execution Times

> Comparison between execution times
> Number of Compute Units = Matrix Size/Chunk Size = (1024*1024)(128x1024)

>FPGA = Alveo U200<

>Large sizes are not included because they result worse timings.
  

<table>
  <tr >
    <td colspan="8"><b>Multiple Compute Units Algorithm for Large Size Matrix Calculations (Experimental Results) </b></td>
  </tr>
  <tr >
    <td colspan="8"><b><center>1024&times;1024</center></b></td>
  </tr>
  <tr>
    <td>Host</td>
    <td>Wall-Clock Time</td>
    <td>Kernels</td>
    <td>Chunk Size</td>
    <td>Wall-Clock Time</td>
    <td>Speedup</td>
    <td>Total on Chip Power</td>
    <td>Utilization</td>
  </tr>
  <tr>
    <td colspan="8"><b>Buffer Size 1x1024</b></td>
  </tr>
    <tr>
      <td>CPU</td>
      <td>7751.969000 ms</td>
      <td>FPGA</td>
      <td>128&times;1024</td>
      <td>1022.646195 ms</td>
      <td>7.580304</td>
      <td>14.116 W</td>
      <td>14%</td>
      <td>(w/o pragmas)</td>
  </tr>
        <tr>
      <td><i>CPU</i></td>
      <td>7828.195000 ms</td>
      <td>FPGA</td>
      <td>1&times;1024</td>
      <td>8.235801 ms</td>
      <td>950.508032</td>
      <td>x</td>
      <td>x</td>
      <td>UNROLL FACTOR 2 for multiplication and storing at sum </td>
  </tr>
          <tr>
      <td><i>CPU</i></td>
      <td>x</td>
      <td>FPGA</td>
      <td>1&times;1024</td>
      <td>x</td>
      <td>x</td>
      <td>x</td>
      <td>x</td>
      <td>UNROLL FACTOR 2 for multiplication and getting B & A</td>
  </tr>
            <tr>
      <td><i>CPU</i></td>
      <td>7792.434000 ms</td>
      <td>FPGA</td>
      <td>1&times;1024</td>
      <td>8.218766 ms</td>
      <td>948.127006</td>
      <td>x</td>
      <td>x</td>
              <td>matmul_multicu_4kernel</td>
      <td>UNROLL FACTOR (size) for multiplication and getting B & A, 8 kernels at the same time</td>
  </tr>
              <tr>
      <td><i>CPU</i></td>
      <td>7785.573000 ms</td>
      <td>FPGA</td>
      <td>1&times;1024</td>
      <td>8.231457 ms</td>
      <td>945.831704</td>
      <td>x</td>
      <td>x</td>
              <td>matmul_multicu_axi</td>
      <td>UNROLL FACTOR (2) 256 max gmem</td>
  </tr>
      <tr>
      <td><i>CPU</i></td>
      <td>7750.896000 ms</td>
      <td>FPGA</td>
      <td>1&times;1024</td>
      <td>8.245870 ms</td>
      <td>939.973102</td>
      <td>14.15 W</td>
      <td>14 %</td>
  </tr>
      <tr>
      <td><i>CPU</i></td>
      <td> 7767.303000 ms</td>
      <td>FPGA</td>
      <td>2&times;1024</td>
      <td>16.224244 ms</td>
      <td>478.746683</td>
      <td>x</td>
      <td>x</td>
  </tr>
      <tr>
      <td><i>CPU</i></td>
      <td>7889.684000 ms</td>
      <td>FPGA</td>
      <td>4&times;1024</td>
      <td>32.212750 ms</td>
      <td>244.924261</td>
      <td>x</td>
      <td>x</td>
  </tr>
  <tr>
      <td><i>CPU</i></td>
      <td>7808.852000 ms</td>
      <td>FPGA</td>
      <td>16&times;1024</td>
      <td>128.016170 ms</td>
      <td>60.998950</td>
      <td>x</td>
      <td>x</td>
  </tr>
  <tr>
      <td><i>CPU</i></td>
      <td>7772.947000 ms</td>
      <td>FPGA</td>
      <td>32&times;1024</td>
      <td>255.810417 ms</td>
      <td>30.385577</td>
      <td>x</td>
      <td>x</td>
  </tr>
    <tr>
      <td><i>CPU</i></td>
      <td>7774.805000 ms</td>
      <td>FPGA</td>
      <td>64&times;1024</td>
      <td>511.463088 ms</td>
      <td>15.201107</td>
      <td>x</td>
      <td>x</td>
  </tr>
    <tr>
      <td><i>CPU</i></td>
      <td>7751.969000 ms</td>
      <td>FPGA</td>
      <td>128&times;1024</td>
      <td>1022.646195 ms</td>
      <td>7.580304</td>
      <td>x</td>
      <td>x</td>
  </tr>
</table>
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
    <td>BRAM Utilization</td>
  </tr>
    <tr>
      <td>256&times;256</td>
      <td>CPU</td>
      <td>117.243000 ms</td>
      <td>FPGA</td>
      <td>0.816261 ms</td>
      <td>143.634240</td>
      <td>x</td>
      <td>x</td>
  </tr>
  <tr>
      <td>1024&times;1024</td>
      <td><i>CPU</i></td>
      <td> 7761.062000 ms</td>
      <td>FPGA</td>
      <td>8.236390 ms</td>
      <td>942.289325</td>
      <td>14.107 W</td>
      <td>14%</td>
  </tr>
    <tr>
      <td>4096&times;4096</td>
      <td><i>CPU</i></td>
      <td>1846402.084000 ms</td>
      <td>FPGA</td>
      <td> 114.759099 ms</td>
      <td>16089.374149</td>
      <td>14.185 W</td>
      <td>14%</td>  
  </tr>
</table>

-matmul_multicu_axi change vector_addition.cpp  maximum in HLS s_mem 1024 to 256 -> it didn't make any improvement to the 8.20 ms
- When transferred as a single row and nested loops the results terminated by the memory limits so that's why Matrix B is transferred as a whole
