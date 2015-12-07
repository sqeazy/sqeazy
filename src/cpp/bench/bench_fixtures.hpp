#ifndef _BENCH_FIXTURES_H_
#define _BENCH_FIXTURES_H_
#include <vector>
#include <cmath>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

template <typename T = unsigned short, 
	  //obtained by `getconf -a /usr|grep -i CACHE` on a Intel(R) Core(TM) i7-3520M
	  const unsigned cache_size_in_byte_as_exponent = 22
	  >
struct synthetic_fixture{

  static const unsigned long size = (1 << (cache_size_in_byte_as_exponent+1))/sizeof(T);
  static const unsigned long size_in_byte = sizeof(T)*size;

  std::vector<T> sin_data = std::vector<T>(size,0);
  std::vector<T> output_data;

  unsigned long axis_length() const {
    return std::pow(size,1.f/3.f);
  }

  void fill_self(){
    float factor_frequency = .25f*axis_length();
    static const float scale = .25f*std::numeric_limits<T>::max();
    unsigned index = 0;
    for( T& _element : sin_data ){
      _element = scale*std::sin(factor_frequency*index++);
    }
  }

  synthetic_fixture():
    sin_data(size,0),
    output_data(sin_data)
  {

    fill_self();
  }

  //copy-constructor: reinit everything (don't copy)
  synthetic_fixture(const synthetic_fixture& _rhs):
    sin_data(size,0),
    output_data(sin_data){

    *this = _rhs;
    
  }

  //assignment: reinit everything (don't copy)
  synthetic_fixture& operator=(const synthetic_fixture& _rhs){

    if(this!=&_rhs){
      
      std::copy(_rhs.sin_data.begin(), _rhs.sin_data.end(), sin_data.begin());
      std::copy(_rhs.output_data.begin(), _rhs.output_data.end(), output_data.begin());

    }
    
    return *this;
    
  }
  
  unsigned long data_in_byte() const {
    return sin_data.size()*sizeof(T);
  }

  friend std::ostream& operator<<(std::ostream& _cout, const synthetic_fixture& _self){
    _cout << _self.axis_length() <<"x"<< _self.axis_length() <<"x"<< _self.axis_length()
	  <<" uint16 = " << _self.data_in_byte()/(1<<20) << " MB";
    return _cout;
  }
};

bool file_exists(const std::string& _path){
  std::ifstream f(_path.c_str());
  if (f.good()) {
    f.close();
    return true;
  } else {
    f.close();
    return false;
  }
}


#endif /* _BENCH_FIXTURES_H_ */
