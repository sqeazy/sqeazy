#define BOOST_TEST_MODULE TEST_H264_SCHEME_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <climits>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include <cstdint>

#define DEBUG_H264

#include "encoders/external_encoders.hpp"
#include "volume_fixtures.hpp"
#include "tiff_utils.hpp"
#include "yuv_utils.hpp"
#include "sqeazy_algorithms.hpp"
#include "traits.hpp"
#include "test_algorithms.hpp"

namespace sqy = sqeazy;


BOOST_FIXTURE_TEST_SUITE( avcodec_8bit, sqeazy::volume_fixture<std::uint8_t> )

BOOST_AUTO_TEST_CASE( encode_to_something_different ){

  av_register_all();

  std::vector<char> results(embryo_.num_elements(),0);
  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  sqeazy::h264_scheme<std::uint8_t> scheme;
  auto results_end = scheme.encode(embryo_.data(),
                   results.data(),
                   shape);
  std::size_t bytes_written = results_end - &results[0];
  results.resize(bytes_written);

  BOOST_CHECK(results_end!=nullptr);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());

  /* check that results is not filled with 0s anymore */
  double sum = std::accumulate(results.begin(), results.end(),0.);
  BOOST_CHECK_NE(sum,0);

}

BOOST_AUTO_TEST_CASE( lossy_roundtrip ){

  av_register_all();

  std::vector<char> encoded(embryo_.num_elements(),0);
  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  sqeazy::h264_scheme<std::uint8_t> scheme("preset=ultrafast");
  auto encoded_end = scheme.encode(embryo_.data(),
                   encoded.data(),
                   shape);
  std::size_t bytes_written = encoded_end - &encoded[0];
  encoded.resize(bytes_written);

  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());


  //std::vector<std::uint8_t> retrieved(embryo_.num_elements(),0);
  sqy::uint8_image_stack roundtrip_ = embryo_;
  int err = scheme.decode(encoded.data(),
              roundtrip_.data(),
              shape);

  BOOST_CHECK_EQUAL(err,0);


  sqy::uint8_image_stack_cref roundtrip_cref(roundtrip_.data(),shape);
  sqy::uint8_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(roundtrip_cref,embryo_cref);
  double l1norm = sqeazy::l1norm(roundtrip_cref,embryo_cref);

  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm << ", l1norm = " << l1norm );

  try{
    BOOST_REQUIRE_LT(l2norm,1);
    BOOST_WARN_MESSAGE(std::equal(roundtrip_.data(),
                  roundtrip_.data()+roundtrip_.num_elements(),
                  embryo_.data()),
               "raw noisy volume and encoded/decoded volume do not match exactly, l2norm " << l2norm);


  }
  catch(...){
    sqeazy::write_image_stack(embryo_,"embryo.tiff");
    sqeazy::write_stack_as_y4m(embryo_,"embryo.y4m");
    sqeazy::write_image_stack(roundtrip_,"retrieved.tiff");
    sqeazy::write_stack_as_y4m(roundtrip_,"retrieved.y4m");
    throw;
  }

}

BOOST_AUTO_TEST_CASE( lossless_roundtrip ){

  av_register_all();

  std::vector<char> encoded(embryo_.num_elements(),0);
  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  sqeazy::h264_scheme<std::uint8_t> scheme("preset=ultrafast,qp=0");
  auto encoded_end = scheme.encode(embryo_.data(),
                   &encoded[0],
                   shape);
  std::size_t bytes_written = encoded_end - &encoded[0];
  encoded.resize(bytes_written);

  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());


  std::vector<std::uint8_t> retrieved(embryo_.num_elements(),0);
  int err = scheme.decode(&encoded[0],
              &retrieved[0],
              shape);

  BOOST_CHECK_EQUAL(err,0);

  sqy::uint8_image_stack_cref retrieved_cref(retrieved.data(),shape);
  sqy::uint8_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(retrieved_cref,embryo_cref);
  double l1norm = sqeazy::l1norm(retrieved_cref,embryo_cref);

  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm << ", l1norm = " << l1norm );

  try{
    BOOST_REQUIRE_LT(l2norm,1);
    BOOST_CHECK_MESSAGE(std::equal(retrieved.begin(),
                   retrieved.begin()+retrieved.size(),
                   embryo_.data()),
            "raw noisy volume and encoded/decoded volume do not match exactly, l2norm " << l2norm);


  }
  catch(...){
    sqeazy::write_image_stack(embryo_,"embryo.tiff");
    sqeazy::write_image_stack(retrieved_,"retrieved.tiff");
    throw;
  }

}


BOOST_AUTO_TEST_CASE( lossless_roundtrip_step_ramp ){

  av_register_all();


  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  // shape[sqy::row_major::w] = 352;
  // shape[sqy::row_major::h] = 288;
  // embryo_.resize(shape);
  std::vector<char> encoded(embryo_.num_elements(),0);

  sqeazy::h264_scheme<std::uint8_t> scheme;

  for(std::size_t i = 0;i<embryo_.num_elements();++i){
    embryo_.data()[i] = 1 << (i % 8);

  }

  auto encoded_end = scheme.encode(embryo_.data(),
                   encoded.data(),
                   shape);
  std::size_t bytes_written = encoded_end - encoded.data();
  encoded.resize(bytes_written);

  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());


  sqy::uint8_image_stack roundtrip = embryo_;
  int err = scheme.decode(encoded.data(),
              roundtrip.data(),
              shape);

  BOOST_CHECK_EQUAL(err,0);

  sqy::uint8_image_stack_cref roundtripcref(roundtrip.data(),shape);
  sqy::uint8_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(roundtripcref,embryo_cref);
  double l1norm = sqeazy::l1norm(roundtripcref,embryo_cref);

  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm << ", l1norm = " << l1norm );

  try{
    BOOST_REQUIRE_LT(l2norm,1);
    BOOST_REQUIRE_MESSAGE(std::equal(roundtrip.data(),
                     roundtrip.data()+roundtrip.num_elements(),
                     embryo_.data()),
            "raw noisy volume and encoded/decoded volume do not match exactly, l2norm " << l2norm);


  }
  catch(...){
    sqeazy::write_image_stack(embryo_,"step_ramp.tiff");
    sqeazy::write_stack_as_y4m(embryo_,"step_ramp.y4m");
    sqeazy::write_image_stack(roundtrip,"step_ramp_rt.tiff");
    sqeazy::write_stack_as_y4m(roundtrip,"step_ramp_rt.y4m");
    std::ofstream h264_file("step_ramp.h264", std::ios_base::binary | std::ios_base::out);
    h264_file.write((char*)encoded.data(),bytes_written);
    h264_file.close();

    const std::size_t frame_size = embryo_.shape()[sqy::row_major::y]*embryo_.shape()[sqy::row_major::x];

    for(std::size_t i = 0;i < frame_size;++i){
      if(embryo_.data()[i]!=roundtrip.data()[i])
    std::cout << "frame:0 pixel:" << i<< " differ, " << (int)embryo_.data()[i] << " != " << (int)roundtrip.data()[i] << "\n";
    }
    throw;
  }
  const size_t len = embryo_.num_elements();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data(), embryo_.data()+10,
                  roundtrip.data(), roundtrip.data()+10);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data()+len-10, embryo_.data()+len,
                  roundtrip.data()+len-10, roundtrip.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data(), embryo_.data()+len,
                  roundtrip.data(), roundtrip.data()+len);

}

BOOST_AUTO_TEST_CASE( lossless_roundtrip_step_ramp_with_2_threads ){

  av_register_all();


  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  // shape[sqy::row_major::w] = 352;
  // shape[sqy::row_major::h] = 288;
  // embryo_.resize(shape);
  std::vector<char> encoded(embryo_.num_elements(),0);

  sqeazy::h264_scheme<std::uint8_t> scheme;
  scheme.set_n_threads(2);

  for(std::size_t i = 0;i<embryo_.num_elements();++i){
    embryo_.data()[i] = 1 << (i % 8);

  }

  auto encoded_end = scheme.encode(embryo_.data(),
                   encoded.data(),
                   shape);
  std::size_t bytes_written = encoded_end - encoded.data();
  encoded.resize(bytes_written);

  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());


  sqy::uint8_image_stack roundtrip = embryo_;
  int err = scheme.decode(encoded.data(),
              roundtrip.data(),
              shape);

  BOOST_CHECK_EQUAL(err,0);

  sqy::uint8_image_stack_cref roundtripcref(roundtrip.data(),shape);
  sqy::uint8_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(roundtripcref,embryo_cref);
  double l1norm = sqeazy::l1norm(roundtripcref,embryo_cref);

  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm << ", l1norm = " << l1norm );

  try{
    BOOST_REQUIRE_LT(l2norm,1);
    BOOST_REQUIRE_MESSAGE(std::equal(roundtrip.data(),
                     roundtrip.data()+roundtrip.num_elements(),
                     embryo_.data()),
            "raw noisy volume and encoded/decoded volume do not match exactly, l2norm " << l2norm);


  }
  catch(...){
    sqeazy::write_image_stack(embryo_,"step_ramp.tiff");
    sqeazy::write_stack_as_y4m(embryo_,"step_ramp.y4m");
    sqeazy::write_image_stack(roundtrip,"step_ramp_rt.tiff");
    sqeazy::write_stack_as_y4m(roundtrip,"step_ramp_rt.y4m");
    std::ofstream h264_file("step_ramp.h264", std::ios_base::binary | std::ios_base::out);
    h264_file.write((char*)encoded.data(),bytes_written);
    h264_file.close();

    const std::size_t frame_size = embryo_.shape()[sqy::row_major::y]*embryo_.shape()[sqy::row_major::x];

    for(std::size_t i = 0;i < frame_size;++i){
      if(embryo_.data()[i]!=roundtrip.data()[i])
    std::cout << "frame:0 pixel:" << i<< " differ, " << (int)embryo_.data()[i] << " != " << (int)roundtrip.data()[i] << "\n";
    }
    throw;
  }
  const size_t len = embryo_.num_elements();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data(), embryo_.data()+10,
                  roundtrip.data(), roundtrip.data()+10);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data()+len-10, embryo_.data()+len,
                  roundtrip.data()+len-10, roundtrip.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data(), embryo_.data()+len,
                  roundtrip.data(), roundtrip.data()+len);

}


BOOST_AUTO_TEST_CASE( lossy_vs_lossless ){
    av_register_all();


  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  sqeazy::h264_scheme<std::uint8_t> lossy_scheme("preset=ultrafast,tune=ssim");
  sqeazy::h264_scheme<std::uint8_t> lossless_scheme("preset=ultrafast,qp=0");

  std::vector<char> lossy_encoded(embryo_.num_elements(),0);
  std::vector<char> lossless_encoded(embryo_.num_elements(),0);


  auto encoded_end = lossy_scheme.encode(embryo_.data(),
                     &lossy_encoded[0],
                     shape);

  std::size_t lossy_bytes_written = encoded_end - &lossy_encoded[0];
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_NE(lossy_bytes_written,0u);
  BOOST_REQUIRE_LT(lossy_bytes_written,embryo_.num_elements());
  lossy_encoded.resize(lossy_bytes_written);

  encoded_end = lossless_scheme.encode(embryo_.data(),
                         &lossless_encoded[0],
                         shape);
  std::size_t lossless_bytes_written = encoded_end - &lossless_encoded[0];
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_NE(lossless_bytes_written,0u);
  BOOST_REQUIRE_LT(lossless_bytes_written,embryo_.num_elements());
  lossless_encoded.resize(lossless_bytes_written);

  sqy::uint8_image_stack lossy_stack(shape);
  sqy::uint8_image_stack lossless_stack(shape);

  int err = lossy_scheme.decode(&lossy_encoded[0],
                lossy_stack.data(),
                shape);

  BOOST_CHECK_EQUAL(err,0);

  err = lossless_scheme.decode(&lossless_encoded[0],
                lossless_stack.data(),
                shape);

  BOOST_CHECK_EQUAL(err,0);


  double l2norm_to_lossy = sqeazy::l2norm(lossy_stack,embryo_);
  double l2norm_to_lossless = sqeazy::l2norm(lossless_stack,embryo_);

  BOOST_REQUIRE_LT(l2norm_to_lossless,l2norm_to_lossy);
  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm_to_lossy = " << l2norm_to_lossy << ", l2norm_to_lossless = " << l2norm_to_lossless );
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( avcodec_16bit, sqeazy::volume_fixture<std::uint16_t> )

BOOST_AUTO_TEST_CASE( lossless_roundtrip_step_ramp ){

  av_register_all();

  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  const std::size_t frame_size = shape[sqy::row_major::y]*shape[sqy::row_major::x];

  std::vector<char> encoded(embryo_.num_elements()*sizeof(std::uint16_t),0);

  sqeazy::h264_scheme<std::uint16_t> scheme;

  for(std::size_t i = 0;i<embryo_.num_elements();++i){
    embryo_.data()[i] = i % frame_size;

  }

  auto encoded_end = scheme.encode(embryo_.data(),
                   encoded.data(),
                   shape);
  std::size_t bytes_written = encoded_end - encoded.data();
  encoded.resize(bytes_written);

  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements()*2);


  sqy::uint16_image_stack roundtrip = embryo_;
  int err = scheme.decode(encoded.data(),
              roundtrip.data(),
              shape);

  BOOST_CHECK_EQUAL(err,0);

  sqy::uint16_image_stack_cref roundtripcref(roundtrip.data(),shape);
  sqy::uint16_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(roundtripcref,embryo_cref);
  double l1norm = sqeazy::l1norm(roundtripcref,embryo_cref);

  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm << ", l1norm = " << l1norm );

  try{
    BOOST_REQUIRE_LT(l2norm,1);
    BOOST_REQUIRE_MESSAGE(std::equal(roundtrip.data(),
                     roundtrip.data()+roundtrip.num_elements(),
                     embryo_.data()),
            "raw noisy volume and encoded/decoded volume do not match exactly, l2norm " << l2norm);


  }
  catch(...){
    sqeazy::write_image_stack(embryo_,"step_ramp_16.tiff");
    // sqeazy::write_stack_as_y4m(embryo_,"step_ramp.y4m");
    sqeazy::write_image_stack(roundtrip,"step_ramp_rt_16.tiff");
    // sqeazy::write_stack_as_y4m(roundtrip,"step_ramp_rt.y4m");
    // std::ofstream h264_file("step_ramp.h264", std::ios_base::binary | std::ios_base::out);
    // h264_file.write((char*)encoded.data(),bytes_written);
    // h264_file.close();


    std::size_t printed = 0;
    for(std::size_t i = 0;i < (10*frame_size) && printed < 64;++i){
      if(embryo_.data()[i]!=roundtrip.data()[i]){
        std::cout << "frame:0 pixel:" << i
          << " ["<< embryo_.shape()[sqy::row_major::x] << "x" << embryo_.shape()[sqy::row_major::y] <<"]"
          << " differ, " << (int)embryo_.data()[i] << " != " << (int)roundtrip.data()[i] << "\n";
    ++printed;
      }
    }
    throw;
  }
  const size_t len = embryo_.num_elements();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data(), embryo_.data()+10,
                  roundtrip.data(), roundtrip.data()+10);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data()+len-10, embryo_.data()+len,
                  roundtrip.data()+len-10, roundtrip.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data(), embryo_.data()+len,
                  roundtrip.data(), roundtrip.data()+len);

}
BOOST_AUTO_TEST_SUITE_END()
