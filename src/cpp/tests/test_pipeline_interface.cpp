#define BOOST_TEST_MODULE TEST_PIPELINE_INTERFACE
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>

#include "boost/filesystem.hpp"
#include "array_fixtures.hpp"

extern "C" {
#include "sqeazy.h"
}

static const std::string default_filter_name = "bitswap1->lz4";
static const std::string deprecated_filter_name = "bswap1_lz4";

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

BOOST_AUTO_TEST_SUITE( pipeline )

BOOST_AUTO_TEST_CASE( does_this_validate ){

  bool answer = SQY_Pipeline_Possible(default_filter_name.c_str());
    
  BOOST_CHECK_EQUAL(answer, true);
  
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( hdf5_inference_queries, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( max_compressed_bytes ){

  const long data_bytes = size_in_byte;
  long length = data_bytes;
  int rvalue = SQY_Pipeline_Max_Compressed_Length_UI16(default_filter_name.c_str(), &length);
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_GT(length,data_bytes);
  
}

BOOST_AUTO_TEST_CASE( max_compressed_bytes_3D ){

  const long data_bytes = size_in_byte;
  long length = data_bytes;
  std::vector<long> ldims(dims.begin(), dims.end());
  int rvalue = SQY_Pipeline_Max_Compressed_Length_3D_UI16(default_filter_name.c_str(),
						     &ldims[0],
						     dims.size(),
						     &length);
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_GT(length,data_bytes);
  
}

BOOST_AUTO_TEST_CASE( decompressed_length ){

  const unsigned long data_bytes = size_in_byte;
  long length = data_bytes;
  std::vector<long> ldims(dims.begin(), dims.end());
  SQY_Pipeline_Max_Compressed_Length_3D_UI16(default_filter_name.c_str(),
					     &ldims[0],
					     dims.size(),
					     &length);
  std::vector<char> compressed(length,0);
  int rvalue = SQY_PipelineEncode_UI16(default_filter_name.c_str(),
				       (const char*)&constant_cube[0],
				       &ldims[0],
				       dims.size(),
				       (char*)&compressed[0],
				       &length);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(length,compressed.size());


  rvalue = SQY_Pipeline_Decompressed_Length(&compressed[0],
					    &length);
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(length, data_bytes);
  
}

BOOST_AUTO_TEST_CASE( roundtrip ){

  const unsigned long data_bytes = size_in_byte;
  long length = data_bytes;
  std::vector<long> ldims(dims.begin(), dims.end());
  SQY_Pipeline_Max_Compressed_Length_3D_UI16(default_filter_name.c_str(),
					     &ldims[0],
					     dims.size(),
					     &length);
  std::vector<char> compressed(length,0);
  int rvalue = SQY_PipelineEncode_UI16(default_filter_name.c_str(),
				       (const char*)&constant_cube[0],
				       &ldims[0],
				       dims.size(),
				       (char*)&compressed[0],
				       &length);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(length,compressed.size());
  
  rvalue = SQY_PipelineDecode_UI16((const char*)&compressed[0],
				   length,
				   (char*)&incrementing_cube[0]
			      );
  
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
				incrementing_cube.begin(), incrementing_cube.end());
  
}
BOOST_AUTO_TEST_SUITE_END()

static const std::string tricky_filter_name = "quantiser->h264";

BOOST_AUTO_TEST_SUITE( video_codecs )

BOOST_AUTO_TEST_CASE( roundtrip ){

  std::vector<size_t> shape(3,128);
  shape.front() *= 2;
  
  const size_t len = std::accumulate(shape.begin(),
				     shape.end(),
				     1.,
				     std::multiplies<size_t>());
  
  const unsigned long data_bytes = len*sizeof(std::uint16_t);
  long length = data_bytes;

  std::vector<std::uint16_t> inputdata(len,1);
  std::size_t count=0;
  for( std::uint16_t& n : inputdata )
    n = 1 << (count++ % 8);
  
  std::vector<std::uint16_t> outputdata(len,0);

  
  SQY_Pipeline_Max_Compressed_Length_3D_UI16(tricky_filter_name.c_str(),
					     (long*)shape.data(),
					     shape.size(),
					     &length);
  std::vector<char> compressed(length,0);
  int rvalue = SQY_PipelineEncode_UI16(tricky_filter_name.c_str(),
				       (const char*)inputdata.data(),
				       (long*)shape.data(),
				       shape.size(),
				       compressed.data(),
				       &length);
  BOOST_REQUIRE_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(length,compressed.size());
  
  rvalue = SQY_PipelineDecode_UI16((const char*)compressed.data(),
				   length,
				   (char*)outputdata.data()
			      );
  
  BOOST_REQUIRE_EQUAL(rvalue, 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+10,
				  outputdata.data(), outputdata.data()+10); 
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data()+len-10, inputdata.data()+len,
				  outputdata.data()+len-10, outputdata.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+len,
				  outputdata.data(), outputdata.data()+len);
  
  // BOOST_CHECK_EQUAL_COLLECTIONS(inputdata.begin(), inputdata.end(),
  // 				outputdata.begin(), outputdata.end()
  // 				);
  
}


BOOST_AUTO_TEST_CASE( quantiser_only ){

  const std::string filter_name = "quantiser";
  
  std::vector<size_t> shape(3,128);
  shape.front() *= 2;
  
  const size_t len = std::accumulate(shape.begin(),
				     shape.end(),
				     1.,
				     std::multiplies<size_t>());
  
  const unsigned long data_bytes = len*sizeof(std::uint16_t);
  long length = data_bytes;

  std::vector<std::uint16_t> inputdata(len,1);
  std::size_t count=0;

  for( std::uint16_t& n : inputdata )
    n = 1 << (count++ % 8);
  
  std::vector<std::uint16_t> outputdata(len,0);

  
  SQY_Pipeline_Max_Compressed_Length_3D_UI16(filter_name.c_str(),
					     (long*)shape.data(),
					     shape.size(),
					     &length);
  std::vector<char> compressed(length,0);
  int rvalue = SQY_PipelineEncode_UI16(filter_name.c_str(),
				       (const char*)inputdata.data(),
				       (long*)shape.data(),
				       shape.size(),
				       compressed.data(),
				       &length);
  BOOST_REQUIRE_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(length,compressed.size());
  
  rvalue = SQY_PipelineDecode_UI16((const char*)compressed.data(),
				   length,
				   (char*)outputdata.data()
			      );
  
  BOOST_REQUIRE_EQUAL(rvalue, 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+10,
				  outputdata.data(), outputdata.data()+10); 
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data()+len-10, inputdata.data()+len,
				  outputdata.data()+len-10, outputdata.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+len,
				  outputdata.data(), outputdata.data()+len);
  
  // BOOST_CHECK_EQUAL_COLLECTIONS(inputdata.begin(), inputdata.end(),
  // 				outputdata.begin(), outputdata.end()
  // 				);
  
}
BOOST_AUTO_TEST_SUITE_END()
