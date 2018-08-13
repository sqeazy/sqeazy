#define BOOST_TEST_MODULE TEST_PIPELINE_INTERFACE
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
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

typedef sqeazy::array_fixture<std::uint16_t> uint16_cube_of_8;
typedef sqeazy::array_fixture<std::uint8_t> uint8_cube_of_8;

BOOST_AUTO_TEST_SUITE( pipeline )

BOOST_AUTO_TEST_CASE( does_this_validate_uint16 ){

  bool answer = SQY_Pipeline_Possible(default_filter_name.c_str(),2);
  BOOST_CHECK_EQUAL(answer, true);

  bool answerui16 = SQY_Pipeline_Possible_UI16(default_filter_name.c_str());
  BOOST_CHECK_EQUAL(answer, answerui16);
}

BOOST_AUTO_TEST_CASE( does_this_validate_uint8 ){

  bool answer = SQY_Pipeline_Possible(default_filter_name.c_str(),1);
  BOOST_CHECK_EQUAL(answer, true);
  bool answerui8 = SQY_Pipeline_Possible_UI8(default_filter_name.c_str());
  BOOST_CHECK_EQUAL(answer, answerui8);
}

BOOST_AUTO_TEST_CASE( this_doesnt_validate_uint16 ){

  bool answer = SQY_Pipeline_Possible("",2);
  BOOST_CHECK_EQUAL(answer, false);
  answer = SQY_Pipeline_Possible(deprecated_filter_name.c_str(),2);
  BOOST_CHECK_EQUAL(answer, false);

}

BOOST_AUTO_TEST_CASE( this_doesnt_validate_uint8 ){

  bool answer = SQY_Pipeline_Possible("",1);
  BOOST_CHECK_EQUAL(answer, false);
  answer = SQY_Pipeline_Possible(deprecated_filter_name.c_str(),1);
  BOOST_CHECK_EQUAL(answer, false);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( pipeline_interface, uint8_cube_of_8 )

BOOST_AUTO_TEST_CASE( max_compressed_bytes ){

  const long data_bytes = size_in_byte;
  long length = data_bytes;
  int rvalue = SQY_Pipeline_Max_Compressed_Length_UI8(default_filter_name.c_str(),
                                                      default_filter_name.size(),
                                                      &length);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_GT(length,data_bytes);

}

BOOST_AUTO_TEST_CASE( max_compressed_bytes_3D ){

  const long data_bytes = size_in_byte;
  long length = default_filter_name.size();
  std::vector<long> ldims(dims.begin(), dims.end());

  int rvalue = SQY_Pipeline_Max_Compressed_Length_3D_UI8(default_filter_name.c_str(),
                                                         &ldims[0],
                                                         dims.size(),
                                                         &length);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_GT(length,data_bytes);

}

BOOST_AUTO_TEST_CASE( decompressed_length ){

  const unsigned long data_bytes = size_in_byte;
  long length = default_filter_name.size();
  std::vector<long> ldims(dims.begin(), dims.end());
  SQY_Pipeline_Max_Compressed_Length_3D_UI8(default_filter_name.c_str(),
                         &ldims[0],
                         dims.size(),
                         &length);
  std::vector<char> compressed(length,0);
  int rvalue = SQY_PipelineEncode_UI8(default_filter_name.c_str(),
                                       (const char*)&constant_cube[0],
                                       &ldims[0],
                                       dims.size(),
                                       (char*)&compressed[0],
                                       &length,
                                       1);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());


  rvalue = SQY_Decompressed_Length(&compressed[0],
                        &length);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(std::size_t(length), data_bytes);

}

BOOST_AUTO_TEST_CASE( decompressed_ndims ){

  long length = default_filter_name.size();
  std::vector<long> ldims(dims.begin(), dims.end());
  SQY_Pipeline_Max_Compressed_Length_3D_UI8(default_filter_name.c_str(),
                         &ldims[0],
                         dims.size(),
                         &length);
  std::vector<char> compressed(length,0);
  int rvalue = SQY_PipelineEncode_UI8(default_filter_name.c_str(),
                                       (const char*)&constant_cube[0],
                                       &ldims[0],
                                       dims.size(),
                                       (char*)&compressed[0],
                                       &length,
                                       1);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());


  rvalue = SQY_Decompressed_NDims(&compressed[0],
                                  &length);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(std::size_t(length), 3);

}

BOOST_AUTO_TEST_CASE( decompressed_shape ){

  long length = default_filter_name.size();
  std::vector<long> ldims(dims.begin(), dims.end());
  SQY_Pipeline_Max_Compressed_Length_3D_UI8(default_filter_name.c_str(),
                         &ldims[0],
                         dims.size(),
                         &length);
  std::vector<char> compressed(length,0);
  int rvalue = SQY_PipelineEncode_UI8(default_filter_name.c_str(),
                                       (const char*)&constant_cube[0],
                                       &ldims[0],
                                       dims.size(),
                                       (char*)&compressed[0],
                                       &length,
                                       1);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());

  std::array<long,3> shape;shape.fill(length);
  rvalue = SQY_Decompressed_Shape(&compressed[0],
                                  shape.data());

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(shape.front(), ldims.front());
  BOOST_CHECK_EQUAL(shape.back(), ldims.back());


}

BOOST_AUTO_TEST_CASE( decompressed_sizeof ){

  long length = default_filter_name.size();
  std::vector<long> ldims(dims.begin(), dims.end());
  SQY_Pipeline_Max_Compressed_Length_3D_UI8(default_filter_name.c_str(),
                         &ldims[0],
                         dims.size(),
                         &length);
  std::vector<char> compressed(length,0);
  int rvalue = SQY_PipelineEncode_UI8(default_filter_name.c_str(),
                                       (const char*)&constant_cube[0],
                                       &ldims[0],
                                       dims.size(),
                                       (char*)&compressed[0],
                                       &length,
                                       1);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());

  rvalue = SQY_Decompressed_Sizeof(&compressed[0],&length);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(length, sizeof(constant_cube[0]));


}

BOOST_AUTO_TEST_CASE( roundtrip ){

  long length = default_filter_name.size();
  std::vector<long> ldims(dims.begin(), dims.end());
  SQY_Pipeline_Max_Compressed_Length_3D_UI8(default_filter_name.c_str(),
                         &ldims[0],
                         dims.size(),
                         &length);
  std::vector<char> compressed(length,0);
  int rvalue = SQY_PipelineEncode_UI8(default_filter_name.c_str(),
                       (const char*)&constant_cube[0],
                       &ldims[0],
                       dims.size(),
                       (char*)&compressed[0],
                                       &length,1);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());

  rvalue = SQY_Decode_UI8((const char*)&compressed[0],
                           length,
                           (char*)&incrementing_cube[0],
                           1
    );

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                incrementing_cube.begin(), incrementing_cube.end());

}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( hdf5_inference_queries, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( max_compressed_bytes ){

  const long data_bytes = size_in_byte;
  long length = data_bytes;
  int rvalue = SQY_Pipeline_Max_Compressed_Length_UI16(default_filter_name.c_str(),
                                                       default_filter_name.size(),
                                                       &length);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_GT(length,data_bytes);

}

BOOST_AUTO_TEST_CASE( max_compressed_bytes_3D ){

  const long data_bytes = size_in_byte;
  long length = default_filter_name.size();
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
  long length = default_filter_name.size();
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
                                       &length,
                                       1);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());


  rvalue = SQY_Decompressed_Length(&compressed[0],
                        &length);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(std::size_t(length), data_bytes);

}

BOOST_AUTO_TEST_CASE( decompressed_ndims ){

  long length = default_filter_name.size();
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
                                       &length,
                                       1);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());


  rvalue = SQY_Decompressed_NDims(&compressed[0],
                                  &length);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(std::size_t(length), 3);

}

BOOST_AUTO_TEST_CASE( decompressed_shape ){

  long length = default_filter_name.size();
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
                                       &length,
                                       1);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());

  std::array<long,3> shape;shape.fill(length);
  rvalue = SQY_Decompressed_Shape(&compressed[0],
                                  shape.data());

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(shape.front(), ldims.front());
  BOOST_CHECK_EQUAL(shape.back(), ldims.back());


}

BOOST_AUTO_TEST_CASE( decompressed_sizeof ){

  long length = default_filter_name.size();
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
                                       &length,
                                       1);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());

  rvalue = SQY_Decompressed_Sizeof(&compressed[0],&length);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(length, sizeof(constant_cube[0]));


}

BOOST_AUTO_TEST_CASE( roundtrip ){

  long length = default_filter_name.size();
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
                                       &length,1);
  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());

  rvalue = SQY_Decode_UI16((const char*)&compressed[0],
                           length,
                           (char*)&incrementing_cube[0],
                           1
    );

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                incrementing_cube.begin(), incrementing_cube.end());

}
BOOST_AUTO_TEST_SUITE_END()

static const std::string tricky_filter_name = "quantiser->h264";

#ifdef SQY_WITH_FFMPEG

BOOST_AUTO_TEST_SUITE( video_codecs )

BOOST_AUTO_TEST_CASE( roundtrip ){

  std::vector<size_t> shape(3,128);
  shape.front() *= 2;

  const size_t len = std::accumulate(shape.begin(),
                     shape.end(),
                     1.,
                     std::multiplies<size_t>());

  long length = tricky_filter_name.size();

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
                                       &length,1);
  BOOST_REQUIRE_EQUAL(rvalue, 0);
  BOOST_CHECK_LT(std::size_t(length),compressed.size());

  rvalue = SQY_Decode_UI16((const char*)compressed.data(),
                                   length,
                                   (char*)outputdata.data(),
                                   1
                  );

  BOOST_REQUIRE_EQUAL(rvalue, 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+10,
                  outputdata.data(), outputdata.data()+10);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data()+len-10, inputdata.data()+len,
                  outputdata.data()+len-10, outputdata.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+len,
                  outputdata.data(), outputdata.data()+len);

  // BOOST_CHECK_EQUAL_COLLECTIONS(inputdata.begin(), inputdata.end(),
  //                outputdata.begin(), outputdata.end()
  //                );

}


BOOST_AUTO_TEST_CASE( quantiser_only ){

  const std::string filter_name = "quantiser";

  std::vector<size_t> shape(3,128);
  shape.front() *= 2;

  const size_t len = std::accumulate(shape.begin(),
                     shape.end(),
                     1.,
                     std::multiplies<size_t>());

  long length = filter_name.size();

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
                                       &length,
                                       1);
  BOOST_REQUIRE_EQUAL(rvalue, 0);
  BOOST_CHECK_LT((std::size_t)length,compressed.size());

  rvalue = SQY_Decode_UI16((const char*)compressed.data(),
                   length,
                                   (char*)outputdata.data(),
                                   1
                  );

  BOOST_REQUIRE_EQUAL(rvalue, 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+10,
                  outputdata.data(), outputdata.data()+10);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data()+len-10, inputdata.data()+len,
                  outputdata.data()+len-10, outputdata.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(inputdata.data(), inputdata.data()+len,
                  outputdata.data(), outputdata.data()+len);

  // BOOST_CHECK_EQUAL_COLLECTIONS(inputdata.begin(), inputdata.end(),
  //                outputdata.begin(), outputdata.end()
  //                );

}
BOOST_AUTO_TEST_SUITE_END()
#endif
