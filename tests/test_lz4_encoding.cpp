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
  long expected_size = uint16_cube_of_8::size;
  int retcode = SQY_LZ4_Max_Compressed_Length(&expected_size);
  char* compressed = new char[expected_size];

  long output_length = uint16_cube_of_8::size;
  retcode += SQY_LZ4Encode(input,
			      uint16_cube_of_8::size,
			      compressed,
			      &output_length
			      );
  
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(constant_cube[0],to_play_with[0]);
  BOOST_CHECK_LT(output_length,expected_size);
}

BOOST_AUTO_TEST_CASE( encode_length )
{
  

  long input_length = uint16_cube_of_8::size;
  long output_length = uint16_cube_of_8::size;
  int retcode = SQY_LZ4_Max_Compressed_Length(&output_length);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_GT(output_length,input_length);
}

BOOST_AUTO_TEST_CASE( decode_length )
{
  
  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  long* begin = reinterpret_cast<long*>(&constant_cube[0]);
  *begin = uint16_cube_of_8::size_in_byte - sizeof(long);

  long output_length = uint16_cube_of_8::size_in_byte - sizeof(long);

  int retcode = SQY_LZ4_Decompressed_Length(input,&output_length);
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(output_length,uint16_cube_of_8::size_in_byte - sizeof(long));

  output_length = 7;
  retcode = SQY_LZ4_Decompressed_Length(input,&output_length);
  BOOST_CHECK_EQUAL(retcode,1);
  BOOST_CHECK_EQUAL(output_length,0);
}

BOOST_AUTO_TEST_CASE( decode_encoded )
{

  const char* input = reinterpret_cast<char*>(&constant_cube[0]);

  
  long expected_size = uint16_cube_of_8::size_in_byte;
  int retcode = SQY_LZ4_Max_Compressed_Length(&expected_size);
  BOOST_CHECK_EQUAL(retcode,0);

  char* compressed = new char[expected_size];
  long output_length = uint16_cube_of_8::size*sizeof(value_type);
  retcode += SQY_LZ4Encode(input,
			      uint16_cube_of_8::size*sizeof(value_type),
			      compressed,
			      &output_length
			      );
 
  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(output_length,0);
  BOOST_CHECK_LT(output_length,uint16_cube_of_8::size*sizeof(value_type));

  long uncompressed_max_size = output_length;
  retcode += SQY_LZ4_Decompressed_Length(compressed,&uncompressed_max_size);

  BOOST_CHECK_EQUAL(retcode,0);

  char* uncompressed = new char[uncompressed_max_size];
  std::fill(uncompressed,uncompressed + uncompressed_max_size,0);

  retcode += SQY_LZ4Decode(compressed,
			   output_length,
			   //reinterpret_cast<char*>(&to_play_with[0])
			   uncompressed
			   );
  
  BOOST_CHECK_EQUAL(retcode,0);
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


BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( lz4_study, sqeazy::lz4_fixture<unsigned short> )

BOOST_AUTO_TEST_CASE( encoded_and_print_length )
{

  std::map<std::string, std::vector<value_type>* >::iterator begin = data.begin();
  std::map<std::string, std::vector<value_type>* >::iterator end = data.end();

  for(;begin!=end;++begin){
    
    long input_length = begin->second->size()*sizeof(value_type);
    const char* input = reinterpret_cast<char*>(&(*(begin->second))[0]);
    long expected_size = input_length;
					
    SQY_LZ4_Max_Compressed_Length(&expected_size);
    
    char* compressed = new char[expected_size];
    long output_length = 0;
    SQY_LZ4Encode(input,
		  input_length,
		  compressed,
		  &output_length
		  );

    std::cout << "compressed " << begin->first.c_str() << "("<< begin->second->size()<< " elements) reduced from " << input_length << " B to " << output_length << " B, ratio out/in: " << output_length/float(input_length) << "\n";
      
    delete [] compressed;
    BOOST_CHECK_GT(output_length,0);
    
    
  }

  for(begin=data.begin();begin!=end;++begin){
    
    std::cout << begin->first.c_str() << "\t[0:32]\t";
    for(int i = 0;i<32;++i){
      std::cout << begin->second->at(i) << " ";
    }
    std::cout << "\n";
  }


}


BOOST_AUTO_TEST_SUITE_END()
