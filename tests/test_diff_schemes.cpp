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
  const unsigned first_index = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length+uint16_cube_of_8::axis_length+1;
  BOOST_CHECK_NE(incrementing_cube[first_index],to_play_with[first_index]);
  const unsigned center_index = size/2;
  BOOST_CHECK_NE(incrementing_cube[center_index],to_play_with[center_index]);
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
  
  BOOST_REQUIRE_EQUAL_COLLECTIONS(&constant_cube[0],
				  &constant_cube[0] + uint16_cube_of_8::axis_length,
				  &input_unpacked[0],
				  &input_unpacked[0] + uint16_cube_of_8::axis_length);


  //the output from decoding the encoded input should be identical
  try{
    BOOST_REQUIRE_EQUAL_COLLECTIONS(&constant_cube[0],
				  &constant_cube[0] + uint16_cube_of_8::size,
				  &input_unpacked[0],
				  &input_unpacked[0] + uint16_cube_of_8::size);
  }
  catch(...){
    const unsigned len = uint16_cube_of_8::size;
    const unsigned len_1d = uint16_cube_of_8::axis_length;
    std::cout << "intermediate:\n";
    print_3d_array(intermediate, len , len_1d);
    std::cout << "\ninput_unpacked:\n";
    print_3d_array(input_unpacked, len , len_1d);
  }
  

}

BOOST_AUTO_TEST_CASE( encode_decode_incrementing_injective )
{
    
  const char* input_casted = reinterpret_cast<char*>(&incrementing_cube[0]);

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
  BOOST_CHECK_EQUAL(incrementing_cube[0],input_unpacked[0]);

  //first line should be equal
  
  BOOST_REQUIRE_EQUAL_COLLECTIONS(&incrementing_cube[0],
				  &incrementing_cube[0] + uint16_cube_of_8::axis_length,
				  &input_unpacked[0],
				  &input_unpacked[0] + uint16_cube_of_8::axis_length);


  //the output from decoding the encoded input should be identical
  try{
    BOOST_REQUIRE_EQUAL_COLLECTIONS(&incrementing_cube[0],
				  &incrementing_cube[0] + uint16_cube_of_8::size,
				  &input_unpacked[0],
				  &input_unpacked[0] + uint16_cube_of_8::size);
  }
  catch(...){
    const unsigned len = uint16_cube_of_8::size;
    const unsigned len_1d = uint16_cube_of_8::axis_length;
    std::cout << "intermediate:\n";
    print_3d_array(intermediate, len , len_1d);
    std::cout << "\ninput_unpacked:\n";
    print_3d_array(input_unpacked, len , len_1d);
  }
  

}

BOOST_AUTO_TEST_SUITE_END()


