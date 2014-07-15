#ifndef _BENCH_UTILS_HPP_
#define _BENCH_UTILS_HPP_


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
    _func(temp.axis_length(), temp.axis_length(), temp.axis_length(), 
	  reinterpret_cast<const char*>(&temp.sin_data[0]),
	  reinterpret_cast<char*>(&temp.output_data[0])
	  );
    auto t_end = std::chrono::high_resolution_clock::now();
    microsecond_timings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count());
  }
  
  double mean = std::accumulate(microsecond_timings.begin(), 
				microsecond_timings.end(),0)/num_iterations;
  double variance = sample_variance(microsecond_timings.begin(), 
				    microsecond_timings.end(),mean);
  
  std::cout << num_iterations << "x runs: " << mean << " +/- " << variance << " us (var = "<< variance/mean <<" * mean); throughput: "<< (_ref.data_in_byte()*1e6)/(mean*(1 << 20))<<" MB/s\n";
  
}

#endif /* _BENCH_UTILS_HPP_ */
