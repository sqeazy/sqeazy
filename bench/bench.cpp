#define __SQY_BENCH_CPP__
#include <iostream>
#include <functional>
#include <chrono>
#include "bench_fixtures.hpp"


void perform(std::function<int(int,int,int,const char*,char*)> _func, data_fixture _ref, unsigned num_iterations = 50){

  std::vector<double> microsecond_timings(num_iterations);
  
  for(unsigned it = 0;it<num_iterations;++it){
    data_fixture temp = _ref;
    auto t_start = std::chrono::high_resolution_clock::now();
    _func(temp.axis_length, temp.axis_length, temp.axis_length, 
	  reinterpret_cast<char*>(&temp.sin_data[0]),
	  reinterpret_cast<char*>(&temp.output_data[0])
	  )
    auto t_end = std::chrono::high_resolution_clock::now();
    microsecond_timings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count());
  }
  


}


int main(int argc, char *argv[])
{

  data_fixture reference;
  
  std::function<int(int,int,int,const char*,char*)> raster_diff_encode_ushort = SQY_RasterDiffEncode_3D_UI16;
  perform(raster_diff_encode_ushort,reference);
  
  
  return 0;
}
