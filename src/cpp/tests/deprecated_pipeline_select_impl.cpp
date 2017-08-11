#define BOOST_TEST_MODULE TEST_PIPELINE_SELECT
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include "deprecated/static_pipeline_select.hpp"
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

BOOST_AUTO_TEST_CASE( typesize_matches )
{
  sqeazy::pipeline_select<> decide;
  BOOST_CHECK_EQUAL(decide.typesize_matches(),false);

  decide.set(1,sqeazy::char_rmbkg_bswap1_lz4_pipe::static_name());
  BOOST_CHECK_EQUAL(decide.typesize_matches(),false);

  decide.set(8,sqeazy::char_rmbkg_bswap1_lz4_pipe::static_name());
  BOOST_CHECK_EQUAL(decide.typesize_matches(),true);


  decide.set(1,sqeazy::rmbkg_bswap1_lz4_pipe::static_name());
  BOOST_CHECK_EQUAL(decide.typesize_matches(),false);

  decide.set(16,sqeazy::rmbkg_bswap1_lz4_pipe::static_name());
  BOOST_CHECK_EQUAL(decide.typesize_matches(),true);

}

BOOST_AUTO_TEST_CASE( max_compressed_size )
{
  sqeazy::pipeline_select<> decide(std::make_pair(8,sqeazy::char_rmbkg_bswap1_lz4_pipe::static_name()));

  BOOST_CHECK_NE(decide.max_compressed_size(42,0),0u);
  BOOST_CHECK_GT(decide.max_compressed_size(42,0),42u);

}

BOOST_AUTO_TEST_CASE( max_compressed_size_throws_on_unknown )
{
  sqeazy::pipeline_select<> decide;

  BOOST_CHECK_THROW(decide.max_compressed_size(42,0),std::runtime_error);

}

BOOST_AUTO_TEST_CASE( change_current )
{
  sqeazy::pipeline_select<> decide(std::make_pair(8,sqeazy::char_rmbkg_bswap1_lz4_pipe::static_name()));
  unsigned result_8bit = decide.max_compressed_size(42);

  decide.set(std::make_pair(16,sqeazy::rmbkg_bswap1_lz4_pipe::static_name()));
  unsigned result_16bit = decide.max_compressed_size(42);


  BOOST_CHECK_EQUAL(result_16bit, result_8bit);


}



BOOST_AUTO_TEST_CASE( compress_callable )
{
  sqeazy::pipeline_select<> decide(std::make_pair(16,sqeazy::bswap1_lz4_pipe::static_name()));

  unsigned long num_encoded = 0;
  to_play_with.resize(decide.max_compressed_size(size_in_byte)/2);

  int ret = decide.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded);

  BOOST_CHECK_NE(incrementing_cube[0], to_play_with[0]);
  BOOST_CHECK_GT(num_encoded, 0u);
  BOOST_CHECK_EQUAL(ret,0);

}

BOOST_AUTO_TEST_CASE( compress_correct_given_native_pipeline )
{
  sqeazy::pipeline_select<> decide(std::make_pair(16,sqeazy::bswap1_lz4_pipe::static_name()));
  to_play_with.resize(decide.max_compressed_size(size_in_byte)/2);
  unsigned long num_encoded = 0;
  int ret = decide.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded);

  BOOST_CHECK_NE(incrementing_cube[0], to_play_with[0]);
  BOOST_CHECK_GT(num_encoded, 0);

  unsigned long num_encoded_native = 0;
  constant_cube.resize(decide.max_compressed_size(size_in_byte)/2);
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

  sqeazy::pipeline_select<> decide2(std::make_pair(24,sqeazy::bswap1_lz4_pipe::static_name()));
  BOOST_CHECK_THROW(decide2.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded),std::runtime_error);


}

BOOST_AUTO_TEST_CASE( compress_correct_header )
{
  sqeazy::pipeline_select<> decide(std::make_pair(16,sqeazy::bswap1_lz4_pipe::static_name()));
  unsigned long num_encoded = 0;
  to_play_with.resize(decide.max_compressed_size(size_in_byte)/2);
  int ret = decide.compress((const char*)&incrementing_cube[0], (char*)&to_play_with[0], dims, num_encoded);

  BOOST_CHECK_EQUAL(ret, 0);
  BOOST_CHECK_GT(num_encoded, 0);
  const char* output = (char*)&to_play_with[0];
  sqeazy::image_header local(output, output + num_encoded);

  BOOST_CHECK_EQUAL(sqeazy::bswap1_lz4_pipe::static_name(), local.pipeline());
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_FIXTURE_TEST_SUITE( decompess_pipeline_select , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( decoded_size_byte )
{
  std::string hdr = sqeazy::image_header::pack<value_type>(dims, sqeazy::bswap1_lz4_pipe::static_name());
  sqeazy::pipeline_select<> decide(16, sqeazy::bswap1_lz4_pipe::static_name());

  unsigned long long size_ = size;
  unsigned long long expected_size_byte_all_known = decide.decoded_size_byte(&hdr[0],hdr.size());

  BOOST_CHECK_GT(expected_size_byte_all_known, size_);

}

BOOST_AUTO_TEST_CASE( decoded_size_byte_throws )
{
  std::string hdr = sqeazy::image_header::pack<value_type>(dims, sqeazy::bswap1_lz4_pipe::static_name());
  sqeazy::pipeline_select<> decide(24, "");


  BOOST_CHECK_THROW(decide.decoded_size_byte(&hdr[0],hdr.size()),std::runtime_error);

  decide.set(16, "empty");

  BOOST_CHECK_THROW(decide.decoded_size_byte(&hdr[0],hdr.size()),std::runtime_error);

}

BOOST_AUTO_TEST_CASE( decoded_shape )
{
  std::string hdr = sqeazy::image_header::pack<value_type>(dims, sqeazy::bswap1_lz4_pipe::static_name());
  sqeazy::pipeline_select<> decide(16, sqeazy::bswap1_lz4_pipe::static_name());

  std::vector<unsigned long> found_shape = decide.decode_dimensions(&hdr[0],hdr.size());

  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), found_shape.begin(), found_shape.end());

}

BOOST_AUTO_TEST_CASE( decoded_shape_throws )
{
  std::string hdr = sqeazy::image_header::pack<value_type>(dims, sqeazy::bswap1_lz4_pipe::static_name());
  sqeazy::pipeline_select<> decide(16, " ");

  BOOST_CHECK_THROW(decide.decode_dimensions(&hdr[0],hdr.size()), std::runtime_error);

  decide.set(16,"empty");
  BOOST_CHECK_THROW(decide.decode_dimensions(&hdr[0],hdr.size()), std::runtime_error);

  decide.set(1,sqeazy::bswap1_lz4_pipe::static_name());
  BOOST_CHECK_THROW(decide.decode_dimensions(&hdr[0],hdr.size()), std::runtime_error);
}

BOOST_AUTO_TEST_CASE( decompress_callable )
{

  sqeazy::pipeline_select<> decide(16, sqeazy::bswap1_lz4_pipe::static_name());

  to_play_with.resize(decide.max_compressed_size(size_in_byte)/2);
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

  sqeazy::pipeline_select<> decide(7, sqeazy::bswap1_lz4_pipe::static_name());


  char* output_to_decompress = reinterpret_cast<char*>(&to_play_with[0]);

  char* output = reinterpret_cast<char*>(&constant_cube[0]);

  unsigned long long l_size = 128;

  BOOST_CHECK_THROW(decide.decompress(output_to_decompress, output, l_size),std::runtime_error);

  decide.set();
  BOOST_CHECK_THROW(decide.decompress(output_to_decompress, output, l_size),std::runtime_error);

}


BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( from_sqy_header , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( non_empty_from_valid_header )
{
  std::string hdr = sqeazy::image_header::pack<value_type>(dims, sqeazy::bswap1_lz4_pipe::static_name());
  sqeazy::pipeline_select<> decide(hdr);


  BOOST_CHECK_MESSAGE(!decide.empty(), "pipeline not set from header " << hdr);

}

BOOST_AUTO_TEST_CASE( header_intact )
{
  //compress first by pipeline method
  unsigned long pipeline_payload = 0;
  constant_cube.resize(incrementing_cube.size()*3);

  int pret = sqeazy::bswap1_lz4_pipe::compress(&incrementing_cube[0],
                    (char*)&constant_cube[0],
                    dims,
                    pipeline_payload
                    );
  BOOST_CHECK(pret == 0);
  BOOST_CHECK(pipeline_payload);

  sqeazy::image_header native_hdr((char*)&constant_cube[0],((char*)&constant_cube[0]) + pipeline_payload);

  std::string hdr = sqeazy::image_header::pack<value_type>(dims,
                               sqeazy::bswap1_lz4_pipe::static_name(),
                               native_hdr.compressed_size_byte()
                               );
  sqeazy::pipeline_select<> decide(hdr);
  to_play_with.resize(decide.max_compressed_size(size_in_byte)/2);

  char* output_ = reinterpret_cast<char*>(&to_play_with[0]);

  unsigned long long bytes_written = 0;
  sqeazy::bswap1_lz4_pipe::compress(&incrementing_cube[0], output_, dims, bytes_written);


  char* result = std::find(output_,output_+bytes_written,'|');
  std::string new_hdr(output_,result+1);

  BOOST_CHECK_EQUAL(hdr, new_hdr);
}

BOOST_AUTO_TEST_CASE( selected_comresses_like_native )
{
  std::string hdr = sqeazy::image_header::pack<value_type>(dims, sqeazy::bswap1_lz4_pipe::static_name());
  sqeazy::pipeline_select<> decide(hdr);
  to_play_with.resize(decide.max_compressed_size(size_in_byte)/2);

  char* output_ = reinterpret_cast<char*>(&to_play_with[0]);

  unsigned long long bytes_written = 0;
  sqeazy::bswap1_lz4_pipe::compress(&incrementing_cube[0], output_, dims, bytes_written);


  char* result = std::find(output_,output_+bytes_written,'|');
  std::string new_hdr(output_,result+1);

  unsigned long long native_bytes_written = 0;
  constant_cube.resize(decide.max_compressed_size(size_in_byte)/2);

  sqeazy::bswap1_lz4_pipe::compress(&incrementing_cube[0], (char*)&constant_cube[0], dims, native_bytes_written);

  BOOST_CHECK_EQUAL(native_bytes_written,bytes_written);
}
BOOST_AUTO_TEST_SUITE_END()
