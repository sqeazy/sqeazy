#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_PIPELINE_SELECT
#include "boost/test/unit_test.hpp"
#include "pipeline_select.hpp"
#include "bench_common.hpp"
#include "array_fixtures.hpp"


typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

BOOST_FIXTURE_TEST_SUITE( measurements, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( instantiated )
{
  sqeazy_bench::compress_select decide;
  BOOST_CHECK_EQUAL(decide.empty(),true);  
}

BOOST_AUTO_TEST_CASE( max_compressed_size )
{
  sqeazy_bench::compress_select decide(std::make_pair(8,char_rmbkg_bswap1_lz4_pipe::name()));
  
  BOOST_CHECK_NE(decide.max_compressed_size(42),0);  
  BOOST_CHECK_GT(decide.max_compressed_size(42),42);  

}

BOOST_AUTO_TEST_CASE( max_compressed_size_throws_on_unknown )
{
  sqeazy_bench::compress_select decide;
  
  BOOST_CHECK_THROW(decide.max_compressed_size(42),std::runtime_error);  

}

BOOST_AUTO_TEST_CASE( change_current )
{
  sqeazy_bench::compress_select decide(std::make_pair(8,char_rmbkg_bswap1_lz4_pipe::name()));
  unsigned result_8bit = decide.max_compressed_size(42);

  decide.set(std::make_pair(16,char_rmbkg_bswap1_lz4_pipe::name()));
  unsigned result_16bit = decide.max_compressed_size(42);


  BOOST_CHECK_EQUAL(result_16bit, result_8bit);  


}



BOOST_AUTO_TEST_CASE( compress_callable )
{
  sqeazy_bench::compress_select decide(std::make_pair(16,bswap1_lz4_pipe::name()));
  unsigned long num_encoded = 0;
  int ret = decide.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded);
  
  BOOST_CHECK_NE(incrementing_cube[0], to_play_with[0]);  
  BOOST_CHECK_GT(num_encoded, 0);  
  BOOST_CHECK_EQUAL(ret,0);

}

BOOST_AUTO_TEST_CASE( compress_correct_given_native_pipeline )
{
  sqeazy_bench::compress_select decide(std::make_pair(16,bswap1_lz4_pipe::name()));
  unsigned long num_encoded = 0;
  int ret = decide.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded);
  
  BOOST_CHECK_NE(incrementing_cube[0], to_play_with[0]);  
  BOOST_CHECK_GT(num_encoded, 0);  

  unsigned long num_encoded_native = 0;
  int ret_native = bswap1_lz4_pipe::compress(&incrementing_cube[0], (char*)&constant_cube[0], dims, num_encoded_native);
  
  BOOST_CHECK_EQUAL(constant_cube[0], to_play_with[0]);  
  BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],&constant_cube[0] + num_encoded*2, &to_play_with[0],&to_play_with[0] + num_encoded*2);  
  BOOST_CHECK_EQUAL(num_encoded, num_encoded_native);  
  BOOST_CHECK_EQUAL(ret, ret_native);  
}

BOOST_AUTO_TEST_CASE( compress_throws )
{
  sqeazy_bench::compress_select decide(std::make_pair(16,"anything"));
  unsigned long num_encoded = 0;
  BOOST_CHECK_THROW(decide.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded),std::runtime_error);
  
  sqeazy_bench::compress_select decide2(std::make_pair(24,bswap1_lz4_pipe::name()));
  BOOST_CHECK_THROW(decide2.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded),std::runtime_error);
  

}

BOOST_AUTO_TEST_CASE( refactor_compress_callable )
{
  sqeazy_bench::compress_select decide(std::make_pair(16,bswap1_lz4_pipe::name()));
  unsigned long num_encoded = 0;
  
  const char* input = (const char*)&incrementing_cube[0];
  char* output = (char*)&to_play_with[0];

  int ret = decide.compress(input, output, dims, num_encoded);
  
  BOOST_CHECK_NE(incrementing_cube[0], to_play_with[0]);  
  BOOST_CHECK_GT(num_encoded, 0);  

  output = (char*)&constant_cube[0];
  int retv = decide.variant_compress(input, output, dims, num_encoded);
  
  BOOST_CHECK_EQUAL(ret,retv);
  BOOST_CHECK_EQUAL(constant_cube[0],to_play_with[0]);
  BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],&constant_cube[0] + num_encoded*2, &to_play_with[0],&to_play_with[0] + num_encoded*2);  
}

BOOST_AUTO_TEST_SUITE_END()




