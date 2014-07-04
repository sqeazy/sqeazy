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

BOOST_AUTO_TEST_CASE( encode_check_magic_values )
{
  
  const char* input = reinterpret_cast<char*>(&incrementing_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_RasterDiffEncode_3D_UI16(uint16_cube_of_8::axis_length,
					     uint16_cube_of_8::axis_length,
					     uint16_cube_of_8::axis_length,
					     input,
					     output);
  
  
  BOOST_CHECK_EQUAL(incrementing_cube[0],to_play_with[0]);
  BOOST_CHECK_NE(incrementing_cube[size - 1],to_play_with[size - 1]);
  BOOST_CHECK_EQUAL(retcode,0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( diff_scheme_decode_out_of_place, uint16_cube_of_8 )
 

BOOST_AUTO_TEST_CASE( decode_success )
{
  

  
  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_RasterDiffDecode_3D_UI16(uint16_cube_of_8::axis_length,
					     uint16_cube_of_8::axis_length,
					     uint16_cube_of_8::axis_length,
					     input,
					     output);
  
  BOOST_CHECK_EQUAL(retcode,0);
}

BOOST_AUTO_TEST_CASE( decode_check_magic_values )
{
  
  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_RasterDiffDecode_3D_UI16(uint16_cube_of_8::axis_length,
					     uint16_cube_of_8::axis_length,
					     uint16_cube_of_8::axis_length,
					     input,
					     output);
  
  
  BOOST_CHECK_EQUAL(constant_cube[0],to_play_with[0]);
  BOOST_CHECK_EQUAL(constant_cube[size - 1],to_play_with[size - 1]);
  BOOST_CHECK_EQUAL(retcode,0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( diff_scheme_decode_out_of_place, uint16_cube_of_8 )
BOOST_AUTO_TEST_CASE( encode_decode_constant_success )
{
  

  
  const char* input_casted = reinterpret_cast<char*>(&constant_cube[0]);

  short intermediate[uint16_cube_of_8::size];
  char* intermediate_casted = reinterpret_cast<char*>(&intermediate[0]);

  unsigned short input_unpacked[uint16_cube_of_8::size];
  char* input_unpacked_casted = reinterpret_cast<char*>(&input_unpacked[0]);

  int retcode = SQY_RasterDiffEncode_3D_UI16(uint16_cube_of_8::axis_length,
					     uint16_cube_of_8::axis_length,
					     uint16_cube_of_8::axis_length,
					     input_casted,
					     intermediate_casted);

  retcode += SQY_RasterDiffDecode_3D_UI16(uint16_cube_of_8::axis_length,
					  uint16_cube_of_8::axis_length,
					  uint16_cube_of_8::axis_length,
					  intermediate_casted,
					  input_unpacked_casted);
  
  BOOST_CHECK_EQUAL(retcode,0);
}

BOOST_AUTO_TEST_CASE( encode_decode_constant_injective )
{
    
  const char* input_casted = reinterpret_cast<char*>(&constant_cube[0]);

  short intermediate[uint16_cube_of_8::size];
  char* intermediate_casted = reinterpret_cast<char*>(&intermediate[0]);

  unsigned short input_unpacked[uint16_cube_of_8::size];
  char* input_unpacked_casted = reinterpret_cast<char*>(&input_unpacked[0]);

  int retcode = SQY_RasterDiffEncode_3D_UI16(uint16_cube_of_8::axis_length,
					     uint16_cube_of_8::axis_length,
					     uint16_cube_of_8::axis_length,
					     input_casted,
					     intermediate_casted);

  retcode += SQY_RasterDiffDecode_3D_UI16(uint16_cube_of_8::axis_length,
					  uint16_cube_of_8::axis_length,
					  uint16_cube_of_8::axis_length,
					  intermediate_casted,
					  input_unpacked_casted);
  
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(constant_cube[0],input_unpacked[0]);

  //first line should be equal
  BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],
				&constant_cube[0] + uint16_cube_of_8::axis_length,
				&input_unpacked[0],
				&input_unpacked[0] + uint16_cube_of_8::axis_length);

  //the output from decoding the encoded input should be identical
  BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],
				&constant_cube[0] + uint16_cube_of_8::size,
				&input_unpacked[0],
				&input_unpacked[0] + uint16_cube_of_8::size);

  

}
BOOST_AUTO_TEST_SUITE_END()
