#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_DIFF_SCHEMES
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include "array_fixtures.hpp"
#include "sqeazy.h"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

BOOST_FIXTURE_TEST_SUITE( diff_scheme_encode_out_of_place, uint16_cube_of_8 )
 

BOOST_AUTO_TEST_CASE( encode_success )
{
  
  const char* input = reinterpret_cast<char*>(&incrementing_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

int retcode = SQY_RasterDiffEncode_3D_UI16(uint16_cube_of_8::axis_length,
					   uint16_cube_of_8::axis_length,
					   uint16_cube_of_8::axis_length,
					   input,
					   output);
  
  BOOST_CHECK_EQUAL(retcode,0);
}

BOOST_AUTO_TEST_SUITE_END()
