#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_LZ4_ENCODING
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>

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
  
  long input_in_bytes = size_in_byte;
  long expected_size = size_in_byte;

  int retcode = SQY_LZ4_Max_Compressed_Length(&expected_size);

  char* compressed = new char[expected_size];
  long output_length = size_in_byte;
  retcode += SQY_LZ4Encode(input,
			      uint16_cube_of_8::size*sizeof(value_type),
			      compressed,
			      &output_length
			      );

  BOOST_CHECK_NE(output_length,input_in_bytes);
    
  retcode = SQY_LZ4_Decompressed_Length(compressed,&output_length);
  BOOST_CHECK_EQUAL(retcode,0);
  unsigned expected = uint16_cube_of_8::size_in_byte;
  
  BOOST_CHECK_EQUAL(output_length, expected);

  
  delete [] compressed;
}

BOOST_AUTO_TEST_CASE( decode_encoded )
{

  const char* input = reinterpret_cast<char*>(&constant_cube[0]);

  
  long expected_size = size_in_byte;
  int retcode = SQY_LZ4_Max_Compressed_Length(&expected_size);
  BOOST_CHECK_EQUAL(retcode,0);

  char* compressed = new char[expected_size];
  long output_length = size_in_byte;
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

  long hdr_size = output_length;
  SQY_Header_Size(compressed,&hdr_size);
  BOOST_CHECK_GT(hdr_size,0);
  BOOST_CHECK_LT(hdr_size,30);

  char* uncompressed = new char[uncompressed_max_size];
  std::fill(uncompressed,uncompressed + uncompressed_max_size,0);

  retcode += SQY_LZ4Decode(compressed,
			   output_length,
			   //reinterpret_cast<char*>(&to_play_with[0])
			   uncompressed
			   );
  
  BOOST_CHECK_EQUAL(retcode,0);


  
  value_type* uncompressed_right = reinterpret_cast<value_type*>(&uncompressed[0]);
  BOOST_CHECK_EQUAL(uncompressed_right[0],constant_cube[0]);
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
    long output_length = expected_size;
    SQY_LZ4Encode(input,
		  input_length,
		  compressed,
		  &output_length
		  );

    std::cout << "compressed " << begin->first.c_str() << "("<< begin->second->size()<< " elements) reduced from " << input_length << " B to " << output_length << " B, ratio out/in: " << output_length/float(input_length) << "\n";
      
    delete [] compressed;
    try{
    BOOST_REQUIRE_GT(output_length,0);
    }
    catch(...){
      break;
      
    }
    
    
  }

  print();


}

BOOST_AUTO_TEST_SUITE_END()

typedef sqeazy::lz4_fixture<unsigned short,64> unsigned_64elements;
typedef sqeazy::lz4_fixture<short,64> signed_64elements;

//BOOST_FIXTURE_TEST_SUITE( lz4_print, unsigned_64elements )


BOOST_FIXTURE_TEST_CASE( print_lz4_input_unsigned , unsigned_64elements)
{

  std::map<std::string, std::vector<value_type>* >::iterator begin = data.begin();
  std::map<std::string, std::vector<value_type>* >::iterator end = data.end();

  print();
  
  for(;begin!=end;++begin){
    
    long input_length = begin->second->size();
    
    if(input_length>64)
      continue;
    
    long input_length_in_byte = begin->second->size()*sizeof(value_type);
    
    const char* input = reinterpret_cast<char*>(&(*(begin->second))[0]);
    char* output = new char[input_length_in_byte];
    
    std::cout << "bitswap4 " << begin->first.c_str() << " as input\n";
    std::bitset<16> current;
    for (int i = 0 ; i < input_length; ++i) {
	current = std::bitset<16>(begin->second->at(i));
	if((i+1) % 8 == 0)
	  std::cout << current.to_string() << "("<< begin->second->at(i)<<")\n";
	else
	  std::cout << current.to_string() << "("<< begin->second->at(i)<<"), ";
    }
    
    std::cout << "\noutput of BitSwap4Encode:\n";
    SQY_BitSwap4Encode_UI16(input,output,input_length_in_byte);
    std::bitset<8> current_byte;
    for (int i = 0 ; i < input_length_in_byte; ++i) {
	current_byte = std::bitset<8>(output[i]);
	if((i+1) % 16 == 0)
	  std::cout << current_byte.to_string() << "\n";
	else
	  std::cout << current_byte.to_string() << ", ";
    }
    
    std::cout << "\noutput of BitSwap1Encode:\n";
    SQY_BitSwap1Encode_UI16(input,output,input_length_in_byte);
    
    for (int i = 0 ; i < input_length_in_byte; ++i) {
	current_byte = std::bitset<8>(output[i]);
	if((i+1) % 16 == 0)
	  std::cout << current_byte.to_string() << "\n";
	else
	  std::cout << current_byte.to_string() << ", ";
    }
    
    std::cout << "\n";
    delete [] output;
  }

  

}

BOOST_FIXTURE_TEST_CASE( print_lz4_input_signed , signed_64elements)
{

   print();
  
  std::map<std::string, std::vector<value_type>* >::iterator begin = data.begin();
  std::map<std::string, std::vector<value_type>* >::iterator end = data.end();

  for(;begin!=end;++begin){
    
    long input_length = begin->second->size();
    
    if(input_length>64)
      continue;
    
    long input_length_in_byte = begin->second->size()*sizeof(value_type);
    
    const char* input = reinterpret_cast<char*>(&(*(begin->second))[0]);
    char* output = new char[input_length_in_byte];
    
    std::cout << "bitswap4 " << begin->first.c_str() << " as input\n";
    std::bitset<16> current;
    for (int i = 0 ; i < input_length; ++i) {
	current = std::bitset<16>(begin->second->at(i));
	if((i+1) % 8 == 0)
	  std::cout << current.to_string() << "("<< begin->second->at(i)<<")\n";
	else
	  std::cout << current.to_string() << "("<< begin->second->at(i)<<"), ";
    }
    
    std::cout << "\noutput of BitSwap4Encode:\n";
    SQY_BitSwap4Encode_I16(input,output,input_length_in_byte);
    std::bitset<8> current_byte;
    for (int i = 0 ; i < input_length_in_byte; ++i) {
	current_byte = std::bitset<8>(output[i]);
	if((i+1) % 16 == 0)
	  std::cout << current_byte.to_string() << "\n";
	else
	  std::cout << current_byte.to_string() << ", ";
    }
    
    std::cout << "\noutput of BitSwap1Encode:\n";
    SQY_BitSwap1Encode_I16(input,output,input_length_in_byte);
    
    for (int i = 0 ; i < input_length_in_byte; ++i) {
	current_byte = std::bitset<8>(output[i]);
	if((i+1) % 16 == 0)
	  std::cout << current_byte.to_string() << "\n";
	else
	  std::cout << current_byte.to_string() << ", ";
    }
    
    std::cout << "\n";
    delete [] output;
  }

  

}

// BOOST_AUTO_TEST_SUITE_END()
