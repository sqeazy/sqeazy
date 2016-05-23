#define BOOST_TEST_MODULE TEST_SQEAZY_PIPELINES_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>

#include "boost/filesystem.hpp"
#include "array_fixtures.hpp"

#include "sqeazy_pipelines.hpp"

static const std::string default_filter_name = "bitswap1->lz4";
static const std::string default_filter_name_part1 = "bitswap1";
static const std::string default_filter_name_part2 = "lz4";



typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( sqeazy_pipelines, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( roundtrip_bitswap1 ){

  const unsigned long data_bytes = size_in_byte;
  long length = data_bytes;
  
  std::vector<size_t> shape(dims.begin(), dims.end());

  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string(default_filter_name_part1);
  
  int max_encoded_size = pipe.max_encoded_size(data_bytes);
  std::vector<char> intermediate(max_encoded_size,0);
  
  char* encoded_end = pipe.encode(constant_cube.data(),
				  intermediate.data(),
				  shape);
    
  BOOST_REQUIRE(encoded_end!=nullptr);
  length = encoded_end - intermediate.data();
  
  BOOST_CHECK_LT(length,max_encoded_size);
  
  int rvalue = pipe.decode(intermediate.data(),
			   incrementing_cube.data(),
			   length
			   );
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.data(), constant_cube.data()+10,
  				incrementing_cube.data(), incrementing_cube.data()+10); 
  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.data()+size-10, constant_cube.data()+size,
				  incrementing_cube.data()+size-10, incrementing_cube.data()+size);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.data(), constant_cube.data()+size,
				  incrementing_cube.data(), incrementing_cube.data()+size);

}

BOOST_AUTO_TEST_CASE( roundtrip_bitswap1_from_casted ){

  const unsigned long data_bytes = size_in_byte;
  long length = data_bytes;
  


  auto pipe = sqeazy::dypeline_from_char::from_string(default_filter_name_part1);
  
  int max_encoded_size = pipe.max_encoded_size(data_bytes);
  std::vector<char> intermediate(max_encoded_size,0);

  char* encoded_end = pipe.encode((const char*)constant_cube.data(),
				  intermediate.data(),
				  length);
    
  BOOST_REQUIRE(encoded_end!=nullptr);
  length = encoded_end - intermediate.data();
  
  BOOST_CHECK_LT(length,max_encoded_size);
  
  int rvalue = pipe.decode(intermediate.data(),
			   (char*)incrementing_cube.data(),
			   length
			   );
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.data(), constant_cube.data()+10,
  				incrementing_cube.data(), incrementing_cube.data()+10); 
  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.data()+size-10, constant_cube.data()+size,
				  incrementing_cube.data()+size-10, incrementing_cube.data()+size);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.data(), constant_cube.data()+size,
				  incrementing_cube.data(), incrementing_cube.data()+size);

}

BOOST_AUTO_TEST_CASE( roundtrip_lz4 ){

  const unsigned long data_bytes = size_in_byte;
  long length = data_bytes;
  
  std::vector<size_t> shape(dims.begin(), dims.end());

  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string(default_filter_name_part2);
  
  int max_encoded_size = pipe.max_encoded_size(data_bytes);
  std::vector<char> intermediate(max_encoded_size,0);
  
  char* encoded_end = pipe.encode(constant_cube.data(),
				  intermediate.data(),
				  shape);
    
  BOOST_REQUIRE(encoded_end!=nullptr);
  length = encoded_end - intermediate.data();
  
  BOOST_CHECK_LT(length,max_encoded_size);
  
  int rvalue = pipe.decode(intermediate.data(),
			   incrementing_cube.data(),
			   length
			   );
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  // BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
  // 				incrementing_cube.begin(), incrementing_cube.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.data(), constant_cube.data()+10,
				incrementing_cube.data(), incrementing_cube.data()+10);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.data()+size-10, constant_cube.data()+size,
				  incrementing_cube.data()+size-10, incrementing_cube.data()+size);

}


BOOST_AUTO_TEST_CASE( roundtrip ){

  const unsigned long data_bytes = size_in_byte;
  long length = data_bytes;
  
  std::vector<size_t> shape(dims.begin(), dims.end());

  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string(default_filter_name);
  
  int max_encoded_size = pipe.max_encoded_size(data_bytes);
  std::vector<char> intermediate(max_encoded_size,0);
  
  char* encoded_end = pipe.encode(constant_cube.data(),
				  intermediate.data(),
				  shape);
    // SQY_PipelineEncode_UI16(default_filter_name.c_str(),
		      // 			      (const char*)&constant_cube[0],
		      // 			      &ldims[0],
		      // 			      dims.size(),
		      // 			      (char*)&compressed[0],
		      // 			      &length);
    
  BOOST_REQUIRE(encoded_end!=nullptr);
  length = encoded_end - intermediate.data();
  
  BOOST_CHECK_LT(length,max_encoded_size);
  
  int rvalue = pipe.decode(intermediate.data(),
			   incrementing_cube.data(),
			   length
			   );
  
  BOOST_CHECK_EQUAL(rvalue, 0);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.data(), constant_cube.data()+10,
				incrementing_cube.data(), incrementing_cube.data()+10);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.data()+size-10, constant_cube.data()+size,
				  incrementing_cube.data()+size-10, incrementing_cube.data()+size);

  
}
BOOST_AUTO_TEST_SUITE_END()



static const std::string tricky_filter_name = "quantiser->h264";

BOOST_AUTO_TEST_SUITE( tricky_pipelines )

BOOST_AUTO_TEST_CASE( roundtrip_quantiser ){

  av_register_all();

  std::vector<size_t> shape(3,128);
  shape.front() *= 2;
  
  const size_t len = std::accumulate(shape.begin(),
				     shape.end(),
				     1.,
				     std::multiplies<size_t>());
  
  const size_t data_bytes = len*sizeof(std::uint16_t);
  long length = data_bytes;
  
  std::vector<std::uint16_t> inputdata(len,1);
  size_t count = 0;
  for( std::uint16_t& n : inputdata )
    n = 1 << ( ((count++) % 8) + 4);
  std::vector<std::uint16_t> outputdata(len,0);

  BOOST_REQUIRE_EQUAL(shape.size(),3);
  BOOST_REQUIRE_NE(shape.front(),shape.back());
  
  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string("quantiser");
  
  int max_encoded_size = pipe.max_encoded_size(data_bytes);
  std::vector<char> intermediate(max_encoded_size,0);
  
  char* encoded_end = pipe.encode(inputdata.data(),
				  intermediate.data(),
				  shape);
    
  BOOST_REQUIRE(encoded_end!=nullptr);
  length = encoded_end - intermediate.data();
  
  BOOST_CHECK_LT(length,max_encoded_size);
  
  int rvalue = pipe.decode(intermediate.data(),
			   outputdata.data(),
			   length
			   );
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+10,
  				outputdata.data(), outputdata.data()+10); 
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data()+len-10, inputdata.data()+len,
				  outputdata.data()+len-10, outputdata.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+len,
				  outputdata.data(), outputdata.data()+len);

}

BOOST_AUTO_TEST_CASE( roundtrip_quantiser2file ){

  av_register_all();

  std::vector<size_t> shape(3,128);
  shape.front() *= 2;
  
  const size_t len = std::accumulate(shape.begin(),
				     shape.end(),
				     1.,
				     std::multiplies<size_t>());
  
  const size_t data_bytes = len*sizeof(std::uint16_t);
  long length = data_bytes;
  
  std::vector<std::uint16_t> inputdata(len,1);
  size_t count = 0;
  for( std::uint16_t& n : inputdata )
    n = 1 << ( ((count++) % 8) + 4);
  std::vector<std::uint16_t> outputdata(len,0);

  BOOST_REQUIRE_EQUAL(shape.size(),3);
  BOOST_REQUIRE_NE(shape.front(),shape.back());
  
  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string("quantiser(decode_lut_path=singlequant.lut)");
  
  int max_encoded_size = pipe.max_encoded_size(data_bytes);
  std::vector<char> intermediate(max_encoded_size,0);
  
  char* encoded_end = pipe.encode(inputdata.data(),
				  intermediate.data(),
				  shape);
    
  BOOST_REQUIRE(encoded_end!=nullptr);
  length = encoded_end - intermediate.data();
  
  BOOST_CHECK_LT(length,max_encoded_size);
  
  int rvalue = pipe.decode(intermediate.data(),
			   outputdata.data(),
			   length
			   );
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+10,
  				outputdata.data(), outputdata.data()+10); 
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data()+len-10, inputdata.data()+len,
				  outputdata.data()+len-10, outputdata.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+len,
				  outputdata.data(), outputdata.data()+len);

}

BOOST_AUTO_TEST_CASE( roundtrip_quantiser_h264 ){

  av_register_all();

  std::vector<size_t> shape(3,128);
  shape.back() *= 2;
  
  const size_t len = std::accumulate(shape.begin(),
				     shape.end(),
				     1.,
				     std::multiplies<size_t>());
  
  const size_t data_bytes = len*sizeof(std::uint16_t);
  long length = data_bytes;
  
  std::vector<std::uint16_t> inputdata(len,42);
  size_t count = 0;
  for( std::uint16_t& n : inputdata )
    n = 1 << ( ((count++) % 8) + 4);
  std::vector<std::uint16_t> outputdata(len,0);

  BOOST_REQUIRE_EQUAL(shape.size(),3);
  BOOST_REQUIRE_NE(shape.front(),shape.back());
  
  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string(tricky_filter_name);
  
  int max_encoded_size = pipe.max_encoded_size(data_bytes);
  std::vector<char> intermediate(max_encoded_size,0);
  
  char* encoded_end = pipe.encode(inputdata.data(),
				  intermediate.data(),
				  shape);
    
  BOOST_REQUIRE(encoded_end!=nullptr);
  length = encoded_end - intermediate.data();
  
  BOOST_CHECK_LT(length,max_encoded_size);
  
  int rvalue = pipe.decode(intermediate.data(),
			   outputdata.data(),
			   length
			   );
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+10,
  				outputdata.data(), outputdata.data()+10); 
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data()+len-10, inputdata.data()+len,
				  outputdata.data()+len-10, outputdata.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+len,
				  outputdata.data(), outputdata.data()+len);

}

BOOST_AUTO_TEST_CASE( roundtrip_quantiser2file_h264 ){

  av_register_all();
  
  
  std::vector<size_t> shape(3,128);
  shape.back() *= 2;
  
  const size_t len = std::accumulate(shape.begin(),
				     shape.end(),
				     1.,
				     std::multiplies<size_t>());
  
  const size_t data_bytes = len*sizeof(std::uint16_t);
  long length = data_bytes;
  
  std::vector<std::uint16_t> inputdata(len,1);
  size_t count = 0;
  for( std::uint16_t& n : inputdata )
    n = 1 << ( ((count++) % 8) + 4);
  std::vector<std::uint16_t> outputdata(len,0);
  
  BOOST_REQUIRE_EQUAL(shape.size(),3);
  BOOST_REQUIRE_NE(shape.front(),shape.back());

  std::string filter_name = "quantiser(decode_lut_path=test.lut)->h264";
  
  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string(filter_name);
  
  int max_encoded_size = pipe.max_encoded_size(data_bytes);
  std::vector<char> intermediate(max_encoded_size,0);
  
  char* encoded_end = pipe.encode(inputdata.data(),
				  intermediate.data(),
				  shape);
    
  BOOST_REQUIRE(encoded_end!=nullptr);
  length = encoded_end - intermediate.data();
  
  BOOST_CHECK_LT(length,max_encoded_size);
  
  int rvalue = pipe.decode(intermediate.data(),
			   outputdata.data(),
			   length
			   );
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+10,
  				outputdata.data(), outputdata.data()+10); 
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data()+len-10, inputdata.data()+len,
				  outputdata.data()+len-10, outputdata.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+len,
				  outputdata.data(), outputdata.data()+len);

}
BOOST_AUTO_TEST_SUITE_END()
