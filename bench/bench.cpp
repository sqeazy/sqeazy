#define __SQY_BENCH_CPP__
#include <iostream>
#include <functional>
#include <chrono>
#include <iterator>
#include <numeric>
#include "bench_fixtures.hpp"

extern "C"{
  #include "sqeazy.h"
}

template <typename It, typename V>
V sample_variance(const It& _begin, 
		  const It& _end, 
		  const V& _mean){
  
  typedef typename std::iterator_traits<It>::value_type return_type;
  
  return_type variance = 0;
  auto begin = _begin;
  for(;begin!=_end;++begin){
    variance += (*begin - _mean)*(*begin - _mean);
  }
  
  return_type value = std::sqrt(variance)/((_end - _begin) - 1);
  return value;
}

template <typename fixture_t>
void perform(std::function<int(int,int,int,const char*,char*)> _func, 
	     fixture_t _ref, 
	     unsigned num_iterations = 50){

  std::vector<double> microsecond_timings(num_iterations);
  
  for(unsigned it = 0;it<num_iterations;++it){
    fixture_t temp = _ref;
    auto t_start = std::chrono::high_resolution_clock::now();
    _func(temp.axis_length, temp.axis_length, temp.axis_length, 
	  reinterpret_cast<char*>(&temp.sin_data[0]),
	  reinterpret_cast<char*>(&temp.output_data[0])
	  );
    auto t_end = std::chrono::high_resolution_clock::now();
    microsecond_timings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count());
  }
  
  double mean = std::accumulate(microsecond_timings.begin(), 
				microsecond_timings.end(),0)/num_iterations;
  double variance = sample_variance(microsecond_timings.begin(), 
				    microsecond_timings.end(),mean);
  
  std::cout << "50x runs: " << mean << " +/- " << variance << " ms\n";
  
}


int main(int argc, char *argv[])
{

  data_fixture<> reference;
  
  std::function<int(int,int,int,const char*,char*)> raster_diff_encode_ushort = SQY_RasterDiffEncode_3D_UI16;
  perform(raster_diff_encode_ushort,reference);
  
  
  return 0;
}
