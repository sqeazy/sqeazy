#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_BITSWAP_SCHEMES
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"

extern "C" {
#include "sqeazy.h"
}

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( bitswap4x4_scheme_encode_out_of_place, uint16_cube_of_8 )
 

BOOST_AUTO_TEST_CASE( encode_success )
{
  
  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_BitSwap4Encode_UI16(input,
					output,
					uint16_cube_of_8::size);
  
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(constant_cube[0],to_play_with[0]);
}

BOOST_AUTO_TEST_CASE( encode_constant_correct )
{
  
  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_BitSwap4Encode_UI16(input,
					output,
					uint16_cube_of_8::size);
  
  unsigned expected_sum = uint16_cube_of_8::size;
  BOOST_CHECK_NE(std::accumulate(&to_play_with[0],&to_play_with[0] + uint16_cube_of_8::size,0),expected_sum);

}

BOOST_AUTO_TEST_SUITE_END()
