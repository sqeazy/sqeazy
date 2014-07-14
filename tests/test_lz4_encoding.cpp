#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_LZ4_ENCODING
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"

extern "C" {
#include "sqeazy.h"
}

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( lz4_out_of_place, uint16_cube_of_8 )
 

BOOST_AUTO_TEST_CASE( encode_success )
{
  
  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  long input_length = uint16_cube_of_8::size;
  unsigned expected_size = SQY_LZ4Length(input, &input_length);
  char* compressed = new char[expected_size];

  long output_length = uint16_cube_of_8::size;
  int retcode = SQY_LZ4Encode(input,
			      uint16_cube_of_8::size,
			      compressed,
			      &output_length
			      );
  
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(constant_cube[0],to_play_with[0]);
  BOOST_CHECK_NE(output_length,input_length);
}

BOOST_AUTO_TEST_CASE( encode_length )
{
  
  const char* input = reinterpret_cast<char*>(&constant_cube[0]);

  long input_length = uint16_cube_of_8::size;
  long output_length = uint16_cube_of_8::size;
  int retcode = SQY_LZ4Length(input,
			      &output_length
			      );
  
  BOOST_CHECK_GT(retcode,input_length+sizeof(long));
}

BOOST_AUTO_TEST_CASE( decode_encoded )
{

  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  long input_length = uint16_cube_of_8::size*sizeof(value_type);
  
  unsigned expected_size = SQY_LZ4Length(input, &input_length);
  char* compressed = new char[expected_size];
  long output_length = uint16_cube_of_8::size*sizeof(value_type);
  int retcode = SQY_LZ4Encode(input,
			      uint16_cube_of_8::size*sizeof(value_type),
			      compressed,
			      &output_length
			      );
  
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(output_length,0);
  BOOST_CHECK_LT(output_length,uint16_cube_of_8::size*sizeof(value_type));

  unsigned uncompressed_max_size = uint16_cube_of_8::size*sizeof(value_type) + sizeof(long);
  char* uncompressed = new char[uncompressed_max_size];
  std::fill(uncompressed,uncompressed + uncompressed_max_size,0);

  retcode = SQY_LZ4Decode(compressed,
			  output_length,
			  //reinterpret_cast<char*>(&to_play_with[0])
			  uncompressed
			  );
  
  BOOST_CHECK_EQUAL(retcode,uint16_cube_of_8::size*sizeof(value_type));
  // BOOST_CHECK_EQUAL(to_play_with[0],constant_cube[0]);
  BOOST_CHECK_EQUAL(uncompressed[0],constant_cube[0]);
  // BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0], &constant_cube[0] + uint16_cube_of_8::size,
  // 				&to_play_with[0], &to_play_with[0] + uint16_cube_of_8::size);

  value_type* uncompressed_right = reinterpret_cast<value_type*>(&uncompressed[0]);
  BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0], &constant_cube[0] + uint16_cube_of_8::size,
  				&uncompressed_right[0], &uncompressed_right[0] + uint16_cube_of_8::size);

  delete [] uncompressed;
  delete [] compressed;
}

// BOOST_AUTO_TEST_CASE( encode_constant_correct )
// {
  
//   const char* input = reinterpret_cast<char*>(&constant_cube[0]);
//   char* output = reinterpret_cast<char*>(&to_play_with[0]);

//   int retcode = SQY_BitSwap4Encode_UI16(input,
// 					output,
// 					uint16_cube_of_8::size);

//   //constant cube has all elements set to 1
//   //BitSwap4Encode splits each element into 4 pieces: Ai, Bi, Ci, Di
//   //the output the is filled as [Ai,..,Bi,..,Ci,..,Di,..]
//   //if the input only contains the value 1 as 16bit integer, the exepected output element 
//   //can be constructed by hand
//   value_type condensed = (constant_cube[0] << 4) + constant_cube[0];
//   condensed += condensed << 8;

//   const unsigned end_three_quarters = .75*uint16_cube_of_8::size;
//   const unsigned expected_sum = (uint16_cube_of_8::size-end_three_quarters)*condensed;

//   BOOST_CHECK_EQUAL(retcode,0);
//   BOOST_CHECK_EQUAL(to_play_with[0],0);
//   BOOST_CHECK_EQUAL(std::accumulate(&to_play_with[0],&to_play_with[0]+ uint16_cube_of_8::size,0),expected_sum);
//   BOOST_CHECK_EQUAL(to_play_with[uint16_cube_of_8::size-1],condensed);

// }

// BOOST_AUTO_TEST_SUITE_END()

// BOOST_FIXTURE_TEST_SUITE( bitswap4x4_scheme_encode_out_of_place, uint16_cube_of_8 )
 

// BOOST_AUTO_TEST_CASE( decode_success )
// {
//   value_type condensed = (constant_cube[0] << 4) + constant_cube[0];
//   condensed += condensed << 8;
//   const unsigned end_three_quarters = .75*uint16_cube_of_8::size;
//   std::fill(&to_play_with[0]+end_three_quarters,&to_play_with[0]+uint16_cube_of_8::size,condensed);

//   const char* input = reinterpret_cast<char*>(&to_play_with[0]);
//   value_type  output[uint16_cube_of_8::size];
//   char* output_casted = reinterpret_cast<char*>(&output[0]);

//   int retcode = SQY_BitSwap4Decode_UI16(input,
// 					output_casted,
// 					uint16_cube_of_8::size);
  
//   BOOST_CHECK_EQUAL(retcode,0);
//   BOOST_CHECK_NE(to_play_with[0],output[0]);

// }

// BOOST_AUTO_TEST_CASE( is_synthetic_input_valid )
// {
//   value_type condensed = (constant_cube[0] << 4) + constant_cube[0];
//   condensed += condensed << 8;
//   const unsigned end_three_quarters = .75*uint16_cube_of_8::size;
//   std::fill(&to_play_with[0],&to_play_with[0]+uint16_cube_of_8::size,0);
//   std::fill(&to_play_with[0]+end_three_quarters,&to_play_with[0]+uint16_cube_of_8::size,condensed);

//   const char* input = reinterpret_cast<char*>(&constant_cube[0]);
//   value_type output[uint16_cube_of_8::size];
//   std::fill(&output[0],&output[0] + uint16_cube_of_8::size,0);
//   char* output_casted = reinterpret_cast<char*>(&output[0]);

//   int retcode = SQY_BitSwap4Encode_UI16(input,
// 					output_casted,
// 					uint16_cube_of_8::size);
  
//   BOOST_CHECK_EQUAL(retcode,0);
//   BOOST_CHECK_EQUAL(to_play_with[0],output[0]);
//   BOOST_CHECK_EQUAL_COLLECTIONS(&output[0], &output[0] + uint16_cube_of_8::size,
// 				&to_play_with[0], &to_play_with[0] + uint16_cube_of_8::size);

// }

// BOOST_AUTO_TEST_CASE( decode_exact )
// {
//   value_type condensed = (constant_cube[0] << 4) + constant_cube[0];
//   condensed += condensed << 8;
//   const unsigned end_three_quarters = .75*uint16_cube_of_8::size;
//   std::fill(&to_play_with[0],&to_play_with[0]+uint16_cube_of_8::size,0);
//   std::fill(&to_play_with[0]+end_three_quarters,&to_play_with[0]+uint16_cube_of_8::size,condensed);

//   const char* input = reinterpret_cast<char*>(&to_play_with[0]);
//   value_type  output[uint16_cube_of_8::size];
//   std::fill(output,output+uint16_cube_of_8::size,0);
//   char* output_casted = reinterpret_cast<char*>(&output[0]);

//   int retcode = SQY_BitSwap4Decode_UI16(input,
// 					output_casted,
// 					uint16_cube_of_8::size);
  
//   BOOST_CHECK_EQUAL(retcode,0);
//   BOOST_CHECK_EQUAL(output[0],constant_cube[0]);
//   BOOST_CHECK_EQUAL_COLLECTIONS(&output[0], &output[0] + uint16_cube_of_8::size,
// 				&constant_cube[0], &constant_cube[0] + uint16_cube_of_8::size);

// }

// BOOST_AUTO_TEST_CASE( decode_encoded )
// {

//   value_type  intermediate[uint16_cube_of_8::size];
//   std::fill(intermediate,intermediate+uint16_cube_of_8::size,0);
//   char* intermediate_casted = reinterpret_cast<char*>(&intermediate[0]);

//   SQY_BitSwap4Encode_UI16(reinterpret_cast<const char*>(&constant_cube[0]),
// 			  intermediate_casted,
// 			  uint16_cube_of_8::size);
  
//   const char* input = reinterpret_cast<const char*>(&intermediate[0]);

//   int retcode = SQY_BitSwap4Decode_UI16(input,
// 					reinterpret_cast<char*>(&to_play_with[0]),
// 					uint16_cube_of_8::size);
  
//   BOOST_CHECK_EQUAL(retcode,0);
//   BOOST_CHECK_EQUAL(to_play_with[0],constant_cube[0]);
//   BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0], &constant_cube[0] + uint16_cube_of_8::size,
// 				&to_play_with[0], &to_play_with[0] + uint16_cube_of_8::size);

// }

// BOOST_AUTO_TEST_CASE( decode_encoded_incrementing )
// {

//   value_type  intermediate[uint16_cube_of_8::size];
//   std::fill(intermediate,intermediate+uint16_cube_of_8::size,1);
//   char* intermediate_casted = reinterpret_cast<char*>(&intermediate[0]);

//   SQY_BitSwap4Encode_UI16(reinterpret_cast<const char*>(&incrementing_cube[0]),
// 			  intermediate_casted,
// 			  uint16_cube_of_8::size);
  
//   const char* input = reinterpret_cast<const char*>(&intermediate[0]);

//   int retcode = SQY_BitSwap4Decode_UI16(input,
// 					reinterpret_cast<char*>(&to_play_with[0]),
// 					uint16_cube_of_8::size);
  
//   BOOST_CHECK_EQUAL(retcode,0);
//   BOOST_CHECK_EQUAL(to_play_with[0],incrementing_cube[0]);
//   BOOST_CHECK_EQUAL_COLLECTIONS(&incrementing_cube[0], &incrementing_cube[0] + uint16_cube_of_8::size,
// 				&to_play_with[0], &to_play_with[0] + uint16_cube_of_8::size);
// }

BOOST_AUTO_TEST_SUITE_END()
