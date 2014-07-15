#ifndef _ARRAY_FIXTURES_H_
#define _ARRAY_FIXTURES_H_
#include <iostream> 
#include <iomanip> 
#include <vector>
#include <cmath>
#include <stdexcept>
#include <string>
#include <sstream>

#include <boost/filesystem.hpp>


namespace sqeazy {

  template<typename T, unsigned dim = 8>
  struct array_fixture {
    
    static const unsigned axis_length = dim;
    static const unsigned size = dim*dim*dim;
    static const unsigned size_in_byte = dim*dim*dim*sizeof(T);

    T constant_cube[size];
    T incrementing_cube[size];
    T to_play_with[size];
    typedef T value_type;
    
    array_fixture():
      constant_cube(),
      incrementing_cube(),
      to_play_with()
    {
      value_type* begin = &constant_cube[0];
      value_type* end = begin + size;
      std::fill(begin,end,1);

      begin = &to_play_with[0];
      end = begin + size;
      std::fill(begin,end,0);
      
      begin = &incrementing_cube[0];
      end = begin + size;
      size_t value = 0;
      for(;begin!=end;++begin,++value){
	*begin = value;
      }
      
    }

    ~array_fixture(){

    }
    
  };
  
}//sqeazy namespace

#endif














