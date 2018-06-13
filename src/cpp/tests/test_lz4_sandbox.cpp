#define BOOST_TEST_MODULE TEST_LZ4_SANDBOX
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <cstring>
#include "array_fixtures.hpp"
#include "encoders/lz4.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::array_fixture<unsigned char> uint8_cube_of_8;

namespace lz4sandbox {

  std::intmax_t max_encoded_size (std::intmax_t _size_bytes){

    std::intmax_t lz4_bound_per_chunk = LZ4F_compressBound(_size_bytes, nullptr)
      + 4
      + LZ4F_HEADER_SIZE_MAX ;

    return lz4_bound_per_chunk;
  }

  char* compress_as_frame(const char* input, const char* input_end, char* _out){

    char* value = nullptr;
    // LZ4F_preferences_t lz4_prefs = {
    //     { LZ4F_max256KB, //commonly L2 size on Intel platforms
    //       LZ4F_blockLinked,
    //       LZ4F_noContentChecksum,
    //       LZ4F_frame,
    //       0 /* content size unknown */,
    //       0 /* no dictID */ ,
    //       LZ4F_noBlockChecksum },
    //     5,   /* compression level */
    //     0,   /* autoflush */
    //     { 0, 0, 0, 0 },  /* reserved, must be set to 0 */
    //   };
    LZ4F_preferences_t lz4_prefs;
    std::memset(&lz4_prefs,0,sizeof(lz4_prefs));
    lz4_prefs.frameInfo = { LZ4F_max256KB, //commonly L2 size on Intel platforms
          LZ4F_blockLinked,
          LZ4F_noContentChecksum,
          LZ4F_frame,
          0 /* content size unknown */,
          0 /* no dictID */ ,
          LZ4F_noBlockChecksum } ;
    lz4_prefs.compressionLevel = 5;

    const auto n_input_bytes = std::distance(input,input_end);

    lz4_prefs.frameInfo.contentSize = n_input_bytes;
    LZ4F_compressionContext_t ctx;
    auto rcode = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
    if (LZ4F_isError(rcode)) {
        std::cerr << "[lz4sandbox::lz4] Failed to create context: error " << rcode << "\n";
        return value;
      } else {
        rcode = 1;//don't ask me why, taken from https://github.com/lz4/lz4/blob/v1.8.0/examples/frameCompress.c#L34
      }

    const auto max_bytes_to_write = max_encoded_size(n_input_bytes) ;
    std::size_t num_written_bytes = 0;
    rcode = num_written_bytes = LZ4F_compressBegin(ctx,
                                                   _out,
                                                   max_bytes_to_write ,
                                                   &lz4_prefs);
    if (LZ4F_isError(rcode)) {
      std::cerr << "[sqy::lz4] Failed to start compression: error " << rcode << "\n";
      return value;
    }

    std::size_t compressed_bytes =LZ4F_compressUpdate(ctx,
                                             _out + num_written_bytes,
                                             max_bytes_to_write - num_written_bytes,
                                             input,
                                             n_input_bytes, NULL);

    if (LZ4F_isError(compressed_bytes)) {
      std::cerr << "[sqy::lz4] Failed to compress input: error " << compressed_bytes << "\n";
      return value;
    }

    num_written_bytes += compressed_bytes;
    value = _out + num_written_bytes;

    compressed_bytes = LZ4F_compressEnd(ctx, value, max_bytes_to_write - num_written_bytes, NULL);
    if (LZ4F_isError(compressed_bytes)) {
      std::cerr << "[sqy::lz4] Failed to finish compression: error " << compressed_bytes << "\n";
      return value;
    }
    num_written_bytes += compressed_bytes;
    LZ4F_freeCompressionContext(ctx);
    return _out + num_written_bytes;
  }

  char* decompress_as_frame(const char* encoded, const char* encoded_end,
                            char* decoded, char* decoded_end){

    char* value = nullptr;
    LZ4F_dctx* dctx = nullptr;
    auto rcode = LZ4F_createDecompressionContext(&dctx, 100);
    if (LZ4F_isError(rcode)) {
        std::cerr << "[lz4sandbox::lz4] Failed to create decode context: error " << rcode << "\n";
        return value;
      }

    const std::size_t encoded_size = std::distance(encoded,encoded_end);
    const std::size_t decoded_size = std::distance(decoded,decoded_end);

    std::size_t srcSize = encoded_size;
    std::size_t dstSize = decoded_size;

    LZ4F_frameInfo_t info;
    rcode = LZ4F_getFrameInfo(dctx, &info, encoded, &srcSize);
    if (LZ4F_isError(rcode)) {
      std::cerr << "[lz4sandbox::lz4] Failed to obtain frame info : error " << rcode << "\n";
      return value;
    }

    encoded += srcSize;
    srcSize = encoded_size - srcSize;

    if (info.contentSize > dstSize) {
      std::cerr << "[lz4sandbox::lz4] decompressed input yields " << info.contentSize
                << " Bytes, buffer is only "<< dstSize << " Bytes large\n";
      return value;
    }

    rcode = LZ4F_decompress(dctx, decoded, &dstSize,
                            encoded, &srcSize,
                            /* LZ4F_decompressOptions_t */ NULL);
    if (LZ4F_isError(rcode)) {
        std::cerr << "[lz4sandbox::lz4] Failed to decode : error " << rcode << "\n";
        return value;
    }
    LZ4F_freeDecompressionContext(dctx);
    return decoded+dstSize;

  }

  char* decompress_until_done(const char* encoded, const char* encoded_end,
                              char* decoded, char* decoded_end){

    char* value = nullptr;
    const std::size_t encoded_size = std::distance(encoded,encoded_end);
    const std::size_t decoded_size = std::distance(decoded,decoded_end);

    std::size_t srcSize = encoded_size;
    std::size_t dstSize = decoded_size;

    LZ4F_dctx* dctx = nullptr;
    std::size_t rcode = LZ4F_createDecompressionContext(&dctx, 100);
    if (LZ4F_isError(rcode)) {
        std::cerr << "[lz4sandbox::lz4] Failed to create decode context: error " << rcode << "\n";
        return value;
      }

    std::size_t num_written_bytes = 0;
    LZ4F_frameInfo_t info;
    const char* src = encoded;
    while(src < encoded_end){

      rcode = LZ4F_getFrameInfo(dctx, &info, src, &srcSize);
      if (LZ4F_isError(rcode)) {
        std::cerr << "[lz4sandbox::lz4] this data does comply to the lz4 frame format\n";
        num_written_bytes = 0;
        break;
      }

      src += srcSize;
      srcSize = std::distance(src,encoded_end);

      if (info.contentSize > (decoded_size - num_written_bytes)) {
        std::cerr << "[lz4sandbox::lz4] decompressed input yields " << info.contentSize
                  << " Bytes, only "<< dstSize << " Bytes left in destination\n";
        num_written_bytes = 0;
        break;
      }

      rcode = LZ4F_decompress(dctx, decoded, &dstSize,
                             src, &srcSize,
                            /* LZ4F_decompressOptions_t */ NULL);

      if (LZ4F_isError(rcode)) {
        num_written_bytes = 0;
        std::cerr << "[lz4sandbox::lz4] Failed to decode : error " << rcode << "\n";
        return value;
      }
      num_written_bytes += dstSize;
      decoded += dstSize;
      src += srcSize;

    }

    if(!num_written_bytes)
      return value;

    LZ4F_freeDecompressionContext(dctx);
    return decoded;

  }
};


BOOST_FIXTURE_TEST_SUITE( lz4sandbox, uint8_cube_of_8 )

BOOST_AUTO_TEST_CASE( encode_with_raw_lz4 )
{

  auto expected_encoded_bytes = lz4sandbox::max_encoded_size(incrementing_cube.size());
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = compress_as_frame((const char*)incrementing_cube.data(),
                               (const char*)incrementing_cube.data() + incrementing_cube.size(),
                               encoded.data()
    );


    BOOST_REQUIRE_NE(res,(char*)nullptr);
    BOOST_REQUIRE_LE(res,(char*)encoded.data()+encoded.size());
}

BOOST_AUTO_TEST_CASE( rt_with_raw_lz4 )
{

  auto expected_encoded_bytes = lz4sandbox::max_encoded_size(incrementing_cube.size());
  std::vector<char> encoded(expected_encoded_bytes);

  auto rt = incrementing_cube;
  std::fill(rt.data(),rt.data()+rt.size(),0);

  auto res = compress_as_frame((const char*)incrementing_cube.data(),
                               (const char*)incrementing_cube.data() + incrementing_cube.size(),
                               encoded.data()
    );

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_REQUIRE_LE(res,(char*)encoded.data()+encoded.size());

  auto dres = decompress_as_frame((const char*)encoded.data(),
                                  (const char*)res,
                                  (char*)rt.data(),
                                  (char*)rt.data() + rt.size()
    );


  BOOST_REQUIRE_NE(dres,(char*)nullptr);
  BOOST_REQUIRE_LE(dres,(char*)rt.data() + rt.size());

  BOOST_REQUIRE_EQUAL_COLLECTIONS(
        incrementing_cube.begin(),
        incrementing_cube.end(),
        rt.begin(),
        rt.end()
        );

}

BOOST_AUTO_TEST_CASE( two_in_one )
{


  auto expected_encoded_bytes = lz4sandbox::max_encoded_size(incrementing_cube.size());
  std::vector<char> encoded(2*expected_encoded_bytes);

  auto res = compress_as_frame((const char*)incrementing_cube.data(),
                               (const char*)incrementing_cube.data() + incrementing_cube.size(),
                               encoded.data()
    );
  res = compress_as_frame((const char*)incrementing_cube.data(),
                          (const char*)incrementing_cube.data() + incrementing_cube.size(),
                          res
    );

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_REQUIRE_LE(res,(char*)encoded.data()+encoded.size());

  auto rt = incrementing_cube;
  rt.resize(2*incrementing_cube.size());
  std::fill(rt.data(),rt.data()+rt.size(),0);

  auto dres = decompress_as_frame((const char*)encoded.data(),
                                  (const char*)res,
                                  (char*)rt.data(),
                                  (char*)rt.data() + rt.size()
    );


  BOOST_REQUIRE_NE(dres,(char*)nullptr);
  BOOST_REQUIRE_LE(dres,(char*)rt.data() + rt.size());
  BOOST_REQUIRE_LE(dres,(char*)rt.data() + rt.size()/2);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
        incrementing_cube.begin(),
        incrementing_cube.end(),
        rt.begin(),
        rt.begin() + rt.size()/2
        );

}

BOOST_AUTO_TEST_CASE( two_in_one_guess_next )
{


  auto expected_encoded_bytes = lz4sandbox::max_encoded_size(incrementing_cube.size());
  std::vector<char> encoded(2*expected_encoded_bytes);

  auto res = compress_as_frame((const char*)incrementing_cube.data(),
                               (const char*)incrementing_cube.data() + incrementing_cube.size(),
                               encoded.data()
    );
  res = compress_as_frame((const char*)incrementing_cube.data(),
                          (const char*)incrementing_cube.data() + incrementing_cube.size(),
                          res
    );

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_REQUIRE_LE(res,(char*)encoded.data()+encoded.size());

  auto rt = incrementing_cube;
  rt.resize(2*incrementing_cube.size());
  std::fill(rt.data(),rt.data()+rt.size(),0);

  auto dres = decompress_until_done((const char*)encoded.data(),
                                    (const char*)res,
                                    (char*)rt.data(),
                                    (char*)rt.data() + rt.size()
    );


  BOOST_REQUIRE_NE(dres,(char*)nullptr);
  BOOST_REQUIRE_GT(dres,(char*)rt.data() + rt.size()/2);


}

BOOST_AUTO_TEST_CASE( rt_two_in_one_guess_next )
{


  auto expected_encoded_bytes = lz4sandbox::max_encoded_size(incrementing_cube.size());
  std::vector<char> encoded(2*expected_encoded_bytes);

  auto res = compress_as_frame((const char*)incrementing_cube.data(),
                               (const char*)incrementing_cube.data() + incrementing_cube.size(),
                               encoded.data()
    );
  res = compress_as_frame((const char*)incrementing_cube.data(),
                          (const char*)incrementing_cube.data() + incrementing_cube.size(),
                          res
    );

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_REQUIRE_LE(res,(char*)encoded.data()+encoded.size());

  auto rt = incrementing_cube;
  rt.resize(2*incrementing_cube.size());
  std::fill(rt.data(),rt.data()+rt.size(),0);

  auto dres = decompress_until_done((const char*)encoded.data(),
                                    (const char*)res,
                                    (char*)rt.data(),
                                    (char*)rt.data() + rt.size()
    );


  BOOST_REQUIRE_NE(dres,(char*)nullptr);
  BOOST_REQUIRE_GT(dres,(char*)rt.data() + rt.size()/2);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(
        incrementing_cube.begin(),
        incrementing_cube.end(),
        rt.begin(),
        rt.begin() + rt.size()/2
        );
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
        incrementing_cube.begin(),
        incrementing_cube.end(),
        rt.begin() + rt.size()/2,
        rt.end()
        );
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( max_encoded_size )
BOOST_AUTO_TEST_CASE( null_prefs )
{


  const int nbytes = 64 << 10;
  auto res = LZ4F_compressBound( nbytes, nullptr);
  BOOST_CHECK_NE(res,0);
  BOOST_CHECK_GT(res,0);
  BOOST_CHECK_LT(res,2*nbytes);
  BOOST_CHECK_NE(res,nbytes);
  BOOST_TEST_MESSAGE(" lz4f_compressbound with no preferences " << res << " for 64 kB input");
  BOOST_CHECK_LE(res-nbytes,LZ4F_HEADER_SIZE_MAX);
}


BOOST_AUTO_TEST_CASE( null_prefs_5Bytes )
{


  const int nbytes = 5;
  auto res = LZ4F_compressBound( nbytes, nullptr);
  BOOST_CHECK_NE(res,0);
  BOOST_CHECK_GT(res,0);
  BOOST_CHECK_LT(res,1.05*(64 << 10));
  BOOST_TEST_MESSAGE(" lz4f_compressbound with no preferences " << res << " for 5 B input");

}

BOOST_AUTO_TEST_CASE( prefs_256kB )
{
  LZ4F_preferences_t kPrefs;
  std::memset(&kPrefs,0,sizeof(kPrefs));

  const int nbytes = 64 << 10;

  auto res = LZ4F_compressBound(nbytes , &kPrefs);
  BOOST_CHECK_NE(res,0);
  BOOST_CHECK_GT(res,0);
  BOOST_CHECK_GT(res,2*nbytes);//this fails as of https://groups.google.com/d/msg/lz4c/TwleHVKldtI/FxdHCdnsCgAJ
  BOOST_TEST_MESSAGE(" lz4f_compressbound with preferences (256kB block size) " << res << " for 64 kB input");
}
BOOST_AUTO_TEST_SUITE_END()
