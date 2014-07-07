#ifndef _BENCH_FIXTURES_H_
#define _BENCH_FIXTURES_H_

#include <cmath>
#include <limits

template <typename T = unsigned short, 
	  const unsigned L1_size_in_byte = 1 << 15,
	  const unsigned L2_size_in_byte = 1 << 18,
	  const unsigned L3_size_in_byte = 1 << 22
	  >
struct data_fixture{

  static const unsigned long size = 2*L3_size_in_byte/sizeof(T);
  static const unsigned long axis_length = std::pow(size,1/3.);

  std::vector<T> sin_data = std::vector<T>(::size,0);
  std::vector<T> output_data;

  static const float scale = .25f*std::limits<T>::max();
  static const float factor_frequency = .25f*axis_length;

  void fill_self(){
    unsigned index = 0;
    for( T& _element : sin_data ){
      _element = scale*std::sin(factor_frequency*index++);
    }
  }

  data_fixture():
    static_data(),
    output_data(sin_data)
  {

    fill_self();
  }

  //copy-constructor: reinit everything (don't copy)
  data_fixture(const data_fixture& _rhs):
    static_data(),
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
