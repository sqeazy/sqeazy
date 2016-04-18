#define BOOST_TEST_MODULE TEST_H264_SCHEME_IMPL
#include "boost/test/unit_test.hpp"
#include <climits>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include <cstdint>

#define DEBUG_H264

#include "encoders/h264.hpp"
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
  char* results_end = scheme.encode(embryo_.data(),
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

  sqeazy::h264_scheme<uint8_t> scheme("preset=ultrafast");
  char* encoded_end = scheme.encode(embryo_.data(),
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

  BOOST_CHECK_EQUAL(err,0u);

  sqy::uint8_image_stack_cref retrieved_cref(retrieved.data(),shape);
  sqy::uint8_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(retrieved_cref,embryo_cref);
  double l1norm = sqeazy::l1norm(retrieved_cref,embryo_cref);
  
  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm << ", l1norm = " << l1norm );
  
  try{
    BOOST_REQUIRE_LT(l2norm,1);    
    BOOST_WARN_MESSAGE(std::equal(retrieved.begin(),
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

BOOST_AUTO_TEST_CASE( lossless_roundtrip ){

  av_register_all();
  
  std::vector<char> encoded(embryo_.num_elements(),0);
  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
				    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
				    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  sqeazy::h264_scheme<uint8_t> scheme("preset=ultrafast,qp=0");
  char* encoded_end = scheme.encode(embryo_.data(),
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

  BOOST_CHECK_EQUAL(err,0u);

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


BOOST_AUTO_TEST_CASE( lossy_vs_lossless ){
    av_register_all();
  

  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
				    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
				    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  sqeazy::h264_scheme<uint8_t> lossy_scheme("preset=ultrafast,tune=ssim");
  sqeazy::h264_scheme<uint8_t> lossless_scheme("preset=ultrafast,qp=0");

  std::vector<char> lossy_encoded(embryo_.num_elements(),0);
  std::vector<char> lossless_encoded(embryo_.num_elements(),0);


  char* encoded_end = lossy_scheme.encode(embryo_.data(),
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

  BOOST_CHECK_EQUAL(err,0u);

  err = lossless_scheme.decode(&lossless_encoded[0],
				lossless_stack.data(),
				shape);

  BOOST_CHECK_EQUAL(err,0u);


  double l2norm_to_lossy = sqeazy::l2norm(lossy_stack,embryo_);
  double l2norm_to_lossless = sqeazy::l2norm(lossless_stack,embryo_);

  BOOST_REQUIRE_LT(l2norm_to_lossless,l2norm_to_lossy);
  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm_to_lossy = " << l2norm_to_lossy << ", l2norm_to_lossless = " << l2norm_to_lossless );
}
BOOST_AUTO_TEST_SUITE_END()
