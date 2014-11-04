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

#include "tiff_utils.h"

namespace sqeazy_bench {

struct data_interface {
  
  virtual char* bytes() = 0;

  virtual unsigned long size_in_byte() const = 0;
  
  virtual void fill_from(const std::string& _path) = 0;

};

template <typename T = unsigned short, bool verbose = true>
struct tiff_fixture : public data_interface {

  std::string file_loc;
  std::vector<T> tiff_data;
  std::vector<unsigned> axis_lengths;

  unsigned long axis_length(const int& _axis_ref) const {
    return axis_lengths.at(_axis_ref);
  }

  T* data(){
    
    return &tiff_data[0];
    
  }
  
  
  T const * data() const {
    
    return &tiff_data[0];
    
  }


  char* bytes(){
    
    return reinterpret_cast<char*>(&tiff_data[0]);
    
  }
  
  
  
  void fill_from(const std::string& _path){
    if(!file_exists(_path) || _path.empty()){
      std::stringstream msg("");
      msg << "unable to load file at path " << _path << "\n";
      throw std::runtime_error(msg.str().c_str());
    }

    TIFF* stack_tiff   = TIFFOpen( _path.c_str() , "r" );
    std::vector<tdir_t> stack_tdirs   ;
    sqeazy_bench::get_tiff_dirs(stack_tiff,   stack_tdirs  );

    axis_lengths = sqeazy_bench::extract_tiff_to_vector(stack_tiff,   stack_tdirs  , tiff_data     );
    
    TIFFClose(stack_tiff);
      
    if(tiff_data.size()>0 && verbose){
      
      std::cout << *this << "successfully loaded \n";
    }

  }

  void swap(tiff_fixture& _first, tiff_fixture& _second){
    std::swap(_first.file_loc, _second.file_loc);
    std::swap(_first.tiff_data, _second.tiff_data);
    std::swap(_first.axis_length, _second.axis_length);
  }

  tiff_fixture():
    file_loc(""),
    tiff_data(),
    axis_lengths()
  {

  }

  tiff_fixture(const std::string& _fileloc):
    file_loc(_fileloc),
    tiff_data(),
    axis_lengths()
  {
    fill_from(_fileloc);
  }


  //copy-constructor: reinit everything (don't copy)
  tiff_fixture(const tiff_fixture& _rhs):
    file_loc(_rhs.file_loc),
    tiff_data(_rhs.tiff_data),
    axis_lengths(_rhs.axis_lengths){

    
  }

  
  tiff_fixture& operator=(tiff_fixture _rhs){

    swap(*this,_rhs);
    return *this;
    
  }
  
  unsigned long size_in_byte() const {
    return tiff_data.size()*sizeof(T);
  }
  
  
  unsigned long size() const {
    
    return tiff_data.size();
    
  }
  

  friend std::ostream& operator<<(std::ostream& _cout, const tiff_fixture& _self){
    _cout << "loaded " << _self.file_loc << " as ";
    for(int axis_id = 0;axis_id<_self.axis_lengths.size();++axis_id){
      _cout << _self.axis_length(axis_id) << ((axis_id < _self.axis_lengths.size()-1) ? "x" : " ");
    }
    _cout <<" size("<< typeid(T).name() <<") = " << _self.size_in_byte()/(1<<20) << " MB ";
    return _cout;
  }

  bool empty() const {
    return tiff_data.empty();
  }
};

};

#endif /* _BENCH_FIXTURES_H_ */
