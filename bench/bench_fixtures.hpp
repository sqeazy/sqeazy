#ifndef _BENCH_FIXTURES_H_
#define _BENCH_FIXTURES_H_
#include <vector>
#include <cmath>
#include <limits>

template <typename T = unsigned short, 
	  const unsigned L1_size_in_byte_as_exponent = 15,
	  const unsigned L2_size_in_byte_as_exponent = 18,
	  const unsigned L3_size_in_byte_as_exponent = 22
	  >
struct data_fixture{

  static const unsigned long size = (1 << (L3_size_in_byte_as_exponent+1))/sizeof(T);
  static const unsigned long axis_length = 1 << ((L3_size_in_byte_as_exponent+1)/3);

  std::vector<T> sin_data = std::vector<T>(size,0);
  std::vector<T> output_data;

  constexpr static const float scale = .25f*std::numeric_limits<T>::max();
  constexpr static const float factor_frequency = .25f*axis_length;

  void fill_self(){
    unsigned index = 0;
    for( T& _element : sin_data ){
      _element = scale*std::sin(factor_frequency*index++);
    }
  }

  data_fixture():
    sin_data(),
    output_data(sin_data)
  {

    fill_self();
  }

  //copy-constructor: reinit everything (don't copy)
  data_fixture(const data_fixture& _rhs):
    sin_data(),
    output_data(sin_data){

    fill_self();
    
  }

  //assignment: reinit everything (don't copy)
  data_fixture& operator=(const data_fixture& _rhs){

    if(this!=*_rhs){
      fill_self();
      std::copy(sin_data.begin(), sin_data.end(), output_data.begin());
    }
    
    return *this;
    
  }
  
  
};



#endif /* _BENCH_FIXTURES_H_ */
