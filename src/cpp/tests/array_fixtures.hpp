#ifndef _ARRAY_FIXTURES_H_
#define _ARRAY_FIXTURES_H_
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <boost/concept_check.hpp>

namespace sqeazy {

template<typename T, unsigned dim = 8>
struct array_fixture {

  static const unsigned axis_length = dim;
  static const unsigned size = dim*dim*dim;
  static const unsigned frame = dim*dim;

  static const unsigned size_in_byte = dim*dim*dim*sizeof(T);
  static const unsigned frame_in_byte = dim*dim*sizeof(T);

  std::vector<T> constant_cube;
  std::vector<T> incrementing_cube;
  std::vector<T> to_play_with;
  std::vector<unsigned> dims;
    
  typedef T value_type;

  array_fixture():
    constant_cube(size,1),
    incrementing_cube(size),
    to_play_with(size,0),
    dims(3, dim)
  {
    auto begin = incrementing_cube.begin();
    auto end = incrementing_cube.end();
      
    size_t value = 0;
    for(; begin!=end; ++begin,++value) {
      *begin = value;
    }
        
  }

  ~array_fixture() {

  }

};

  template<typename T, const unsigned size = 1 << 14 >
  struct lz4_fixture {

    typedef T value_type;

    static const unsigned large_size = 1 << 20;
    static const unsigned large_size_in_byte = large_size*sizeof(T);
    static const unsigned size_in_byte = size*sizeof(T);

    static const value_type max_rnd_value = 1 << (sizeof(char)*8);

    std::vector<T> constant_zeros	;
    std::vector<T> constant_ones	;
    std::vector<T> constant_1024	;
    std::vector<T> constant_1025	;
    std::vector<T> ramp_256		;
    std::vector<T> ramp_16bit		;
    std::vector<T> random		;
    std::vector<T> random_256		;
    std::vector<T> ramp_256_from_minus128		;

    std::map<std::string, std::vector<T>* > data;

    lz4_fixture():
      constant_zeros(size,0),
      constant_ones (size,1),
      constant_1024 (size,1024),
      constant_1025 (size,1025),
      ramp_256      (size,0),
      ramp_16bit    (large_size,0),
      random        (size,0),
      random_256    (size,0),
      ramp_256_from_minus128(size,0),
      data()
    {
      for(long i = 0; i<size; ++i) {
        ramp_256[i] = i % 256;
      }

      for(long i = 0; i<size; ++i) {
        ramp_256_from_minus128[i] = (i % 256);
	if(size<256 && i<size/2)
	  ramp_256_from_minus128[i] -= 128;
      }

      for(long i = 0; i<large_size; ++i) {
        ramp_16bit[i] = i % (1 << 15);
      }

      std::srand(sizeof(T)*8);

      float rnd = 0;
      for(long i = 0; i<size; ++i) {
        rnd = float(std::rand())/RAND_MAX;
        random[i] = rnd*max_rnd_value;
        random_256[i] = static_cast<T>(rnd*max_rnd_value) << sizeof(T)*4;
      }

      data["constant_zeros"] = &constant_zeros       ;
      data["constant_ones "] = &constant_ones        ;
      data["constant_1024 "] = &constant_1024        ;
      data["constant_1025 "] = &constant_1025        ;
      data["ramp_256      "] = &ramp_256             ;
      data["ramp_256_from_minus128"] = &ramp_256_from_minus128             ;
      data["ramp_16bit    "] = &ramp_16bit           ;
      data["random        "] = &random               ;
      data["random_256    "] = &random_256               ;

    }

    ~lz4_fixture() {

    }

    void print(){
  
      typename std::map<std::string, std::vector<value_type>* >::iterator begin = data.begin();
      typename std::map<std::string, std::vector<value_type>* >::iterator end = data.end();
  
  
      for(begin=data.begin();begin!=end;++begin){
    
	std::cout << begin->first.c_str() << "\t[0:32]\t";
	for(int i = 0;i<32;++i){
	  std::cout << begin->second->at(i) << " ";
	}
	std::cout << "\n";
      }
    }

  };


}//sqeazy namespace

#endif














