#define BOOST_TEST_MODULE TEST_PIPELINE_SELECT
#include "boost/test/unit_test.hpp"
#include "pipeline_select.hpp"
//#include "bench_common.hpp"
#include "array_fixtures.hpp"
#include "sqeazy_header.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

BOOST_FIXTURE_TEST_SUITE( compress_select , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( instantiated )
{
  sqeazy::pipeline_select<> decide;
  BOOST_CHECK_EQUAL(decide.empty(),true);  
}

BOOST_AUTO_TEST_CASE( max_compressed_size )
{
  sqeazy::pipeline_select<> decide(std::make_pair(8,sqeazy::char_rmbkg_bswap1_lz4_pipe::name()));
  
  BOOST_CHECK_NE(decide.max_compressed_size(42,0),0);  
  BOOST_CHECK_GT(decide.max_compressed_size(42,0),42);  

}

BOOST_AUTO_TEST_CASE( max_compressed_size_throws_on_unknown )
{
  sqeazy::pipeline_select<> decide;
  
  BOOST_CHECK_THROW(decide.max_compressed_size(42,0),std::runtime_error);  

}

BOOST_AUTO_TEST_CASE( change_current )
{
  sqeazy::pipeline_select<> decide(std::make_pair(8,sqeazy::char_rmbkg_bswap1_lz4_pipe::name()));
  unsigned result_8bit = decide.max_compressed_size(42);

  decide.set(std::make_pair(16,sqeazy::char_rmbkg_bswap1_lz4_pipe::name()));
  unsigned result_16bit = decide.max_compressed_size(42);


  BOOST_CHECK_EQUAL(result_16bit, result_8bit);  


}



BOOST_AUTO_TEST_CASE( compress_callable )
{
  sqeazy::pipeline_select<> decide(std::make_pair(16,sqeazy::bswap1_lz4_pipe::name()));
  unsigned long num_encoded = 0;
  int ret = decide.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded);
  
  BOOST_CHECK_NE(incrementing_cube[0], to_play_with[0]);  
  BOOST_CHECK_GT(num_encoded, 0);  
  BOOST_CHECK_EQUAL(ret,0);

}

BOOST_AUTO_TEST_CASE( compress_correct_given_native_pipeline )
{
  sqeazy::pipeline_select<> decide(std::make_pair(16,sqeazy::bswap1_lz4_pipe::name()));
  unsigned long num_encoded = 0;
  int ret = decide.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded);
  
  BOOST_CHECK_NE(incrementing_cube[0], to_play_with[0]);  
  BOOST_CHECK_GT(num_encoded, 0);  

  unsigned long num_encoded_native = 0;
  int ret_native = sqeazy::bswap1_lz4_pipe::compress(&incrementing_cube[0], (char*)&constant_cube[0], dims, num_encoded_native);
  
  BOOST_CHECK_EQUAL(constant_cube[0], to_play_with[0]);  
  BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],&constant_cube[0] + num_encoded*2, &to_play_with[0],&to_play_with[0] + num_encoded*2);  
  BOOST_CHECK_EQUAL(num_encoded, num_encoded_native);  
  BOOST_CHECK_EQUAL(ret, ret_native);  
}

BOOST_AUTO_TEST_CASE( compress_throws )
{
  sqeazy::pipeline_select<> decide(std::make_pair(16,"anything"));
  unsigned long num_encoded = 0;
  BOOST_CHECK_THROW(decide.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded),std::runtime_error);
  
  sqeazy::pipeline_select<> decide2(std::make_pair(24,sqeazy::bswap1_lz4_pipe::name()));
  BOOST_CHECK_THROW(decide2.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded),std::runtime_error);
  

}

BOOST_AUTO_TEST_CASE( compress_correct_header )
{
  sqeazy::pipeline_select<> decide(std::make_pair(16,sqeazy::bswap1_lz4_pipe::name()));
  unsigned long num_encoded = 0;
  int ret = decide.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded);
  
  BOOST_CHECK_EQUAL(ret, 0);  
  BOOST_CHECK_GT(num_encoded, 0);  
  const char* output = (char*)&to_play_with[0];
  sqeazy::image_header<value_type> local(output, output + num_encoded);

  BOOST_CHECK_EQUAL(sqeazy::bswap1_lz4_pipe::name(), local.pipeline());  
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_FIXTURE_TEST_SUITE( decompess_pipeline_select , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( decoded_size_byte )
{
  std::string hdr = sqeazy::image_header<value_type>::pack(dims, sqeazy::bswap1_lz4_pipe::name());
  sqeazy::pipeline_select<> decide(16, sqeazy::bswap1_lz4_pipe::name());
  
  unsigned long long size_ = size;
  unsigned long long expected_size_byte_all_known = decide.decoded_size_byte(&hdr[0],hdr.size());
  
  BOOST_CHECK_GT(expected_size_byte_all_known, size_); 

  decide.set(1, "empty");
  
  unsigned long long expected_size_byte_all_unknown = decide.decoded_size_byte(&hdr[0],hdr.size());
  
  BOOST_CHECK_GT(expected_size_byte_all_unknown, size_); 
  BOOST_CHECK_EQUAL(expected_size_byte_all_unknown, expected_size_byte_all_known); 
  
}

BOOST_AUTO_TEST_CASE( decoded_size_byte_throws )
{
  std::string hdr = sqeazy::image_header<value_type>::pack(dims, sqeazy::bswap1_lz4_pipe::name());
  sqeazy::pipeline_select<> decide(24, "");
  
  
  BOOST_CHECK_THROW(decide.decoded_size_byte(&hdr[0],hdr.size()),std::runtime_error); 
  
}

BOOST_AUTO_TEST_CASE( decoded_shape )
{
  std::string hdr = sqeazy::image_header<value_type>::pack(dims, sqeazy::bswap1_lz4_pipe::name());
  sqeazy::pipeline_select<> decide(16, sqeazy::bswap1_lz4_pipe::name());
  
  std::vector<unsigned> found_shape = decide.decode_dimensions(&hdr[0],hdr.size());

  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), found_shape.begin(), found_shape.end());  

  decide.set(0,"empty");
  
  found_shape = decide.decode_dimensions(&hdr[0],hdr.size());

  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), found_shape.begin(), found_shape.end());  

  //BY INTENTION: mismatching n_bits = 8 with raw_type of bswap1_lz4_pipe (16-bit)
  decide.set(8,sqeazy::bswap1_lz4_pipe::name());
  
  found_shape = decide.decode_dimensions(&hdr[0],hdr.size());

  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), found_shape.begin(), found_shape.end());  
}

BOOST_AUTO_TEST_CASE( decoded_shape_throws )
{
  std::string hdr = sqeazy::image_header<value_type>::pack(dims, sqeazy::bswap1_lz4_pipe::name());
  sqeazy::pipeline_select<> decide(16, " ");
  
  BOOST_CHECK_THROW(decide.decode_dimensions(&hdr[0],hdr.size()), std::runtime_error);


}

BOOST_AUTO_TEST_CASE( decompress_callable )
{

  sqeazy::pipeline_select<> decide(16, sqeazy::bswap1_lz4_pipe::name());


  char* output_to_decompress = reinterpret_cast<char*>(&to_play_with[0]);
  
  unsigned long long bytes_written = 0;
  sqeazy::bswap1_lz4_pipe::compress(&incrementing_cube[0], output_to_decompress, dims, bytes_written);

  char* output = reinterpret_cast<char*>(&constant_cube[0]);
  

  decide.decompress(output_to_decompress, output, bytes_written);

  BOOST_CHECK_EQUAL(incrementing_cube[0],constant_cube[0]);
  BOOST_CHECK_EQUAL_COLLECTIONS(&incrementing_cube[0],&incrementing_cube[0]+ size,&constant_cube[0],&constant_cube[0]+ size); 
}

BOOST_AUTO_TEST_CASE( decompress_throws )
{

  sqeazy::pipeline_select<> decide(7, sqeazy::bswap1_lz4_pipe::name());


  char* output_to_decompress = reinterpret_cast<char*>(&to_play_with[0]);
 
  char* output = reinterpret_cast<char*>(&constant_cube[0]);
  
  unsigned long long l_size = 128;

  BOOST_CHECK_THROW(decide.decompress(output_to_decompress, output, l_size),std::runtime_error);

  decide.set();
  BOOST_CHECK_THROW(decide.decompress(output_to_decompress, output, l_size),std::runtime_error);

}


BOOST_AUTO_TEST_SUITE_END()
