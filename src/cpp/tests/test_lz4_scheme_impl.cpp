#define BOOST_TEST_MODULE TEST_LZ4_SCHEME_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"
#include "encoders/lz4.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::array_fixture<unsigned char> uint8_cube_of_8;

BOOST_AUTO_TEST_SUITE( max_encoded_size )

BOOST_AUTO_TEST_CASE( encodes_1M )
{

  sqeazy::lz4_scheme<std::uint16_t> local;
  auto res = local.max_encoded_size(1 << 20);
  BOOST_REQUIRE_GT(res,0);

}

BOOST_AUTO_TEST_CASE( encodes_1M_1chunk )
{
  sqeazy::lz4_scheme<std::uint16_t> no_chunks;
  auto ref = no_chunks.max_encoded_size(1 << 20);

  sqeazy::lz4_scheme<std::uint16_t> local("n_chunks_of_input=1");
  auto res_chunked = local.max_encoded_size(1 << 20);
  BOOST_REQUIRE_GT(res_chunked,0);
  BOOST_REQUIRE_LT(res_chunked,ref);

}

BOOST_AUTO_TEST_CASE( encodes_1M_0chunk )
{
  sqeazy::lz4_scheme<std::uint16_t> no_chunks;
  auto ref = no_chunks.max_encoded_size(1 << 20);

  sqeazy::lz4_scheme<std::uint16_t> local("n_chunks_of_input=0");
  auto res_chunked = local.max_encoded_size(1 << 20);
  BOOST_REQUIRE_GT(res_chunked,0);
  BOOST_REQUIRE_EQUAL(res_chunked,ref);

}


BOOST_AUTO_TEST_CASE( encodes_1M_20chunks )
{
  sqeazy::lz4_scheme<std::uint16_t> no_chunks;
  auto ref = no_chunks.max_encoded_size(1 << 20);

  sqeazy::lz4_scheme<std::uint16_t> local("n_chunks_of_input=64");
  auto res64 = local.max_encoded_size(1 << 20);
  BOOST_REQUIRE_GT(res64,0);
  BOOST_REQUIRE_GE(res64,ref);

}

BOOST_AUTO_TEST_CASE( encodes_1M_framestep_4M )
{
  sqeazy::lz4_scheme<std::uint16_t> no_chunks;
  auto fs16kb = no_chunks.max_encoded_size(1 << 20);

  sqeazy::lz4_scheme<std::uint16_t> local("framestep_kb=4096");
  auto fs4M = local.max_encoded_size(1 << 20);
  BOOST_REQUIRE_GT(fs4M,0);
  BOOST_REQUIRE_LT(fs4M,fs16kb);//if framstep_kb is larger than payload, we only have the overhead once

}

BOOST_AUTO_TEST_CASE( encodes_tiny )
{
  sqeazy::lz4_scheme<std::uint16_t> no_chunks;
  auto fs16kb = no_chunks.max_encoded_size(1 << 4);

  sqeazy::lz4_scheme<std::uint16_t> local("framestep_kb=4096");
  auto fs4M = local.max_encoded_size(1 << 4);
  BOOST_REQUIRE_GT(fs4M,0);
  BOOST_REQUIRE_EQUAL(fs4M,fs16kb);//if framstep_kb is larger than payload, we only have the overhead once

}

BOOST_AUTO_TEST_CASE( encodes_tiny_4chunks )
{
  sqeazy::lz4_scheme<std::uint16_t> no_chunks;
  auto fs16kb = no_chunks.max_encoded_size(1 << 4);

  sqeazy::lz4_scheme<std::uint16_t> local("n_chunks_of_input=4");
  auto nc4 = local.max_encoded_size(1 << 4);
  BOOST_REQUIRE_GT(nc4,0);
  BOOST_REQUIRE_GT(nc4,fs16kb);//if framstep_kb is larger than payload, we only have the overhead once

}

BOOST_AUTO_TEST_CASE( prefs_contentSize )
{

  sqeazy::lz4_scheme<std::uint16_t> local("n_chunks_of_input=64");
  local.lz4_prefs.frameInfo.contentSize = 1 << 20;
  auto res = local.max_encoded_size(1 << 20);

  local.lz4_prefs.frameInfo.contentSize = 0;
  auto res0 = local.max_encoded_size(1 << 20);
  BOOST_REQUIRE_EQUAL(res,res0);

  local.lz4_prefs.frameInfo.blockSizeID = LZ4F_max64KB;
  auto res64 = local.max_encoded_size(1 << 20);
  BOOST_REQUIRE_LT(res64,res0);


}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( isolated_util_encode16, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( one_step_serial_encode )
{

  LZ4F_preferences_t lz4_prefs = {
    { sqeazy::lz4::closest_blocksize::of(64), //commonly L2 size on Intel platforms
          LZ4F_blockLinked,
          LZ4F_noContentChecksum,
          LZ4F_frame,
          0 /* content size unknown */,
          0 /* no dictID */ ,
          LZ4F_noBlockChecksum },
        9,   /* compression level */
        0,   /* autoflush */
        { 0, 0, 0, 0 },  /* reserved, must be set to 0 */
      };

  auto exp_bytes = LZ4F_compressBound(size_in_byte, &lz4_prefs);
  std::vector<char> encoded(exp_bytes);

  const char* in =  reinterpret_cast<char*>(incrementing_cube.data());
  const char* in_end = in + size_in_byte;

  auto res = sqeazy::lz4::encode_serial(in,in_end,
                                        encoded.data(),encoded.data()+encoded.size(),
                                        size_in_byte,
                                         lz4_prefs);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_REQUIRE_LT(res,encoded.data()+encoded.size());

}

BOOST_AUTO_TEST_CASE( two_step_serial_encode )
{

  LZ4F_preferences_t lz4_prefs = {
    { sqeazy::lz4::closest_blocksize::of(64), //commonly L2 size on Intel platforms
      LZ4F_blockLinked,
      LZ4F_noContentChecksum,
      LZ4F_frame,
      0 /* content size unknown */,
      0 /* no dictID */ ,
      LZ4F_noBlockChecksum },
    9,   /* compression level */
    0,   /* autoflush */
    { 0, 0, 0, 0 },  /* reserved, must be set to 0 */
  };

  auto exp_bytes = LZ4F_compressBound(size_in_byte, &lz4_prefs);
  std::vector<char> encoded(exp_bytes);

  const char* in =  reinterpret_cast<char*>(incrementing_cube.data());
  const char* in_end = in + size_in_byte;

  auto res = sqeazy::lz4::encode_serial(in,in_end,
                                        encoded.data(),encoded.data()+encoded.size(),
                                        size_in_byte/2,
                                        lz4_prefs);

  BOOST_REQUIRE_NE(res,(char*)nullptr);

  BOOST_REQUIRE_LT(res,encoded.data()+encoded.size());
}


BOOST_AUTO_TEST_CASE( one_step_encode_parallel )
{

  LZ4F_preferences_t lz4_prefs = {
    { sqeazy::lz4::closest_blocksize::of(64), //commonly L2 size on Intel platforms
          LZ4F_blockLinked,
          LZ4F_noContentChecksum,
          LZ4F_frame,
          0 /* content size unknown */,
          0 /* no dictID */ ,
          LZ4F_noBlockChecksum },
        9,   /* compression level */
        0,   /* autoflush */
        { 0, 0, 0, 0 },  /* reserved, must be set to 0 */
      };

  auto exp_bytes = LZ4F_compressBound(size_in_byte, &lz4_prefs);
  std::vector<char> encoded(exp_bytes);

  const char* in =  reinterpret_cast<char*>(incrementing_cube.data());
  const char* in_end = in + size_in_byte;

  auto res = sqeazy::lz4::encode_parallel(in,in_end,
                                          encoded.data(),encoded.data()+encoded.size(),
                                          size_in_byte/2,
                                          lz4_prefs,
                                          2);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_REQUIRE_LT(res,encoded.data()+encoded.size());

}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( sixteen_bit, uint16_cube_of_8 )


BOOST_AUTO_TEST_CASE( encodes )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local;
  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_CHECK_NE(res,encoded.data());
  BOOST_CHECK_LT(std::distance(encoded.data(),res),expected_encoded_bytes);

}


BOOST_AUTO_TEST_CASE( decodes )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local;
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);

}


BOOST_AUTO_TEST_CASE( roundtrip )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local;
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);
    BOOST_REQUIRE_EQUAL_COLLECTIONS(
        incrementing_cube.begin(),
        incrementing_cube.end(),
        to_play_with.begin(),
        to_play_with.end()
        );
}


BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( eight_bit, uint8_cube_of_8 )

BOOST_AUTO_TEST_CASE( encodes )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local;
  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_CHECK_NE(res,encoded.data());

}

BOOST_AUTO_TEST_CASE( decodes )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local;
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);

}


BOOST_AUTO_TEST_CASE( roundtrip )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local;
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);
    BOOST_REQUIRE_EQUAL_COLLECTIONS(
        incrementing_cube.begin(),
        incrementing_cube.end(),
        to_play_with.begin(),
        to_play_with.end()
        );
}


BOOST_AUTO_TEST_CASE( roundtrip_blocksize4M )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local("blocksize_kb=4096");
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);
    BOOST_REQUIRE_EQUAL_COLLECTIONS(
        incrementing_cube.begin(),
        incrementing_cube.end(),
        to_play_with.begin(),
        to_play_with.end()
        );
}


BOOST_AUTO_TEST_CASE( roundtrip_nchunks_4 )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local("n_chunks_of_input=4");
  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);

  auto rcode = local.decode(encoded.data(),
                            to_play_with.data(),
                            encoded.size(),
                            size_in_byte);

  BOOST_REQUIRE_NE(rcode,1);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
    incrementing_cube.begin(),
    incrementing_cube.end(),
    to_play_with.begin(),
    to_play_with.end()
    );
}

BOOST_AUTO_TEST_CASE( roundtrip_nchunks_17 )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local("n_chunks_of_input=17");
  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);

  auto rcode = local.decode(encoded.data(),
                            to_play_with.data(),
                            encoded.size(),
                            size_in_byte);

  BOOST_REQUIRE_NE(rcode,1);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
    incrementing_cube.begin(),
    incrementing_cube.end(),
    to_play_with.begin(),
    to_play_with.end()
    );
}

BOOST_AUTO_TEST_CASE( roundtrip_nchunks_17_enforced )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local("framestep_kb=1,n_chunks_of_input=17");

  BOOST_REQUIRE_NE(local.framestep_kb,1);
  BOOST_REQUIRE_EQUAL(local.framestep_kb,0);

  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);

  auto rcode = local.decode(encoded.data(),
                            to_play_with.data(),
                            encoded.size(),
                            size_in_byte);

  BOOST_REQUIRE_NE(rcode,1);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
    incrementing_cube.begin(),
    incrementing_cube.end(),
    to_play_with.begin(),
    to_play_with.end()
    );
}

BOOST_AUTO_TEST_CASE( roundtrip_too_many_chunks )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local("n_chunks_of_input=1024");

  BOOST_REQUIRE_NE(local.framestep_kb,1);
  BOOST_REQUIRE_EQUAL(local.framestep_kb,0);

  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);

  auto rcode = local.decode(encoded.data(),
                            to_play_with.data(),
                            encoded.size(),
                            size_in_byte);

  BOOST_REQUIRE_NE(rcode,1);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
    incrementing_cube.begin(),
    incrementing_cube.end(),
    to_play_with.begin(),
    to_play_with.end()
    );
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( blocksizes, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( less_than )
{
  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(32),sqeazy::lz4::closest_blocksize::vals[0] );
  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(0),sqeazy::lz4::closest_blocksize::vals[0] );
}

BOOST_AUTO_TEST_CASE( greater_than )
{
  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(16 << 10),sqeazy::lz4::closest_blocksize::vals.back() );
  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(~0),sqeazy::lz4::closest_blocksize::vals.back() );
}

BOOST_AUTO_TEST_CASE( equal )
{

  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(64),sqeazy::lz4::closest_blocksize::vals[0] );
  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(256),sqeazy::lz4::closest_blocksize::vals[1] );
  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(1024),sqeazy::lz4::closest_blocksize::vals[2] );
  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(4096),sqeazy::lz4::closest_blocksize::vals[3] );

}

BOOST_AUTO_TEST_CASE( inbetween )
{

  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(65),sqeazy::lz4::closest_blocksize::vals[0] );
  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(255),sqeazy::lz4::closest_blocksize::vals[1] );
  BOOST_CHECK_EQUAL( sqeazy::lz4::closest_blocksize::of(257),sqeazy::lz4::closest_blocksize::vals[1] );

}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( sixteen_bit_in_parallel, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( default_args )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local;
  local.set_n_threads(2);
  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_CHECK_NE(res,encoded.data());
  BOOST_CHECK_LT(std::distance(encoded.data(),res),expected_encoded_bytes);

}

BOOST_AUTO_TEST_CASE( two_chunks )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local("n_chunks_of_input=2");
  local.set_n_threads(2);
  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_CHECK_NE(res,encoded.data());
  BOOST_CHECK_LT(std::distance(encoded.data(),res),expected_encoded_bytes);

}
BOOST_AUTO_TEST_SUITE_END()
