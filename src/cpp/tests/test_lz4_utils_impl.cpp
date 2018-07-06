#define BOOST_TEST_MODULE TEST_LZ4_SCHEME_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"
#include "encoders/lz4_utils.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::array_fixture<unsigned char> uint8_cube_of_8;

BOOST_FIXTURE_TEST_SUITE( isolated_util_encode16, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( one_step_serial_encode )
{

  LZ4F_preferences_t lz4_prefs;
  lz4_prefs.frameInfo = { sqeazy::lz4::closest_blocksize::of(64), //commonly L2 size on Intel platforms
                          LZ4F_blockLinked,
                          LZ4F_noContentChecksum,
                          LZ4F_frame,
                          0 /* content size unknown */,
                          0 /* no dictID */ ,
                          LZ4F_noBlockChecksum };
  lz4_prefs.compressionLevel = 9;
  lz4_prefs.autoFlush = 0;
  sqeazy::lz4::wrap<decltype(lz4_prefs)>::favorDecSpeed_initialisation(lz4_prefs,0);


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

  LZ4F_preferences_t lz4_prefs;
  lz4_prefs.frameInfo = { sqeazy::lz4::closest_blocksize::of(64), //commonly L2 size on Intel platforms
                          LZ4F_blockLinked,
                          LZ4F_noContentChecksum,
                          LZ4F_frame,
                          0 /* content size unknown */,
                          0 /* no dictID */ ,
                          LZ4F_noBlockChecksum };
  lz4_prefs.compressionLevel = 9;
  lz4_prefs.autoFlush = 0;
  sqeazy::lz4::wrap<decltype(lz4_prefs)>::favorDecSpeed_initialisation(lz4_prefs,0);

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

  LZ4F_preferences_t lz4_prefs;
  lz4_prefs.frameInfo = { sqeazy::lz4::closest_blocksize::of(64), //commonly L2 size on Intel platforms
                          LZ4F_blockLinked,
                          LZ4F_noContentChecksum,
                          LZ4F_frame,
                          0 /* content size unknown */,
                          0 /* no dictID */ ,
                          LZ4F_noBlockChecksum };
  lz4_prefs.compressionLevel = 9;
  lz4_prefs.autoFlush = 0;
  sqeazy::lz4::wrap<decltype(lz4_prefs)>::favorDecSpeed_initialisation(lz4_prefs,0);
  auto exp_bytes = LZ4F_compressBound(size_in_byte/2, &lz4_prefs);
  BOOST_REQUIRE_GT(2*exp_bytes,LZ4F_compressBound(size_in_byte, &lz4_prefs));

  std::vector<char> encoded(2*exp_bytes);

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

BOOST_AUTO_TEST_CASE( two_step_encode_parallel )
{

  LZ4F_preferences_t lz4_prefs;
  lz4_prefs.frameInfo = { sqeazy::lz4::closest_blocksize::of(64), //commonly L2 size on Intel platforms
                          LZ4F_blockLinked,
                          LZ4F_noContentChecksum,
                          LZ4F_frame,
                          0 /* content size unknown */,
                          0 /* no dictID */ ,
                          LZ4F_noBlockChecksum };
  lz4_prefs.compressionLevel = 9;
  lz4_prefs.autoFlush = 0;
  sqeazy::lz4::wrap<decltype(lz4_prefs)>::favorDecSpeed_initialisation(lz4_prefs,0);
  auto exp_bytes = LZ4F_compressBound(size_in_byte/4, &lz4_prefs);
  BOOST_REQUIRE_GT(4*exp_bytes,LZ4F_compressBound(size_in_byte, &lz4_prefs));

  std::vector<char> encoded(4*exp_bytes);

  const char* in =  reinterpret_cast<char*>(incrementing_cube.data());
  const char* in_end = in + size_in_byte;

  auto res = sqeazy::lz4::encode_parallel(in,in_end,
                                          encoded.data(),encoded.data()+encoded.size(),
                                          size_in_byte/4,
                                          lz4_prefs,
                                          2);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_REQUIRE_LT(res,encoded.data()+encoded.size());

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


BOOST_AUTO_TEST_SUITE( blanks )

BOOST_AUTO_TEST_CASE( two_blocks )
{

  std::vector<std::uint8_t> exp(12,1);
  std::fill(exp.begin()+8,exp.end(),2);

  std::vector<std::uint8_t> with_blanks(32,0);
  std::fill(with_blanks.begin(),with_blanks.begin()+8,1);
  std::fill(with_blanks.begin()+16,with_blanks.begin()+20,2);
  std::vector<std::size_t> n_bytes = {8,4};

  const std::size_t chunk_stride = 16;

  auto res = sqeazy::lz4::remove_blanks(with_blanks.data(),
                           n_bytes,
                           chunk_stride);

  BOOST_CHECK_EQUAL_COLLECTIONS(with_blanks.begin(),
                                with_blanks.begin()+std::distance(with_blanks.data(),res),
                                exp.begin(), exp.end());
}


BOOST_AUTO_TEST_CASE( four_blocks )
{

  std::vector<std::size_t> n_bytes(4,0);
  int cnt = 1;
  for( std::size_t & it : n_bytes )
    it = 1 << cnt;

  const std::size_t chunk_stride = 1 << (n_bytes.size()+1);

  BOOST_CHECK_LT(n_bytes.back(), chunk_stride);

  const std::size_t n_filled = std::accumulate(n_bytes.begin(), n_bytes.end(), 0);

  std::vector<std::uint8_t> exp(n_filled,0);
  std::vector<std::uint8_t> with_blanks(n_bytes.size()*chunk_stride,0);

  std::size_t written = 0;
  for(std::size_t i = 0;i<n_bytes.size();++i){
    std::fill(exp.begin()+written,exp.begin()+written+n_bytes[i],i);
    std::fill(with_blanks.begin()+i*chunk_stride,with_blanks.begin()+i*chunk_stride+written,i);
    written += n_bytes[i];
  }

  auto res = sqeazy::lz4::remove_blanks(with_blanks.data(),
                                        n_bytes,
                                        chunk_stride);

  auto size = std::distance(with_blanks.data(),res);
  BOOST_CHECK_EQUAL(size,exp.size());
  BOOST_CHECK_EQUAL_COLLECTIONS(with_blanks.data(),
                                res,
                                exp.begin(), exp.end());
}

BOOST_AUTO_TEST_SUITE_END()
