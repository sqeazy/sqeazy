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

BOOST_AUTO_TEST_SUITE( block_framesize_logic )

BOOST_AUTO_TEST_CASE( defaults )
{
  sqeazy::lz4_scheme<std::uint16_t> local;

  BOOST_REQUIRE_EQUAL(local.blocksize_kb,256);
  BOOST_REQUIRE_EQUAL(local.framestep_kb,256);

}

BOOST_AUTO_TEST_CASE( framestep_of_n64kb )
{
  sqeazy::lz4_scheme<std::uint16_t> local("framestep_kb=64");

  BOOST_REQUIRE_EQUAL(local.framestep_kb,256);

  sqeazy::lz4_scheme<std::uint16_t> local2("framestep_kb=64,blocksize_kb=64");

  BOOST_REQUIRE_EQUAL(local2.framestep_kb,64);

  sqeazy::lz4_scheme<std::uint16_t> local3("blocksize_kb=64,framestep_kb=128");

  BOOST_REQUIRE_EQUAL(local3.framestep_kb,128);

}

BOOST_AUTO_TEST_CASE( framestep_of_512kb )
{

  sqeazy::lz4_scheme<std::uint16_t> offby1("framestep_kb=513");

  BOOST_REQUIRE_EQUAL(offby1.framestep_kb,512);

  sqeazy::lz4_scheme<std::uint16_t> minus1("framestep_kb=511");

  BOOST_REQUIRE_EQUAL(minus1.framestep_kb,512);


  sqeazy::lz4_scheme<std::uint16_t> local2("framestep_kb=512");

  BOOST_REQUIRE_EQUAL(local2.framestep_kb,512);


}


BOOST_AUTO_TEST_SUITE_END()


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
  auto res = local.max_encoded_size(1 << 20);
  BOOST_REQUIRE_GT(res,0);
  BOOST_REQUIRE_LE(res,ref);

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
  auto fs256kb = no_chunks.max_encoded_size(1 << 20);

  sqeazy::lz4_scheme<std::uint16_t> local("framestep_kb=4096");
  auto fs4M = local.max_encoded_size(1 << 20);
  BOOST_REQUIRE_GT(fs4M,0);
  BOOST_REQUIRE_LE(fs4M,fs256kb);//if framstep_kb is larger than payload, we only have the overhead once

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
  auto fs256kb = no_chunks.max_encoded_size(1 << 4);

  sqeazy::lz4_scheme<std::uint16_t> local("n_chunks_of_input=4");
  auto nc4 = local.max_encoded_size(1 << 4);
  BOOST_REQUIRE_GT(nc4,0);
  BOOST_REQUIRE_GE(nc4,fs256kb);//if framstep_kb is larger than payload, we only have the overhead once

}

BOOST_AUTO_TEST_CASE( prefs_contentSize )
{

  sqeazy::lz4_scheme<std::uint16_t> local("n_chunks_of_input=64");
  local.lz4_prefs.frameInfo.contentSize = 1 << 20;
  auto res = local.max_encoded_size(1 << 20);

  local.lz4_prefs.frameInfo.contentSize = 0;
  auto res0 = local.max_encoded_size(1 << 20);
  BOOST_REQUIRE_EQUAL(res,res0);


}

BOOST_AUTO_TEST_CASE( max_encoded_size_404MB )
{

  const auto input_bytes = 423624704; //taken from a test sample
  sqeazy::lz4_scheme<std::uint16_t> alldefaults;
  auto alldefaults_size = alldefaults.max_encoded_size(input_bytes);
  BOOST_REQUIRE_GT(alldefaults_size,input_bytes);
  BOOST_REQUIRE_LT(alldefaults_size,1.5*input_bytes);

  sqeazy::lz4_scheme<std::uint16_t> fs4MB("framestep_kb=4048");
  auto fs4MB_size = fs4MB.max_encoded_size(input_bytes);
  BOOST_REQUIRE_GT(fs4MB_size,0);
  BOOST_REQUIRE_LT(fs4MB_size,1.1*input_bytes);//if framstep_kb is larger than payload, we only have the overhead once

  sqeazy::lz4_scheme<std::uint16_t> fs64kB_bs4MB("framestep_kb=64,blocksize_kb=4048");
  auto fs64kB_bs4MB_size = fs64kB_bs4MB.max_encoded_size(input_bytes);
  BOOST_REQUIRE_GT(fs64kB_bs4MB_size,0);
  BOOST_REQUIRE_LT(fs64kB_bs4MB_size,1.1*input_bytes);
  BOOST_REQUIRE_NE(fs64kB_bs4MB_size,fs4MB_size);

  sqeazy::lz4_scheme<std::uint16_t> nchunk4("n_chunks_of_input=4");
  auto nchunk4_size = nchunk4.max_encoded_size(input_bytes);
  BOOST_REQUIRE_GT(nchunk4_size,0);
  BOOST_REQUIRE_LT(nchunk4_size,1.1*input_bytes);
  //BOOST_REQUIRE_NE(nchunk4_size,fs4MB_size);

}

BOOST_AUTO_TEST_CASE( serial_vs_parallel_404MB )
{
  const auto nbytes = 423624704;
  sqeazy::lz4_scheme<std::uint16_t> alldefaults_serial;
  auto serial = alldefaults_serial.max_encoded_size(nbytes);

  sqeazy::lz4_scheme<std::uint16_t> alldefaults_parallel;
  alldefaults_parallel.set_n_threads(4);
  auto parallel = alldefaults_parallel.max_encoded_size(nbytes);

  BOOST_REQUIRE_GE(parallel,serial);

}

BOOST_AUTO_TEST_CASE( serial_vs_parallel_3MB )
{
  const auto nbytes = 3 << 20;
  sqeazy::lz4_scheme<std::uint16_t> alldefaults_serial;
  auto serial = alldefaults_serial.max_encoded_size(nbytes);

  sqeazy::lz4_scheme<std::uint16_t> alldefaults_parallel;
  alldefaults_parallel.set_n_threads(4);
  auto parallel = alldefaults_parallel.max_encoded_size(nbytes);

  BOOST_REQUIRE_GE(parallel,serial);

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
  BOOST_CHECK_GT(res,encoded.data());
  BOOST_CHECK_LE(res,encoded.data()+encoded.size());
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
    BOOST_REQUIRE_GT(res,encoded.data());
    BOOST_REQUIRE_LE(res,encoded.data()+encoded.size());

    std::size_t encoded_size = std::distance(encoded.data(),res);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded_size,
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
    std::size_t encoded_size = std::distance(encoded.data(),res);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded_size,
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

    auto encoded_size = std::distance(encoded.data(),res);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded_size,
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
                              std::distance(encoded.data(),res),
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

BOOST_AUTO_TEST_CASE( encoded_size_similar )
{

  const auto len = 4 << 19;// want a 4MB test image
  incrementing_cube.resize(len);

  std::vector<std::size_t> shape(dims.begin(), dims.end());
  shape.back() = (len) / frame;

  sqeazy::lz4_scheme<value_type> parallel;
  parallel.set_n_threads(2);
  auto expected_encoded_bytes = parallel.max_encoded_size(len*sizeof(value_type));
  std::vector<char> encoded(expected_encoded_bytes);

  auto parallelo = parallel.encode(incrementing_cube.data(),
                                   encoded.data(),
                                   shape);

  BOOST_REQUIRE_NE(parallelo,(char*)nullptr);
  BOOST_REQUIRE_NE(parallelo,encoded.data());
  auto encode_size_parallel = std::distance(encoded.data(),parallelo);
  BOOST_REQUIRE_GT(encode_size_parallel,0);
  BOOST_CHECK_LT(encode_size_parallel,expected_encoded_bytes);
  BOOST_CHECK_GT(encode_size_parallel,LZ4F_HEADER_SIZE_MAX);

  sqeazy::lz4_scheme<value_type> serial;
  expected_encoded_bytes = serial.max_encoded_size(len*sizeof(value_type));
  std::vector<char> serial_encoded(expected_encoded_bytes);

  auto serialo = serial.encode(&incrementing_cube[0],
                               serial_encoded.data(),
                               shape);


  BOOST_REQUIRE_NE(serialo,(char*)nullptr);
  BOOST_CHECK_NE(serialo,encoded.data());
  auto encode_size_serial = std::distance(serial_encoded.data(),serialo);
  BOOST_CHECK_LE(encode_size_serial,expected_encoded_bytes);//!
  BOOST_CHECK_GT(encode_size_serial,LZ4F_HEADER_SIZE_MAX);
}

BOOST_AUTO_TEST_CASE( default_args_roundtrip )
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

  auto encoded_size = std::distance(encoded.data(),res);

  auto rcode = local.decode(encoded.data(),
                            to_play_with.data(),
                            encoded_size,
                            size_in_byte);

  BOOST_REQUIRE_NE(rcode,1);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
    incrementing_cube.begin(),
    incrementing_cube.end(),
    to_play_with.begin(),
    to_play_with.end()
    );

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


BOOST_AUTO_TEST_CASE( two_chunks_roundtrip )
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


  auto encoded_size = std::distance(encoded.data(),res);

  auto rcode = local.decode(encoded.data(),
                            to_play_with.data(),
                            encoded_size,
                            size_in_byte);

  BOOST_REQUIRE_NE(rcode,1);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
    incrementing_cube.begin(),
    incrementing_cube.end(),
    to_play_with.begin(),
    to_play_with.end()
    );


}

BOOST_AUTO_TEST_CASE( four_chunks_roundtrip )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local("n_chunks_of_input=4");
  local.set_n_threads(2);
  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_CHECK_NE(res,encoded.data());
  BOOST_CHECK_LT(std::distance(encoded.data(),res),expected_encoded_bytes);


  auto encoded_size = std::distance(encoded.data(),res);

  auto rcode = local.decode(encoded.data(),
                            to_play_with.data(),
                            encoded_size,
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
