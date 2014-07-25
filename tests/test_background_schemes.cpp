#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_DIFF_SCHEMES
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"

extern "C" {
#include "sqeazy.h"
}

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

template <typename T,typename U>
void print_3d_array(const T* _data, const U& _len, const U& _1d_len){

  for(U i = 0;i<_len;++i){
    if((i) % (_1d_len*_1d_len) == 0)
      std::cout << ">> "<< i/(_1d_len*_1d_len)<<"\n";
    
    std::cout << _data[i] << " ";
      
      if((i+1) % _1d_len == 0)
	std::cout << "\n";
      if((i+1) % (_1d_len*_1d_len) == 0)
	std::cout << "\n";
  }
  std::cout << "\n";
}

BOOST_FIXTURE_TEST_SUITE( remove_background, uint16_cube_of_8 )
 

BOOST_AUTO_TEST_CASE( background_removed_success )
{
  
  char* input = reinterpret_cast<char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_RmBackground_AtMode_UI16(input,
					     output,
					     uint16_cube_of_8::size_in_byte,
					     1);
  
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(to_play_with[0],constant_cube[0]);
  BOOST_CHECK_NE(to_play_with[16],constant_cube[16]);

  retcode = SQY_RmBackground_AtMode_UI16(input,
					 0,
					 uint16_cube_of_8::size_in_byte,
					 1);
  
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(1,constant_cube[0]);

}


BOOST_AUTO_TEST_CASE( auto_background_removed_success )
{
  
  char* input = reinterpret_cast<char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_RmBackground_Estimated_UI16(input,
					     output,
					     uint16_cube_of_8::size_in_byte);
  
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(to_play_with[0],constant_cube[0]);
  BOOST_CHECK_NE(to_play_with[16],constant_cube[16]);

  retcode = SQY_RmBackground_Estimated_UI16(input,
					 0,
					 uint16_cube_of_8::size_in_byte);
  
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(1,constant_cube[0]);

}


BOOST_AUTO_TEST_SUITE_END()


